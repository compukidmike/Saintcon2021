/**
  ******************************************************************************
  * @file    lib_nfcdeptarget.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the NFC DEP commands
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
#ifndef _LIB_NFCDEPTARGET_H
#define _LIB_NFCDEPTARGET_H

#include "lib_iso14443Apicc.h"

#define uc8 uint8_t
#define uc16 uint16_t
#define u8 uint8_t
#define u16 uint16_t

/* succes code ---------------------------------------------------------------------*/
#define PICCNFCDEP_SUCESSSCODE										RESULTOK

/* error code ---------------------------------------------------------------------*/
#define PICCNFCDEP_ERRORCODE_GENERIC							0xA0
#define PICCNFCDEP_ERRORCODE_COMMANDUNKNOWN				0xA1
#define PICCNFCDEP_ERRORCODE_CODECOMMAND					0xA2
#define PICCNFCDEP_ERRORCODE_CRCERROR					  	0xA3
#define PICCNFCDEP_ERRORCODE_SENDERRORCODE			  0xA7
#define PICCNFCDEP_ERRORCODE_TRANSMISSION					0xA8
#define PICCNFCDEP_ERRORCODE_PARAMERROR						0xA9
#define PICCNFCDEP_ERRORCODE_ATRCOMMAND						0xAA
#define PICCNFCDEP_ERRORCODE_BAD_DID_PARAM				0xAB
#define PICCNFCDEP_ERRORCODE_BAD_FSL_PARAM				0xAC
#define PICCNFCDEP_ERRORCODE_PSLCOMMAND						0xAD


/* Nb of bytes  ----------------------------------------------------------------------------*/
#define PICCNFCDEP_MAX_NFCID_SIZE									0x08
#define PICCNFCDEP_MAX_PAD_SIZE										0x08
#define PICCNFCDEP_CRC_SIZE												0x02

#define PICCNFCDEP_MAX_NFCIDI3_SIZE								0x0A

#define PICCNFCDEP_MAX_NFCDEP_BUFFER_SIZE					0xFF

#define PICCNFCDEP_CMD1_OFFSET										0x00
#define PICCNFCDEP_CMD2_OFFSET										0x01
#define PICCNFCDEP_PAYLOAD_OFFSET									0x02

#define PICCNFCDEP_MIN_BYTE_IN_ATR								0x0E
#define PICCNFCDEP_MAX_GI_BYTES										0xF0
#define PICCNFCDEP_MAX_GT_BYTES										0xF0


#define PICCNFCDEP_LR_MASK												0x30
#define PICCNFCDEP_PFB_PNI_MASK										0x03
#define PICCNFCDEP_PFB_PDU_MASK										0xC0
#define PICCNFCDEP_PFB_NAD_MASK										0x08
#define PICCNFCDEP_PFB_DID_MASK										0x04
#define PICCNFCDEP_BRS_DSI_MASK										0x38
#define PICCNFCDEP_BRS_DRI_MASK										0x07

/* Protocol  -------------------------------------------------------------------------------*/

#define NFCDEP_ON_NFCA														0xAA
#define NFCDEP_ON_NFCF														0xFF

#define PICCNFCDEP_TARGET_TYPEA										0x0A
#define PICCNFCDEP_TARGET_TYPEF										0x0F

/* response values -------------------------------------------------------------------------*/
#define PICCNFCDEP_SB															0xF0

#define PICCNFCDEP_COMMAND_REQ_CMD1								0xD4
#define PICCNFCDEP_COMMAND_RES_CMD1								0xD5

#define PICCNFCDEP_COMMAND_ATR_REQ_CMD2						0x00
#define PICCNFCDEP_COMMAND_ATR_RES_CMD2						0x01
#define PICCNFCDEP_COMMAND_PSL_REQ_CMD2						0x04
#define PICCNFCDEP_COMMAND_PSL_RES_CMD2						0x05
#define PICCNFCDEP_COMMAND_DEP_REQ_CMD2						0x06
#define PICCNFCDEP_COMMAND_DEP_RES_CMD2						0x07
#define PICCNFCDEP_COMMAND_DSL_REQ_CMD2						0x08
#define PICCNFCDEP_COMMAND_DSL_RES_CMD2						0x09
#define PICCNFCDEP_COMMAND_RLS_REQ_CMD2						0x0A
#define PICCNFCDEP_COMMAND_RLS_RES_CMD2						0x0B


#define PICCNFCDEP_BRS_106												0x00
#define PICCNFCDEP_BRS_212												0x09
#define PICCNFCDEP_BRS_424												0x12


typedef struct{
	/* NFCID : target Identification*/
	uint8_t		TargetType;
	uint8_t 	NfcId	[PICCNFCDEP_MAX_NFCIDI3_SIZE];
	uint8_t 	Pad		[PICCNFCDEP_MAX_PAD_SIZE];
}PICCNFCDEP_TARGET;


typedef struct{
/* Information receive with ATR_REQ */
	uint8_t 	NfcId3i	[PICCNFCDEP_MAX_NFCIDI3_SIZE];
	uint8_t   Did;
	uint8_t 	BSi;
	uint8_t 	BRi;
	uint8_t 	PPi;
	uint8_t 	LRi; /* value retrieve from PPi */
	uint8_t 	Gi[PICCNFCDEP_MAX_GI_BYTES];
	bool			LLCPModeSupported; /* if magic number in G param */
/* Information receive with PSL_REQ */
	uint8_t 	BRS;
	uint8_t 	FSL;
/* Information receive with DEP_REQ */	
	uint8_t 	PFB;
	uint8_t 	NAD;
}PICCNFCDEP_INITIATOR_PROPERTIES;

typedef struct{
/* Information send with ATR_RES */
	uint8_t 	NfcId3t	[PICCNFCDEP_MAX_NFCIDI3_SIZE];
	uint8_t   Did;
	uint8_t 	BSt;
	uint8_t 	BRt;
	uint8_t 	TO;
	uint8_t 	PPt;
	uint8_t 	LRt; /* value retrieve from PPt */
	uint8_t 	Gt[PICCNFCDEP_MAX_GT_BYTES];
	bool			LLCPModeSupported; /* must be init to true if supported */
/* Information send with PSL_REQ */
	
/* Information send with DEP_REQ */	
	uint8_t 	PFB;
	uint8_t 	NAD;
}PICCNFCDEP_TARGET_PROPERTIES;

void PICCNFCDEP_Init ( uint8_t Protocol, bool LLCPModeSupported );
int8_t PICCNFCDEP_ReplyCommand( uc8 *pData );

int8_t PICCNFCDEP_ReqC( uc8 *pData );

int8_t PICCNFCDEP_Atr (uc8 *pData, u8 nbBytes);
int8_t PICCNFCDEP_Psl ( uc8 *pData );
int8_t PICCNFCDEP_Dep (uc8 *pData, u16 Length );
int8_t PICCNFCDEP_Dsl ( uc8 *pData );
int8_t PICCNFCDEP_Rls ( uc8 *pData );

void PICCNFCDEP_CallBack(uc8 *pData, u16 *nbByte);
void PICCNFCDEP_LLCPPaxCallBack(uc8 *pData, u16 *nbByte);

#endif /* _LIB_NFCDEPTARGET_H */
/**
  * @}
  */
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
