/**
  ******************************************************************************
  * @file    lib_PICCEmulator.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides set of firmware functions to manages PICC device. 
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
#ifndef _LIB_PICCEMULATOR_H
#define _LIB_PICCEMULATOR_H

#define ST95HF

//#if defined(ST95HF)
#define CARD_EMULATION
#define TAG_MANAGEMENT
//#elif defined(RX95HF)
//#define TAG_MANAGEMENT
//#define CARD_EMULATION
//#endif

#include "lib_nfctype2picc.h"
#include "lib_nfctype4picc.h"
#include "lib_cardemul.h"
//#ifdef ST95HF
#include "lib_nfcdeptarget.h"
//#endif /* ST95HF */

/* UID -------------------------------------------------------------------------*/
/* Cascade level 1 4  UID bytes */
/* Cascade level 2 7  UID bytes */
/* Cascade level 3 10 UID bytes */
#define CASCADE_TAG																	0x88 /* CT when using 2 or 3 cascade level */
#define ST_MANUF_ID																	0x02 /* ISO/IEC 7816-6/AM1 ST manuf ID */
#define RX95HF_REF_ID																0x80 /* First byte of the Ref ID */
#define ST95HF_REF_ID																0x80 /* First byte of the Ref ID */

/* status and error code  ----------------------------------------------------------------*/
#define PICCEMUL_SUCESSSCODE 												RESULTOK
#define PICCEMUL_ERROR_GENERIC											0x41
#define PICCEMUL_ERRORCODE_TRANSMISSION							0x42
#define PICCEMUL_ERRORCODE_UNKNOWNCOMMAND						0x43

/* state of the anticollision    ---------------------------------------------------------*/
typedef enum {
	ACSTATE_IDLE = 0,
	ACSTATE_READY_1,
	ACSTATE_READY_2,
	ACSTATE_READY_3,
	ACSTATE_ACTIVE,
	ACSTATE_HALT = 0x80,
	ACSTATE_READYX_1 = 0x81,
	ACSTATE_READYX_2 = 0x82,
	ACSTATE_READYX_3 = 0x83,
	ACSTATE_ACTIVEX = 0x84,
	ACSTATE_UNKNOWN =0xFF,
}ANTICOL_STATE;

/* state of the card emulator   ----------------------------------------------------------*/
typedef enum {
 	PICCSTATE_UNKNOWN 	 = 0,
	PICCSTATE_DESACTIVATED,
	PICCSTATE_RFFIELD_CUTOFF,
	PICCSTATE_WAIT_RFFIELD,
	PICCSTATE_ACTIVATED,
	PICCSTATE_DATA_EXCHANGED,
}PICCEMULATOR_STATE;

/* protocol selected for the card emulator  -----------------------------------------------*/
typedef enum {
 	PICCPROTOCOL_UNKNOWN 	 = 0,
	PICCPROTOCOL_14443A,
	PICCPROTOCOL_14443B,
	PICCPROTOCOL_18092,
	PICCPROTOCOL_NFCDEPA,
	PICCPROTOCOL_NFCDEPF
}PICCEMULATOR_PROTOCOL;

/* structure of the Tag Type Emulated	 ---------------------------------------------------------*/
typedef enum {
 	PICCEMULATOR_TAG_TYPE_UNKNOWN 	 = 0,
	PICCEMULATOR_TAG_TYPE_1, 	/* not supported by ST95HF */
	PICCEMULATOR_TAG_TYPE_2,
	PICCEMULATOR_TAG_TYPE_3,
	PICCEMULATOR_TAG_TYPE_4A,
	PICCEMULATOR_TAG_TYPE_4B,
	PICCEMULATOR_TAG_TYPE_5,
	PICCEMULATOR_TAG_TYPE_6,
	PICCEMULATOR_TAG_TYPE_7,
	PICCEMULATOR_TAG_TYPE_PRO,
	PICCEMULATOR_NFCDEP_TYPE_A,
	PICCEMULATOR_NFCDEP_TYPE_F,
	PICCEMULATOR_LLCP_TYPE_A,
	PICCEMULATOR_LLCP_TYPE_F
}PICCEMULATOR_SELECT_TAG_TYPE;

/* select application Identifiers   -------------------------------------------------------*/
typedef enum {
 	PICCPROTOCOL_APPLI_UNKNOWN 	 = 0,
	PICCPROTOCOL_APPLI_NFCTYPE4,
	PICCPROTOCOL_APPLI_BMPXHG,
	PICCPROTOCOL_APPLI_FWU,
}PICCEMULATOR_SELECTCMD_APPL;

/* structure of the card emulator	 ---------------------------------------------------------*/
typedef struct {
 	PICCEMULATOR_STATE 			State;
	PICCEMULATOR_PROTOCOL 	Protocol;
	PICCEMULATOR_SELECT_TAG_TYPE   TagType;
	PICCEMULATOR_SELECTCMD_APPL		 Application;
}PICCEMULATOR;

/* public function	 ----------------------------------------------------------------*/
void PICCEmul_InitPICCEmulation ( PICCEMULATOR_SELECT_TAG_TYPE PICC_Emulated_TagType);
void PICCEmul_DisablePICCEmulation ( void );
PICCEMULATOR_STATE PICCEmul_ManagePICCEmulation ( void );

#endif


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

