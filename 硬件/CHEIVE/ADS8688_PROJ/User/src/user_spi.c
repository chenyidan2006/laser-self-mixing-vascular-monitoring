
#include "user_spi.h"


//uint32_t spi_error_cnt;



uint16_t spi_transmit_receive8(SPI_TypeDef *spi,uint8_t *tx,uint8_t *rx,uint16_t size)
{
	uint32_t timeout_cnt;
	static const uint32_t timeout_cnt_num = 1000000;
	
	uint16_t i = 0;
    for ( i = 0; i < size; i++)
    {   
        timeout_cnt = 0;
            while(!LL_SPI_IsActiveFlag_TXE(spi)){
            timeout_cnt++;
            if(timeout_cnt > timeout_cnt_num){
                break;
            }
        }
        
        LL_SPI_TransmitData8(spi, tx[i]);
        
        timeout_cnt = 0;
        while(LL_SPI_IsActiveFlag_BSY(spi)){
            timeout_cnt++;
            if(timeout_cnt > timeout_cnt_num){
                break;
            }
        }	
        
        timeout_cnt = 0;
        while(!LL_SPI_IsActiveFlag_RXNE(spi)){
            timeout_cnt++;
            if(timeout_cnt > timeout_cnt_num){
                break;
            }
        }

        rx[i] = LL_SPI_ReceiveData8(spi);
    }


	return i;

}


uint16_t spi_transmit_receive16(SPI_TypeDef *spi,uint16_t *tx,uint16_t *rx,uint16_t size)
{
	uint32_t timeout_cnt;
	static const uint32_t timeout_cnt_num = 1000000;
	
	uint16_t i =0;

    for ( i = 0; i < (size>>2);i++)
    {
        timeout_cnt = 0;
	
        while(!LL_SPI_IsActiveFlag_TXE(spi)){
            timeout_cnt++;
            if(timeout_cnt > timeout_cnt_num){
                break;
            }
        }
        
        LL_SPI_TransmitData16(spi, tx[i]);
        
        timeout_cnt = 0;
        while(LL_SPI_IsActiveFlag_BSY(spi)){
            timeout_cnt++;
            if(timeout_cnt > timeout_cnt_num){
                break;
            }
        }	
        
        timeout_cnt = 0;
        while(!LL_SPI_IsActiveFlag_RXNE(spi)){
            timeout_cnt++;
            if(timeout_cnt > timeout_cnt_num){
                break;
            }
        }

        rx[i] = LL_SPI_ReceiveData16(spi);
    }

    return i;
}
