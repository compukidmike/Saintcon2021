/**
  ******************************************************************************
  * @file    drv_uart.h
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides 
  				- set of firmware functions to manages UART communications
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  */ 

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __UART_H
#define __UART_H

/* Includes ----------------------------------------------------------------- */
#include "hw_config.h"

#define UART_RESPONSEBUFFER_SIZE		(uint16_t)528


/* UART functions */
void 		UART_SendByte		(USART_TypeDef* USARTx, uint8_t data) ;
uint8_t UART_ReceiveByte	(USART_TypeDef* USARTx);
void 		UART_SendBuffer		(USART_TypeDef* USARTx, uc8 *pCommand, uint16_t length); 
void 		UART_ReceiveBuffer	(USART_TypeDef* USARTx, uint8_t *pResponse, uint16_t length);



#endif /* __UART_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
