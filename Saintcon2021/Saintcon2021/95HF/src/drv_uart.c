/**
  ******************************************************************************
  * @file    drv_uart.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides  a set of firmware functions to manages UART communications
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
#include "drv_uart.h"

#ifdef CR95HF

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

/** @addtogroup drv_UART
 *  @{
 *  @brief  This file includes the UART driver used by CR95HF to communicate with the MCU.         
 */


/** @addtogroup drv_UART_Private_Functions
 * 	@{
 */

 
 /**
  * @}
  */

/** @addtogroup drv_UART_Public_Functions
 * 	@{
 */

extern __IO uint8_t						uTimeOut;
/**
 * @brief  Send one byte over UART
 * @param  USARTx : where x can be 1, 2, 3 to select the USART peripheral
 * @param  data 	: data to send
 */
void UART_SendByte(USART_TypeDef* USARTx, uint8_t data) 
{	
	/* Wait for USART Tx buffer empty */
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);

	/* Send byte through the UART peripheral */
	USART_SendData(USARTx, data);	
}

/**
 * @brief  Receive one byte over UART
 * @param  USARTx : where x can be 1, 2, 3 to select the USART peripheral
 */
uint8_t UART_ReceiveByte( USART_TypeDef* USARTx ) 
{	
	StartTimeOut(3000);
	/* Wait for UART data reception	*/
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET && uTimeOut != true);
	StopTimeOut( );
	/* Read & return UART received data	*/
	return USART_ReceiveData(USARTx);
}

/**
 * @brief  Send a byte array over UART
 * @param  USARTx : where x can be 1, 2, 3 to select the USART peripheral
 * @param  pData  : buffer to send
 * @param  length : number of bytes to send
 */
void UART_SendBuffer(USART_TypeDef* USARTx, uc8 *pCommand, uint16_t length) 
{
	uint16_t i;

	for(i=0; i<length; i++)
	{
		UART_SendByte(USARTx, pCommand[i]);
	}
}

/**
 * @brief  Send a byte array over UART
 * @param  USARTx 	 : where x can be 1, 2, 3 to select the USART peripheral
 * @param  pResponse : pointer on the buffer response
 * @param  length 	 : number of bytes to read
 */
void UART_ReceiveBuffer(USART_TypeDef* USARTx, uint8_t *pResponse, uint16_t length) 
{
	uint16_t i;

	// the buffer size is limited to SPI_RESPONSEBUFFER_SIZE
	length = MIN (UART_RESPONSEBUFFER_SIZE,length);
	for(i=0; i<length; i++)
		pResponse[i] = UART_ReceiveByte(USARTx);
	
}

#endif /* CR95HF */

/**
 * @}
 */ 

/**
 * @}
 */ 

/**
*  @}
*/ 

/**
*  @}
*/
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
