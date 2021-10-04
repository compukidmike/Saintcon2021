/**
  ******************************************************************************
  * @file    drv_spi.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides a set of firmware functions to manages SPI communications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
	******************************************************************************
*/ 

/* Includes ------------------------------------------------------------------*/
#include "drv_spi.h"

#define uc8 uint8_t

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup _95HF_Driver
 * 	@{
 *  @brief  <b>This folder contains the driver layer of 95HF family (CR95HF, RX95HF, ST95HF)</b>
 */

/** @addtogroup drv_SPI
 *  @{
 *  @brief  This file includes the SPI driver used by xx95HF family (CR95HF, RX95HF, ST95HF)
 *          to communicate with the MCU.
 */
 
 
/** @addtogroup drv_SPI_Private_Functions
 * 	@{
 */
 
#ifdef USE_DMA

static void RFTRANS_InitializeDMA(uint16_t length, uint8_t *pResponse, uc8 *pCommand);

/**
 *	@brief  this functions initializes the DMA in order to Transfert data from the CR95HF by SPI
 *  @param  void
 *  @retval void 
 */
static void RFTRANS_InitializeDMA(uint16_t length, uint8_t *pResponse, uc8 *pCommand)
{
	
	DMA_InitTypeDef    DMA_InitStructure;
	length = MIN (SPI_RESPONSEBUFFER_SIZE,length);

	DMA_DeInit(RFTRANS_95HF_SPI_Rx_DMA_Channel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)RFTRANS_95HF_SPI_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pResponse;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = length;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(RFTRANS_95HF_SPI_Rx_DMA_Channel, &DMA_InitStructure);
	
	DMA_DeInit(RFTRANS_95HF_SPI_Tx_DMA_Channel);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pCommand;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init(RFTRANS_95HF_SPI_Tx_DMA_Channel, &DMA_InitStructure);

}

#endif /* USE_DMA */

 
 /**
  * @}
  */

/** @addtogroup drv_SPI_Public_Functions
 * 	@{
 */

/**
 *	@brief  Sends one byte over SPI
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send	(8 bit)
 *  @retval None 
 */
void SPI_SendByte(SPI_TypeDef* SPIx, uint8_t data) 
{	
	/* Wait for SPIx Tx buffer empty */
	//while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPIx peripheral */
	//SPI_I2S_SendData(SPIx, data);
	while(_spi_m_sync_io_write(io, &data, 1));
}

/**  
 *	@brief  Sends one word over SPI
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send	(16 bit)
 *  @retval None 
 */
void SPI_SendWord(SPI_TypeDef* SPIx, uint16_t data) 
{	
	/* Wait for SPIx Tx buffer empty */	
	//while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPIx peripheral */
	//SPI_I2S_SendData(SPIx, data);
	uint8_t data8[2];
	data8[0] = data>>8;
	data8[1] = data;
	while(_spi_m_sync_io_write(io, data8, 2));
}

/**  
 *	@brief  Sends one byte over SPI and recovers a response
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send
 *  @retval data response from SPIx 
 */
uint8_t SPI_SendReceiveByte(SPI_TypeDef* SPIx, uint8_t data) 
{	
	/* Wait for SPI1 Tx buffer empty */
	//while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	//SPI_I2S_SendData(SPIx, data);	

	/* Wait for SPI1 data reception	*/
	//while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);

	/* Read & return SPI1 received data	*/
	//return SPI_I2S_ReceiveData(SPIx);
	uint8_t rxdata = 0;
	
	spi_m_sync_io_readwrite(io, &rxdata, &data, 1);
	return rxdata;
	//TODO - Does this return the correct byte??
}


/**
 *	@brief  reveive a byte array over SPI
 *  @param  SPIx	 	: where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  pCommand  	: pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse 	: pointer on the buffer response
 *  @retval None 
 */
void SPI_SendReceiveBuffer(SPI_TypeDef* SPIx, uc8 *pCommand, uint16_t length, uint8_t *pResponse) 
{
	//uint16_t i;
	
	/* the buffer size is limited to SPI_RESPONSEBUFFER_SIZE */
	//length = MIN (SPI_RESPONSEBUFFER_SIZE,length);
	//for(i=0; i<length; i++)
	//	pResponse[i] = SPI_SendReceiveByte(SPIx, pCommand[i]);
	spi_m_sync_io_readwrite(io, pResponse, pCommand, length);
}

#ifdef USE_DMA
/**
 *	@brief  reveive a byte array over SPI
 *  @param  SPIx	 	: where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  pCommand  	: pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse 	: pointer on the buffer response
 */
void SPI_SendReceiveBufferDMA(SPI_TypeDef* SPIx, uc8 *pCommand, uint16_t length, uint8_t *pResponse) 
{
	
	/* the buffer size is limited to SPI_RESPONSEBUFFER_SIZE */
	length = MIN (SPI_RESPONSEBUFFER_SIZE,length);
	RFTRANS_InitializeDMA(length, pResponse,pCommand);
	/* Enable SPI_SLAVE DMA Rx request */
		
	/* Enable DMA channels */
	DMA_Cmd(RFTRANS_95HF_SPI_Rx_DMA_Channel, ENABLE);
	DMA_Cmd(RFTRANS_95HF_SPI_Tx_DMA_Channel, ENABLE);
	/* Transfer complete */
	while(!DMA_GetFlagStatus(RFTRANS_95HF_SPI_Rx_DMA_FLAG));
}

#endif /* USE_DMA */

/**
 * @}
 */ 

/**
 * @}
 */ 

/**
 * @}
 */ 

/**
 * @}
 */ 
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
