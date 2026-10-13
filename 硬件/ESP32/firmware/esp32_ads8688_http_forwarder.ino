#include <Arduino.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp32-hal-rmt.h>
#include <driver/gpio.h>

// STM32 -> ESP32-S3 -> HTTP raw-frame forwarder.
//
// Wiring (both sides are 3.3 V TTL):
//   STM32 USART1_TX (PA9)  -> ESP32-S3 GPIO18 (UART1 RX)
//   STM32 GND              -> ESP32-S3 GND
// GPIO5 -> pushbutton -> GND, with external 1 kOhm pull-up to 3.3 V.
// GPIO7 -> WS2812 status LED data input.
// GPIO47 -> MOS control (HIGH = laser on, LOW = laser off).
// GPIO17 -> STM32 USART1_RX (PA10), sends start-acquisition command.
// UART: 1152000 baud, 8 data bits, no parity, 1 stop bit, no flow control.

// Fill in the Wi-Fi network used by the ESP32.  Do not put these credentials
// in the STM32 firmware or in the data stream.
// Public-repository template: fill these locally before flashing. Never commit
// real Wi-Fi credentials.
constexpr char WIFI_SSID[] = "REPLACE_WITH_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "REPLACE_WITH_WIFI_PASSWORD";

constexpr char SERVER_HOST[] = "192.144.185.64";
constexpr uint16_t SERVER_PORT = 3000;
constexpr char SERVER_PATH[] = "/api/upload/raw";

constexpr int STM32_RX_PIN = 18;
constexpr int STM32_TX_PIN = 17;
constexpr uint32_t STM32_BAUD = 1152000;
constexpr int MEASURE_BUTTON_PIN = 5;
constexpr int STATUS_LED_PIN = 7;
constexpr int LASER_ENABLE_PIN = 47;
constexpr uint32_t LASER_ON_DURATION_MS = 15 * 1000;
// The laser supply has a 2-second soft start.  Wait 5 seconds before asking
// STM32 to start its 8-second acquisition.
constexpr uint32_t LASER_WARMUP_MS = 5000;
constexpr uint32_t STM32_FIRST_FRAME_TIMEOUT_MS = 2000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint8_t LED_BRIGHTNESS_50_PERCENT = 128;
constexpr uint8_t START_ACQUISITION_COMMAND[] = {0xA5, 0x5A, 0x01};
constexpr char FIRMWARE_ID[] = "BP-CTRL-2026-10-09-RECOVER";

// A completed frame is 640000 bytes plus the generated header and #END line.
// Two 2 MiB slots consumed 4 MiB and made boards with 2 MiB PSRAM stop in
// setup().  One 704 KiB slot is sufficient because STM32 sends one frame per
// button press and the upload runs in its own FreeRTOS task.
constexpr size_t FRAME_CAPACITY = 704 * 1024;
constexpr int FRAME_SLOTS = 1;
constexpr size_t UART_RX_BUFFER_BYTES = 32 * 1024;
constexpr size_t READ_BLOCK_BYTES = 4096;
constexpr size_t DIAGNOSTIC_SAMPLE_BYTES = 96;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 20 * 1000;
constexpr uint32_t HTTP_CONNECT_TIMEOUT_MS = 10 * 1000;

HardwareSerial Stm32Uart(1);
QueueHandle_t uploadQueue;
uint8_t *frameSlots[FRAME_SLOTS] = {nullptr};
volatile bool slotBusy[FRAME_SLOTS] = {false};
size_t slotLengths[FRAME_SLOTS] = {0};
bool frameStorageReady = false;

enum class ReceiveState { SEEK_HEADER, READ_HEADER, READ_PAYLOAD, READ_TRAILER };
ReceiveState receiveState = ReceiveState::SEEK_HEADER;

constexpr char FRAME_MAGIC[] = "#ADS8688";
constexpr size_t FRAME_MAGIC_LENGTH = sizeof(FRAME_MAGIC) - 1;
constexpr char FRAME_TRAILER[] = "#END";
constexpr size_t FRAME_TRAILER_LENGTH = sizeof(FRAME_TRAILER) - 1;

// The currently connected STM32 emits only this frame's 640000-byte binary
// payload followed by the #END status line.  It omits the ASCII header.
// Recreate the documented header on ESP32 so the server always receives the
// complete MATLAB-compatible ADS8688 frame.  Set false if STM32 firmware is
// later changed to send its own #ADS8688 header.
constexpr bool STM32_OMITS_ASCII_HEADER = true;
constexpr size_t HEADERLESS_PAYLOAD_BYTES = 640000;
constexpr char GENERATED_FRAME_HEADER[] =
    "#ADS8688 v1 rate=40000 secs=8 col=10000 blk=2000 nblk=160 ch=2 "
    "range=0x02 code=u16le\r\n";
constexpr size_t GENERATED_FRAME_HEADER_LENGTH = sizeof(GENERATED_FRAME_HEADER) - 1;
// STM32 sends exactly the raw 640000-byte payload.  Keep the server's
// established frame contract by adding this transport trailer on ESP32;
// it does not alter any ADC sample byte.
constexpr char GENERATED_FRAME_TRAILER[] =
    "#END blk=160 skip=0 over=0 dma=0 ecode=0 to=0 spi=0 ack=0\r\n";
constexpr size_t GENERATED_FRAME_TRAILER_LENGTH = sizeof(GENERATED_FRAME_TRAILER) - 1;

size_t magicMatched = 0;
int activeSlot = -1;
size_t activeLength = 0;
size_t headerLength = 0;
char header[256];
size_t payloadRemaining = 0;
size_t trailerMatched = 0;
size_t trailerMetadataLength = 0;
bool trailerSawCarriageReturn = false;
bool headerlessPayloadStarted = false;
uint32_t droppedFrames = 0;
uint32_t lastWifiHealthCheckMs = 0;
uint32_t lastUartReportMs = 0;
uint64_t uartBytesSeen = 0;
uint32_t headersSeen = 0;
uint32_t framesCaptured = 0;
uint8_t diagnosticSample[DIAGNOSTIC_SAMPLE_BYTES];
size_t diagnosticSampleLength = 0;
bool diagnosticSamplePrinted = false;
bool measurementActive = false;
bool startCommandSent = false;
bool awaitingStm32Frame = false;
uint32_t laserTurnOffAtMs = 0;
uint32_t measurementStartedAtMs = 0;
uint32_t stm32StartCommandAtMs = 0;
uint32_t stm32FirstFrameDeadlineMs = 0;
bool lastButtonReading = HIGH;
bool debouncedButtonState = HIGH;
uint32_t buttonLastChangedMs = 0;

uint8_t inputBuffer[READ_BLOCK_BYTES];

// Defined below the measurement-button code; a new measurement uses it to
// discard any incomplete previous UART frame before sending START.
static void resetReceiver();

static bool wifiCredentialsConfigured() {
  return !String(WIFI_SSID).startsWith("REPLACE_");
}

static const char *receiveStateName() {
  switch (receiveState) {
    case ReceiveState::SEEK_HEADER: return "seeking #ADS8688";
    case ReceiveState::READ_HEADER: return "reading header";
    case ReceiveState::READ_PAYLOAD: return "reading payload";
    case ReceiveState::READ_TRAILER: return "reading #END";
  }
  return "unknown";
}

static void printDiagnosticSample() {
  if (diagnosticSamplePrinted || diagnosticSampleLength == 0) {
    return;
  }
  diagnosticSamplePrinted = true;
  Serial.printf("[uart] first %u bytes (hex): ", static_cast<unsigned>(diagnosticSampleLength));
  for (size_t i = 0; i < diagnosticSampleLength; ++i) {
    Serial.printf("%02X%s", diagnosticSample[i],
                  (i + 1 == diagnosticSampleLength) ? "\n" : " ");
  }
}

static void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {
  // Values are limited to 128 (50% brightness).  neopixelWrite is available
  // in the installed Arduino-ESP32 2.x core.
  neopixelWrite(STATUS_LED_PIN, red, green, blue);
}

static void forceStm32RxPinHighImpedance() {
  // UART1 owns the input matrix after begin(). Explicitly disable the GPIO
  // output driver and internal pulls so PA9 sees GPIO18 only as a high-
  // impedance receiver.
  const gpio_num_t rxPin = static_cast<gpio_num_t>(STM32_RX_PIN);
  const esp_err_t pullResult = gpio_set_pull_mode(rxPin, GPIO_FLOATING);
  const esp_err_t directionResult = gpio_set_direction(rxPin, GPIO_MODE_INPUT);
  Serial.printf("[uart] GPIO%d RX high-impedance: pull=%d direction=%d\n",
                STM32_RX_PIN, static_cast<int>(pullResult),
                static_cast<int>(directionResult));
}

static void stopMeasurement() {
  digitalWrite(LASER_ENABLE_PIN, LOW);
  measurementActive = false;
  awaitingStm32Frame = false;
  setStatusLed(0, 0, LED_BRIGHTNESS_50_PERCENT);  // idle blue
  Serial.println("[laser] measurement ended; GPIO47=LOW, laser power off");
}

static void startMeasurement() {
  if (measurementActive) {
    Serial.println("[laser] button ignored; measurement is already active");
    return;
  }
  if (slotBusy[0]) {
    Serial.println("[control] measurement ignored; previous frame is still uploading");
    return;
  }
  // A failed PSRAM allocation must never prevent the physical measurement
  // command.  It only means this attempt cannot be retained/uploaded; the
  // operator still gets a clear UART diagnosis and can verify STM32 control.
  if (!frameStorageReady || uploadQueue == nullptr) {
    Serial.println("[control] WARNING: frame storage/upload is unavailable; starting STM32 measurement anyway");
  }
  digitalWrite(LASER_ENABLE_PIN, HIGH);
  // A new button press starts a new STM32 frame.  Discard any partial frame
  // left by a reset, unplugged UART, or an earlier failed measurement.
  resetReceiver();
  measurementActive = true;
  startCommandSent = false;
  awaitingStm32Frame = false;
  measurementStartedAtMs = millis();
  laserTurnOffAtMs = millis() + LASER_ON_DURATION_MS;
  stm32StartCommandAtMs = millis() + LASER_WARMUP_MS;
  setStatusLed(LED_BRIGHTNESS_50_PERCENT, LED_BRIGHTNESS_50_PERCENT / 2, 0);  // amber
  Serial.printf("[laser] measurement started; GPIO47=HIGH, warmup=%lu ms, total=%lu ms\n",
                static_cast<unsigned long>(LASER_WARMUP_MS),
                static_cast<unsigned long>(LASER_ON_DURATION_MS));
}

static void discardPendingStm32Bytes() {
  size_t discarded = 0;
  while (Stm32Uart.available() > 0) {
    Stm32Uart.read();
    ++discarded;
  }
  if (discarded != 0) {
    Serial.printf("[uart] discarded %u stale bytes before START\n", static_cast<unsigned>(discarded));
  }
}

static void updateMeasurementButton() {
  const bool reading = digitalRead(MEASURE_BUTTON_PIN);
  if (reading != lastButtonReading) {
    buttonLastChangedMs = millis();
    lastButtonReading = reading;
  }
  if (millis() - buttonLastChangedMs >= BUTTON_DEBOUNCE_MS && reading != debouncedButtonState) {
    debouncedButtonState = reading;
    if (debouncedButtonState == LOW) startMeasurement();
  }
  if (measurementActive && static_cast<int32_t>(millis() - laserTurnOffAtMs) >= 0) {
    stopMeasurement();
  }
  if (measurementActive && !startCommandSent &&
      static_cast<int32_t>(millis() - stm32StartCommandAtMs) >= 0) {
    // A5 5A 01 defines the frame boundary.  Arm the exact 640000-byte capture
    // first, then transmit the command; do not guess a 0xFFFF sample marker.
    resetReceiver();
    discardPendingStm32Bytes();
    if (STM32_OMITS_ASCII_HEADER && !startHeaderlessFrame()) {
      Serial.println("[control] cannot reserve frame slot; START was not sent");
      stopMeasurement();
      return;
    }
    const size_t sent = Stm32Uart.write(START_ACQUISITION_COMMAND,
                                        sizeof(START_ACQUISITION_COMMAND));
    if (sent == sizeof(START_ACQUISITION_COMMAND)) {
      startCommandSent = true;
      awaitingStm32Frame = true;
      stm32FirstFrameDeadlineMs = millis() + STM32_FIRST_FRAME_TIMEOUT_MS;
      // Use pure green rather than cyan; cyan can look blue on some WS2812s.
      setStatusLed(0, LED_BRIGHTNESS_50_PERCENT, 0);  // waiting for STM32
      Serial.printf("[control] START command sent after %lu ms; waiting for STM32 data frame\n",
                    static_cast<unsigned long>(millis() - measurementStartedAtMs));
    } else {
      Serial.println("[control] failed to send complete START command; laser turned off");
      resetReceiver();
      stopMeasurement();
    }
  }

  if (awaitingStm32Frame &&
      static_cast<int32_t>(millis() - stm32FirstFrameDeadlineMs) >= 0) {
    awaitingStm32Frame = false;
    setStatusLed(LED_BRIGHTNESS_50_PERCENT, 0, 0);  // red: command was not confirmed by data
    Serial.println("[control] ERROR: no STM32 frame within 2 seconds");
    resetReceiver();
    stopMeasurement();
  }
}

// Runs asynchronously from the ESP32 Wi-Fi event task.  It gives immediate,
// useful diagnostics even before the first data frame arrives.
static void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("[wifi] station started");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("[wifi] associated with %s; requesting IP\n", WIFI_SSID);
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf("[wifi] connected, IP=%s RSSI=%d dBm\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("[wifi] disconnected; will retry automatically");
      break;
    default:
      break;
  }
}

static void startWifiConnection() {
  if (!wifiCredentialsConfigured()) {
    return;
  }
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

static int acquireFreeSlot() {
  for (int i = 0; i < FRAME_SLOTS; ++i) {
    if (frameSlots[i] != nullptr && !slotBusy[i]) {
      slotBusy[i] = true;
      return i;
    }
  }
  return -1;
}

static void resetReceiver() {
  if (activeSlot >= 0) {
    slotBusy[activeSlot] = false;
  }
  receiveState = ReceiveState::SEEK_HEADER;
  magicMatched = 0;
  activeSlot = -1;
  activeLength = 0;
  headerLength = 0;
  payloadRemaining = 0;
  trailerMatched = 0;
  trailerMetadataLength = 0;
  trailerSawCarriageReturn = false;
  headerlessPayloadStarted = false;
}

static bool appendByte(uint8_t value) {
  if (activeSlot < 0 || activeLength >= FRAME_CAPACITY) {
    return false;
  }
  frameSlots[activeSlot][activeLength++] = value;
  return true;
}

static bool startHeaderlessFrame() {
  activeSlot = acquireFreeSlot();
  if (activeSlot < 0) {
    ++droppedFrames;
    return false;
  }
  memcpy(frameSlots[activeSlot], GENERATED_FRAME_HEADER, GENERATED_FRAME_HEADER_LENGTH);
  activeLength = GENERATED_FRAME_HEADER_LENGTH;
  headerLength = GENERATED_FRAME_HEADER_LENGTH;
  payloadRemaining = HEADERLESS_PAYLOAD_BYTES;
  trailerMatched = 0;
  trailerMetadataLength = 0;
  trailerSawCarriageReturn = false;
  headerlessPayloadStarted = false;
  receiveState = ReceiveState::READ_PAYLOAD;
  Serial.printf("[frame] STM32 header omitted; armed %u-byte ADS8688 frame\n",
                static_cast<unsigned>(GENERATED_FRAME_HEADER_LENGTH));
  return true;
}

static bool parsePayloadLength() {
  // MATLAB's producer emits, for example: blk=2000 nblk=160 ch=2.
  // The wire payload is nblk blocks of blk unsigned-16-bit samples; channels
  // are already interleaved in those blocks, so it is exactly blk*nblk*2 bytes.
  unsigned long blockSamples = 0;
  unsigned long blockCount = 0;
  if (sscanf(header, "#ADS8688 v1 rate=%*lu secs=%*lu col=%*lu blk=%lu nblk=%lu",
             &blockSamples, &blockCount) != 2) {
    return false;
  }
  const uint64_t length = static_cast<uint64_t>(blockSamples) * blockCount * 2ULL;
  if (length == 0 || length > FRAME_CAPACITY - headerLength - 32) {
    return false;
  }
  payloadRemaining = static_cast<size_t>(length);
  return true;
}

static void finishFrame() {
  slotLengths[activeSlot] = activeLength;
  const int completedSlot = activeSlot;
  ++framesCaptured;
  if (uploadQueue == nullptr) {
    Serial.println("[drop] upload queue is unavailable");
    slotBusy[completedSlot] = false;
    ++droppedFrames;
    resetReceiver();
    return;
  }
  if (xQueueSend(uploadQueue, &completedSlot, 0) != pdPASS) {
    Serial.println("[drop] upload queue unexpectedly full");
    slotBusy[completedSlot] = false;
    ++droppedFrames;
  } else {
    Serial.printf("[frame] captured %u bytes in slot %d\n",
                  static_cast<unsigned>(activeLength), completedSlot);
  }
  receiveState = ReceiveState::SEEK_HEADER;
  magicMatched = 0;
  activeSlot = -1;
  activeLength = 0;
  headerLength = 0;
  payloadRemaining = 0;
  trailerMatched = 0;
  trailerMetadataLength = 0;
  trailerSawCarriageReturn = false;
}

static void consumeByte(uint8_t value) {
  if (receiveState == ReceiveState::SEEK_HEADER) {
    if (STM32_OMITS_ASCII_HEADER) {
      // STM32's first ADS8688 conversion is an ADC value, not a reliable
      // 0xFFFF marker.  The A5 5A 01 command is the frame boundary: setup
      // arms READ_PAYLOAD before sending it, so ignore stray bytes while idle.
      return;
    }

    if (value == static_cast<uint8_t>(FRAME_MAGIC[magicMatched])) {
      ++magicMatched;
      if (magicMatched == FRAME_MAGIC_LENGTH) {
        ++headersSeen;
        activeSlot = acquireFreeSlot();
        if (activeSlot < 0) {
          ++droppedFrames;
          magicMatched = 0;
          return;
        }
        memcpy(frameSlots[activeSlot], FRAME_MAGIC, FRAME_MAGIC_LENGTH);
        memcpy(header, FRAME_MAGIC, FRAME_MAGIC_LENGTH);
        activeLength = FRAME_MAGIC_LENGTH;
        headerLength = FRAME_MAGIC_LENGTH;
        receiveState = ReceiveState::READ_HEADER;
      }
    } else {
      magicMatched = (value == static_cast<uint8_t>(FRAME_MAGIC[0])) ? 1 : 0;
    }
    return;
  }

  if (!appendByte(value)) {
    Serial.println("[drop] frame exceeded PSRAM slot capacity");
    ++droppedFrames;
    resetReceiver();
    return;
  }

  if (receiveState == ReceiveState::READ_HEADER) {
    if (headerLength >= sizeof(header) - 1) {
      Serial.println("[drop] frame header too long");
      ++droppedFrames;
      resetReceiver();
      return;
    }
    header[headerLength++] = static_cast<char>(value);
    if (headerLength >= 2 && header[headerLength - 2] == '\r' && header[headerLength - 1] == '\n') {
      header[headerLength] = '\0';
      if (!parsePayloadLength()) {
        Serial.printf("[drop] unsupported frame header: %s", header);
        ++droppedFrames;
        resetReceiver();
        return;
      }
      receiveState = ReceiveState::READ_PAYLOAD;
    }
    return;
  }

  if (receiveState == ReceiveState::READ_PAYLOAD) {
    if (!headerlessPayloadStarted && STM32_OMITS_ASCII_HEADER) {
      headerlessPayloadStarted = true;
      if (awaitingStm32Frame) {
        awaitingStm32Frame = false;
        setStatusLed(LED_BRIGHTNESS_50_PERCENT, 0, LED_BRIGHTNESS_50_PERCENT);
        Serial.println("[control] STM32 first payload byte received; acquisition confirmed");
      }
    }
    if (--payloadRemaining == 0) {
      if (STM32_OMITS_ASCII_HEADER) {
        if (activeLength + GENERATED_FRAME_TRAILER_LENGTH > FRAME_CAPACITY) {
          Serial.println("[drop] no room for generated frame trailer");
          ++droppedFrames;
          resetReceiver();
          return;
        }
        memcpy(frameSlots[activeSlot] + activeLength,
               GENERATED_FRAME_TRAILER, GENERATED_FRAME_TRAILER_LENGTH);
        activeLength += GENERATED_FRAME_TRAILER_LENGTH;
        finishFrame();
        return;
      }
      receiveState = ReceiveState::READ_TRAILER;
      trailerMatched = 0;
      trailerMetadataLength = 0;
      trailerSawCarriageReturn = false;
    }
    return;
  }

  // A valid producer trailer starts #END and ends in CRLF.  The producer is
  // allowed to put ASCII metadata (such as a frame counter) after #END.
  // This prevents a random byte sequence inside binary payload from splitting
  // a frame, while remaining compatible with the MATLAB sender's #END ... line.
  if (trailerMatched < FRAME_TRAILER_LENGTH) {
    if (value != static_cast<uint8_t>(FRAME_TRAILER[trailerMatched])) {
      Serial.println("[drop] missing #END trailer after payload");
      ++droppedFrames;
      resetReceiver();
      return;
    }
    ++trailerMatched;
    return;
  }

  if (trailerSawCarriageReturn) {
    if (value == '\n') {
      finishFrame();
      return;
    }
    Serial.println("[drop] malformed frame trailer line ending");
    ++droppedFrames;
    resetReceiver();
    return;
  }

  if (value == '\r') {
    trailerSawCarriageReturn = true;
    return;
  }
  if (value >= 0x20 && value <= 0x7e && trailerMetadataLength++ < 240) {
    return;
  }
  Serial.println("[drop] malformed frame trailer metadata");
  ++droppedFrames;
  resetReceiver();
}

static bool ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  if (!wifiCredentialsConfigured()) {
    Serial.println("[wifi] credentials have not been configured");
    return false;
  }
  Serial.printf("[wifi] connecting to %s\n", WIFI_SSID);
  WiFi.disconnect(true, false);
  startWifiConnection();
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < WIFI_CONNECT_TIMEOUT_MS) {
    vTaskDelay(pdMS_TO_TICKS(250));
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[wifi] connection failed");
    return false;
  }
  Serial.printf("[wifi] connected, IP=%s RSSI=%d dBm\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());
  return true;
}

static bool uploadFrame(const uint8_t *data, size_t length) {
  if (!ensureWifi()) {
    return false;
  }

  WiFiClient client;
  client.setTimeout(HTTP_CONNECT_TIMEOUT_MS / 1000);
  if (!client.connect(SERVER_HOST, SERVER_PORT, HTTP_CONNECT_TIMEOUT_MS)) {
    Serial.println("[http] TCP connection failed");
    return false;
  }

  client.printf("POST %s HTTP/1.1\r\n", SERVER_PATH);
  client.printf("Host: %s:%u\r\n", SERVER_HOST, SERVER_PORT);
  client.print("Content-Type: application/octet-stream\r\n");
  client.printf("Content-Length: %u\r\n", static_cast<unsigned>(length));
  // Must match the device_id bound to the APP account in user_devices.
  client.print("X-Device-Id: ESP32_001\r\n");
  client.print("Connection: close\r\n\r\n");

  size_t sent = 0;
  while (sent < length) {
    const size_t wrote = client.write(data + sent, length - sent);
    if (wrote == 0) {
      client.stop();
      Serial.printf("[http] body write stopped at %u/%u\n",
                    static_cast<unsigned>(sent), static_cast<unsigned>(length));
      return false;
    }
    sent += wrote;
    vTaskDelay(1);
  }

  String statusLine = client.readStringUntil('\n');
  client.stop();
  statusLine.trim();
  const bool success = statusLine.startsWith("HTTP/1.1 2") || statusLine.startsWith("HTTP/1.0 2");
  Serial.printf("[http] %s (%u bytes)\n", statusLine.c_str(), static_cast<unsigned>(length));
  return success;
}

static void uploadTask(void *) {
  for (;;) {
    int slot = -1;
    if (xQueueReceive(uploadQueue, &slot, portMAX_DELAY) != pdPASS) {
      continue;
    }
    bool sent = false;
    for (int attempt = 1; attempt <= 3 && !sent; ++attempt) {
      Serial.printf("[upload] slot %d, attempt %d\n", slot, attempt);
      sent = uploadFrame(frameSlots[slot], slotLengths[slot]);
      if (!sent) {
        vTaskDelay(pdMS_TO_TICKS(1000 * attempt));
      }
    }
    if (!sent) {
      Serial.printf("[drop] upload failed after retries, slot %d\n", slot);
    }
    slotBusy[slot] = false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== ADS8688 Wi-Fi frame forwarder ===");
  Serial.printf("[firmware] %s; warmup=%lu ms; laser-total=%lu ms\n",
                FIRMWARE_ID,
                static_cast<unsigned long>(LASER_WARMUP_MS),
                static_cast<unsigned long>(LASER_ON_DURATION_MS));

  // GPIO47 drives MOS enable.  Keep it LOW from boot until a debounced press.
  digitalWrite(LASER_ENABLE_PIN, LOW);
  pinMode(LASER_ENABLE_PIN, OUTPUT);
  // GPIO5 already has the physical 1 kOhm pull-up; no internal pull-up.
  pinMode(MEASURE_BUTTON_PIN, INPUT);
  setStatusLed(0, 0, LED_BRIGHTNESS_50_PERCENT);
  Serial.printf("[laser] button=GPIO%d, WS2812=GPIO%d, MOS=GPIO%d; default off\n",
                MEASURE_BUTTON_PIN, STATUS_LED_PIN, LASER_ENABLE_PIN);

  if (!wifiCredentialsConfigured()) {
    Serial.println("Set WIFI_SSID and WIFI_PASSWORD before using this firmware.");
  }

  Serial.printf("[memory] PSRAM=%u bytes; requested frame storage=%u bytes\n",
                static_cast<unsigned>(ESP.getPsramSize()),
                static_cast<unsigned>(FRAME_CAPACITY));
  for (int i = 0; i < FRAME_SLOTS; ++i) {
    frameSlots[i] = static_cast<uint8_t *>(ps_malloc(FRAME_CAPACITY));
    if (frameSlots[i] == nullptr) {
      Serial.printf("[memory] ERROR: PSRAM slot %d allocation failed; forwarding disabled\n", i);
      break;
    }
    frameStorageReady = true;
  }
  uploadQueue = xQueueCreate(FRAME_SLOTS, sizeof(int));
  if (uploadQueue == nullptr) {
    Serial.println("[memory] ERROR: upload queue allocation failed; forwarding disabled");
  }

  Stm32Uart.setRxBufferSize(UART_RX_BUFFER_BYTES);
  Stm32Uart.setTimeout(2);
  Stm32Uart.begin(STM32_BAUD, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);
  forceStm32RxPinHighImpedance();
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(onWifiEvent);
  startWifiConnection();
  if (uploadQueue != nullptr) {
    xTaskCreatePinnedToCore(uploadTask, "uploadTask", 8192, nullptr, 1, nullptr, 0);
  }

  Serial.printf("UART1 RX=GPIO%d, %lu baud; PSRAM slots=%d x %u bytes; ready=%s\n",
                STM32_RX_PIN, static_cast<unsigned long>(STM32_BAUD), FRAME_SLOTS,
                static_cast<unsigned>(FRAME_CAPACITY), frameStorageReady ? "yes" : "no");
}

void loop() {
  updateMeasurementButton();
  // Independent health check: a temporary router outage will be visible in
  // the serial monitor and is retried without waiting for a data frame.
  if (wifiCredentialsConfigured() && millis() - lastWifiHealthCheckMs >= 15000) {
    lastWifiHealthCheckMs = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[wifi] health check: not connected, retrying");
      WiFi.reconnect();
    }
  }

  const int available = Stm32Uart.available();
  if (available > 0) {
    const size_t wanted = min(static_cast<size_t>(available), READ_BLOCK_BYTES);
    const size_t received = Stm32Uart.readBytes(inputBuffer, wanted);
    uartBytesSeen += received;
    for (size_t i = 0; i < received; ++i) {
      if (diagnosticSampleLength < DIAGNOSTIC_SAMPLE_BYTES) {
        diagnosticSample[diagnosticSampleLength++] = inputBuffer[i];
      }
      consumeByte(inputBuffer[i]);
    }
  }

  if (millis() - lastUartReportMs >= 1000) {
    lastUartReportMs = millis();
    Serial.printf("[uart] bytes=%llu headers=%u frames=%u drops=%u state=%s queued=%d\n",
                  uartBytesSeen, headersSeen, framesCaptured, droppedFrames,
                  receiveStateName(), Stm32Uart.available());
    printDiagnosticSample();
  }
  delay(0);
}
