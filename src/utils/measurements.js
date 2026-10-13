export const validNumber = value => value !== null && value !== undefined && value !== '' && Number.isFinite(Number(value));
export const measurementTime = value => new Date(String(value).replace(' ', 'T')).getTime();
export function tenMinuteTicks(start, end) {
  if (!Number.isFinite(start) || !Number.isFinite(end) || end <= start) return [];
  const interval = 600000, ticks = [];
  for (let value = Math.ceil(start / interval) * interval; value <= end; value += interval) {
    const date = new Date(value);
    ticks.push({value, label: `${String(date.getHours()).padStart(2,'0')}:${String(date.getMinutes()).padStart(2,'0')}`});
  }
  return ticks;
}
export function axisBounds(values, minimumSpan = 10) {
  const numbers = values.filter(validNumber).map(Number);
  if (!numbers.length) return { min: 0, max: minimumSpan };
  const low = Math.min(...numbers), high = Math.max(...numbers);
  const span = Math.max(minimumSpan, high - low);
  const center = (low + high) / 2;
  const step = 10 ** Math.floor(Math.log10(span / 5));
  return { min: Math.floor((center - span * 0.65) / step) * step, max: Math.ceil((center + span * 0.65) / step) * step };
}
export function lastHour(rows, now = Date.now()) {
  const seen = new Set();
  return rows.filter(row => {
    const time = measurementTime(row.measure_time);
    const key = row.id ?? `${row.measure_time}|${row.device_id || ''}`;
    if (seen.has(key) || !Number.isFinite(time) || time > now || time < now - 3600000) return false;
    seen.add(key);
    return true;
  }).sort((a, b) => measurementTime(a.measure_time) - measurementTime(b.measure_time));
}
export function average(rows, key) {
  const values = rows.map(row => row[key]).filter(validNumber).map(Number);
  return values.length ? (values.reduce((a, b) => a + b, 0) / values.length).toFixed(1) : '--';
}
