/**
  ******************************************************************************
  * @file    lib_nfcdepinitiator.h 
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
#ifndef _LIB_NFCDEPINITIATOR_H
#define _LIB_NFCDEPINITIATOR_H

#include "lib_iso18092pcd.h"
#include "lib_iso14443Apcd.h"

#define uc8 uint8_t
#define uc16 uint16_t
#define u8 uint8_t
#define u16 uint16_t


/* succes code ---------------------------------------------------------------------*/
#define PCDNFCDEP_SUCESSSCODE										RESULTOK

/* error code ---------------------------------------------------------------------*/
#define PCDNFCDEP_ERRORCODE_GENERIC								0xA0
#define PCDNFCDEP_ERRORCODE_COMMANDUNKNOWN				0xA1
#define PCDNFCDEP_ERRORCODE_CODECOMMAND						0xA2
#define PCDNFCDEP_ERRORCODE_CRCERROR					  	0xA3
#define PCDNFCDEP_ERRORCODE_SENDERRORCODE			  	0xA7
#define PCDNFCDEP_ERRORCODE_TRANSMISSION					0xA8
#define PCDNFCDEP_ERRORCODE_PARAMERROR						0xA9
#define PCDNFCDEP_ERRORCODE_ATRCOMMAND						0xAA
#define PCDNFCDEP_ERRORCODE_BAD_DID_PARAM					0xAB
#define PCDNFCDEP_ERRORCODE_BAD_FSL_PARAM					0xAC


/* Nb of bytes  ----------------------------------------------------------------------------*/
#define PCDNFCDEP_MAX_NFCID_SIZE									0x08
#define PCDNFCDEP_MAX_PAD_SIZE										0x08
#define PCDNFCDEP_CRC_SIZE												0x02

#define PCDNFCDEP_MAX_NFCIDI3_SIZE								0x0A

#define PCDNFCDEP_MAX_NFCDEP_BUFFER_SIZE					0xFF

#define PCDNFCDEP_CMD1_OFFSET											0x00
#define PCDNFCDEP_CMD2_OFFSET											0x01
#define PCDNFCDEP_PAYLOAD_OFFSET									0x02

#define PCDNFCDEP_MIN_BYTE_IN_ATR									0x0E
#define PCDNFCDEP_MAX_GI_BYTES										0xF0
#define PCDNFCDEP_MAX_GT_BYTES										0xF0


#define PCDNFCDEP_LR_MASK													0x30
#define PCDNFCDEP_PFB_PNI_MASK										0x03
#define PCDNFCDEP_PFB_PDU_MASK										0xC0
#define PCDNFCDEP_PFB_NAD_MASK										0x08
#define PCDNFCDEP_PFB_DID_MASK										0x04

/* Protocol  -------------------------------------------------------------------------------*/

#define NFCDEP_ON_NFCA														0xAA
#define NFCDEP_ON_NFCF														0xFF

/* response values -------------------------------------------------------------------------*/
#define PCDCNFCDEP_SB															0xF0

#define PCDNFCDEP_COMMAND_REQ_CMD1								0xD4
#define PCDNFCDEP_COMMAND_RES_CMD1								0xD5

#define PCDNFCDEP_COMMAND_ATR_REQ_CMD2						0x00
#define PCDNFCDEP_COMMAND_ATR_RES_CMD2						0x01
#define PCDNFCDEP_COMMAND_PSL_REQ_CMD2						0x04
#define PCDNFCDEP_COMMAND_PSL_RES_CMD2						0x05
#define PCDNFCDEP_COMMAND_DEP_REQ_CMD2						0x06
#define PCDNFCDEP_COMMAND_DEP_RES_CMD2						0x07
#define PCDNFCDEP_COMMAND_DSL_REQ_CMD2						0x08
#define PCDNFCDEP_COMMAND_DSL_RES_CMD2						0x09
#define PCDNFCDEP_COMMAND_RLS_REQ_CMD2						0x0A
#define PCDNFCDEP_COMMAND_RLS_RES_CMD2						0x0B


typedef enum {
 	BRS_106 = 0x00,
	BRS_212 = 0x09,
	BRS_424 = 0x12,
}PCDNFCDEP_BRS;

typedef struct{
	/* NFCID : Target Identification*/
	uint8_t		TargetType;
	uint8_t 	NfcId	[PCDNFCDEP_MAX_NFCIDI3_SIZE];
	uint8_t 	Pad		[PCDNFCDEP_MAX_PAD_SIZE];
}PCDNFCDEP_TARGET;


typedef struct{
/* Information receive with ATR_REQ */
	uint8_t 	NfcId3i	[PCDNFCDEP_MAX_NFCIDI3_SIZE];
	uint8_t   Did;
	uint8_t 	BSi;
	uint8_t 	BRi;
	uint8_t 	PPi;
	uint8_t 	LRi; /* value retrieve from PPi */
	uint8_t 	Gi[PCDNFCDEP_MAX_GI_BYTES];
	uint8_t		GiSize;
	bool			LLCPModeSupported; /* if magic number in G param */
/* Information receive with PSL_REQ */
	uint8_t 	BRS;
	uint8_t 	FSL;
/* Information receive with DEP_REQ */	
	uint8_t 	PFB;
	uint8_t 	NAD;
}PCDNFCDEP_PCD_INITIATOR;

typedef struct{
/* Information send with ATR_RES */
	uint8_t 	NfcId3t	[PCDNFCDEP_MAX_NFCIDI3_SIZE];
	uint8_t   Did;
	uint8_t 	BSt;
	uint8_t 	BRt;
	uint8_t 	TO;
	uint8_t 	PPt;
	uint8_t 	LRt; /* value retrieve from PPt */
	uint8_t 	Gt[PCDNFCDEP_MAX_GT_BYTES];
	uint8_t		GtSize;
	bool			LLCPModeSupported; /* must be init to true if supported */
/* Information send with PSL_REQ */
	
/* Information send with DEP_REQ */	
	uint8_t 	PFB;
	uint8_t 	NAD;
}PCDNFCDEP_PCD_TARGET;

void PCDNFCDEP_Init ( uint8_t Protocol, bool LLCPModeSupported );
int8_t PCDNFCDEP_ReplyCommand( uc8 *pData );

int8_t PCDNFCDEP_IsPresentNFCA( void);
int8_t PCDNFCDEP_IsPresentNFCF( void);

int8_t PCDNFCDEP_Atr ( void );
int8_t PCDNFCDEP_Psl ( PCDNFCDEP_BRS BRS );
int8_t PCDNFCDEP_Dep (uc8 PFB, u8 *pData, u16 *nbByte );
int8_t PCDNFCDEP_Dsl ( void );
int8_t PCDNFCDEP_Rls ( void );

#endif /* _LIB_NFCDEPINITIATOR_H */
/**
  * @}
  */
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
