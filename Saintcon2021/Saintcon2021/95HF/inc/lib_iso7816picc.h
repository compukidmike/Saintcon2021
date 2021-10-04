/**
  ******************************************************************************
  * @file    lib_iso7816picc.h
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the Iso/IEC 7816 communication in the PICC mode emulator.
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
#ifndef _LIB_ISO7816PICC_H
#define _LIB_ISO7816PICC_H

#include "lib_PICC.h"

/* instruction byte ----------------------------------------------------------------------*/
#define PICC7816_NFCTYPE4_CLASS															0x00
#define PICC7816_NFCTYPE4_STMCLASS													0xA2
#define PICC7816_NFCTYPE4_SELECTCMD													0xA4
#define PICC7816_NFCTYPE4_READCMD														0xB0
#define PICC7816_NFCTYPE4_UPDATECMD													0xD6
#define PICC7816_NFCTYPE4_CLEAR															0x07

#define COMMAND_SBLOCK																			0xC2
#define COMMAND_NACKBLOCK_B2																0xB2
#define COMMAND_NACKBLOCK_B3																0xB3

#define COMMAND_ACKBLOCK_A2																	0xA2
#define COMMAND_ACKBLOCK_A3																	0xA3



#define PICC_ISO7816_CLASS_0X00															0x00
#define PICC_ISO7816_CLASS_STM															0xA2

/*ISO7816 status code ---------------------------------------------------------------------*/
#define  PICC7816_STATUS_STATUSOK														0x9000
#define  PICC7816_WARNING_NOINFORMATION											0x6200
#define  PICC7816_STATUS_APPLICATIONNOTFOUND								0x6A82
#define  PICC7816_STATUS_FILENNOTFOUND											0x6A82
#define  PICC7816_STATUS_WRONGP1P2PARAMETERS								0x5B00
#define  PICC7816_STATUS_COMMANDNOTALLOWED									0x6900
#define  PICC7816_STATUS_DEFAULTEERORCODE										0x6A00
#define  PICC7816_STATUS_TRANSMISSIONERORCODE								0x6A01
#define  PICC7816_STATUS_INSTRUCTIONCODENOTSUPPORTED				0x6D00
#define  PICC7816_STATUS_CLASSNOTSUPPORTED									0x6E00

/*offset of the different field of the APDU ------------------------------------------------*/
#define ISO7816_ADPUOFFSET_BLOCK		0x00
#define ISO7816_ADPUOFFSET_CLASS		ISO7816_ADPUOFFSET_BLOCK 		+ 1
#define ISO7816_ADPUOFFSET_INS			ISO7816_ADPUOFFSET_BLOCK 		+ 2
#define ISO7816_ADPUOFFSET_P1				ISO7816_ADPUOFFSET_BLOCK		+ 3
#define ISO7816_ADPUOFFSET_P2				ISO7816_ADPUOFFSET_BLOCK 		+ 4
#define ISO7816_ADPUOFFSET_LC				ISO7816_ADPUOFFSET_BLOCK 		+	5
#define ISO7816_ADPUOFFSET_DATA			ISO7816_ADPUOFFSET_BLOCK 		+ 6

/* Public functions ------------------------------------------------------------------------*/
void PICC7816_ACK 												( uc8 *pData );
void PICC7816_Deselect 										( uc8 *pData );
void PICC7816_SendAStatusCode							( uc8 *pData, uc16 StatusCode );
void PICC7816_SendBuffer 									( uc8 *pData ,uc8 *pDataTosend,uc8 NbByte, uc16 statusCode );
void PICC7816_SendANotAck 								( uc8 *pData );

#endif


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
