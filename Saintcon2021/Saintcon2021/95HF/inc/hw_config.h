/**
  ******************************************************************************
  * @file    hw_config.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    december-2012
  * @brief   Hardware Configuration & Setup
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* Includes ------------------------------------------------------------------*/

#include "miscellaneous.h"
#include "hal_delay.h"
#include "platform.h"

#define uc8 uint8_t
#define uc16 uint16_t

#define delayHighPriority_ms delay_ms

/* Exported types ------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define BULK_MAX_PACKET_SIZE  0x00000040

/* Regarding board antenna (and matching) appropriate value may be modified to optimized RF performances */
/*
 Analogue configuration register
 ARConfigB	bits 7:4	MOD_INDEX	Modulation index to modulator
								 3:0	RX_AMP_GAIN	Defines receiver amplifier gain

 For type A you can also adjust the Timer Window
*/

/******************  PCD  ******************/
/* ISO14443A */
#define PCD_TYPEA_ARConfigA	0x01
#define PCD_TYPEA_ARConfigB	0xDF

#define PCD_TYPEA_TIMERW    0x5A

/* ISO14443B */
#define PCD_TYPEB_ARConfigA	0x01
#define PCD_TYPEB_ARConfigB	0x51

/* Felica */
#define PCD_TYPEF_ARConfigA	0x01
#define PCD_TYPEF_ARConfigB	0x51

/* ISO15693 */
#define PCD_TYPEV_ARConfigA	0x01
#define PCD_TYPEV_ARConfigB	0xD1

/******************  PICC  ******************/
/* ISO14443A */
#define PICC_TYPEA_ACConfigA 0x27  /* backscaterring */


typedef struct{
	uint8_t spiid;
} SPI_TypeDef;

typedef struct{
	uint8_t spiid;
} USART_TypeDef;


#endif  /*__HW_CONFIG_H*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
