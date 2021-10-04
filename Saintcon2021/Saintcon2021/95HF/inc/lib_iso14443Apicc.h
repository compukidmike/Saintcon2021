/**
  ******************************************************************************
  * @file    lib_iso14443Apicc.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso14443A communication in the card emulator mode.
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
#ifndef _LIB_ISO14443APICC_H
#define _LIB_ISO14443APICC_H

#include "lib_iso14443A.h"
#include "lib_PICC.h"


/* succes code */
#define LIB14443APICC_SUCESSSCODE										RESULTOK

/* error code */
#define LIB14443APICC_ERRORCODE_GENERIC							0xA0
#define LIB14443APICC_ERRORCODE_COMMANDUNKNOWN			0xA1
#define LIB14443APICC_ERRORCODE_CODECOMMAND					0xA2
#define LIB14443APICC_ERRORCODE_RATSCOMMAND					0xA3
#define LIB14443APICC_ERRORCODE_SELECTAPPLICATION		0xA4
#define LIB14443APICC_ERRORCODE_SELECTCCFILE				0xA5
#define LIB14443APICC_ERRORCODE_SELECTNDEFFILE			0xA6
#define LIB14443APICC_ERRORCODE_SENDERRORCODE				0xA7
#define LIB14443APICC_ERRORCODE_TRANSMISSION				0xA8


/* Nb of bytes of command ---------------------------------------------------------------*/

/* Value of Card  */
#define LIB14443APICC_TL														0x06
#define LIB14443APICC_T0														0x78
#define LIB14443APICC_TA1														0x80
#define LIB14443APICC_TB1														0x81
#define LIB14443APICC_TC1														0x00


int8_t PICC14A_RATS	( uc8 *pData );
int8_t PICC14A_PPS	( uc8 *pData );


#endif
/**
  * @}
  */
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
