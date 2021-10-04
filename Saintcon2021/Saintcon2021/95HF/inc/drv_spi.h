/**
  ******************************************************************************
  * @file    drv_spi.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides a set of firmware functions to manages SPI communications
  ******************************************************************************
  * @copyright
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
#ifndef __SPI_H
#define __SPI_H

/* Includes ----------------------------------------------------------------- */
#include "hw_config.h"
#include "main.h"

#define uc8 uint8_t

#define SPI_RESPONSEBUFFER_SIZE		(uint16_t)528

void SPI_SendByte(SPI_TypeDef* SPIx, uint8_t data);
void SPI_SendWord(SPI_TypeDef* SPIx, uint16_t data);

uint8_t SPI_SendReceiveByte		(SPI_TypeDef* SPIx, uint8_t data); 
void 		SPI_SendReceiveBuffer	(SPI_TypeDef* SPIx, uc8 *pCommand, uint16_t length, uint8_t *pResponse);
#ifdef USE_DMA
void SPI_SendReceiveBufferDMA(SPI_TypeDef* SPIx, uc8 *pCommand, uint16_t length, uint8_t *pResponse); 
#endif /* USE_DMA */

#endif /* __SPI_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
