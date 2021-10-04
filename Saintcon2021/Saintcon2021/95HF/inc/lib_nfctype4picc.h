/**
  ******************************************************************************
  * @file    lib_nfctype4picc.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the NFC type4 commands
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
#ifndef _LIB_NFCTYPE4PICC_H
#define _LIB_NFCTYPE4PICC_H

#include "lib_iso7816picc.h"
#include "lib_iso14443Apicc.h"


/* CC file -------------------------------------------------------------------------------*/
#define CCLEN_UB																					0x00
#define CCLEN_LB																					0x0F
#define MAPVER																						0x20
#define MLE_UB																						0x00
#define MLE_LB																						0xF6
#define MLC_UB																						0x00
#define MLC_LB																						0xF6
#define	TLV_T																							0x04
#define TLV_L																							0x06
#define TLV_V_FILEID_UB																		0x00
#define TLV_V_FILEID_LB																		0x01
#define TLV_V_FILESIZE_UB																	((NFCT4_MAX_NDEFMEMORY&0xFF00)>>8)
#define TLV_V_FILESIZE_LB																	(NFCT4_MAX_NDEFMEMORY&0x00FF)
#define TLV_V_FILEREADACCESS															0x00
#define TLV_V_FILEWRITEACCESS 														0x00

/* NDEF file -----------------------------------------------------------------------------*/
/* In this example the following URL is stored http://www.st.com */
/* It's a short record SR=1 without ID length field IL=0 */
/* ------------------------------- */
/* | 7 | 6 | 5 | 4 | 3 | 2  1  0 | */
/* ------------------------------- */
/* |MB |ME |CF |SR |IL |   TNF	 | */ 
/* ------------------------------- */
/* |				TYPE LENGTH					 | */
/* ------------------------------- */
/* |				PAYLOAD LENGTH			 | */
/* ------------------------------- */
/* |				TYPE     						 | */
/* ------------------------------- */
/* |				ID									 | */
/* ------------------------------- */
/* |				PAYLOAD							 | */
/* ------------------------------- */
/* MB=1 this record is the start of NDEF, ME=1 this record is also the end of the NDEF message */
/* CF=0 this record chunk contains the last chunk of data, SR=1 this is a short record */
/* IL=0 ID_LENGTH field is not present, TNF=1 it's a NFC Forum well-known type */
#define NDEF_LENGTH_UB																		0x00
#define NDEF_LENGTH_LB																		0x0B
#define RECORD_FLAGS																			0xD1
#define	TYPE_LENGTH 																			0x01
#define PAYLOAD_LENGTH 																		0x07
#define TYPE 																							0x55
#define HTTP_HEADER 																			0x01

/* instruction byte ----------------------------------------------------------------------*/
#define PICCNFCT4_CLASS																		PICC7816_NFCTYPE4_CLASS
#define PICCNFCT4_SELECTCMD																PICC7816_NFCTYPE4_SELECTCMD
#define PICCNFCT4_READCMD																	PICC7816_NFCTYPE4_READCMD
#define PICCNFCT4_UPDATECMD																PICC7816_NFCTYPE4_UPDATECMD
#define PICCNFCT4_CLEAR																		PICC7816_NFCTYPE4_CLEAR

/*ISO7816 status code ---------------------------------------------------------------------*/
#define PICCNFCT4_STATUS_STATUSOK													PICC7816_STATUS_STATUSOK
#define PICCNFCT4_STATUS_APPLICATIONNOTFOUND							PICC7816_STATUS_APPLICATIONNOTFOUND
#define PICCNFCT4_STATUS_FILENNOTFOUND										PICC7816_STATUS_FILENNOTFOUND
#define PICCNFCT4_STATUS_WRONGP1P2PARAMETERS							PICC7816_STATUS_WRONGP1P2PARAMETERS
#define PICCNFCT4_STATUS_COMMANDNOTALLOWED								PICC7816_STATUS_COMMANDNOTALLOWED
#define PICCNFCT4_STATUS_DEFAULTEERORCODE									PICC7816_STATUS_DEFAULTEERORCODE
#define PICCNFCT4_STATUS_RANSMISSIONERORCODE							PICC7816_STATUS_TRANSMISSIONERORCODE

/* status and error code  ----------------------------------------------------------------*/
#define PICCNFCT4_SUCCESSCODE															RESULTOK
#define PICCNFCT4_ERRORCODE_GENERIC												0x51
#define PICCNFCT4_ERRORCODE_SELECTAPPLICATION							0x52
#define PICCNFCT4_ERRORCODE_PARAMETER											0x53
#define PICCNFCT4_ERRORCODE_COMMANDUNKNOWN								0x54

/* File identifier   ----------------------------------------------------------------*/
#define PICCEMUL_NFCTYPE4_NDEFAPPLICATION						0xD276
#define PICCEMUL_NFCTYPE4_CCFILE										0xE103
#define PICCEMUL_NFCTYPE4_NDEFFILE									0x0001
#define PICCEMUL_NFCTYPE4_STPROPRIETARY							0xF002
#define PICCEMUL_NFCTYPE4_DEBUG											0xDEB0

/* NDEF File Identifier    -----------------------------------------------------------------*/
typedef enum{
	NDEFFILE_SELECTED_NONE,
	NDEFFILE_SELECTED_CCFILE,
	NDEFFILE_SELECTED_NDEFFILE1,
	NDEFFILE_SELECTED_NDEFFILE2,
	NDEFFILE_SELECTED_NDEFFILE3,
	NDEFFILE_SELECTED_NDEFFILE4,
	NDEFFILE_SELECTED_NDEFFILE5,
	NDEFFILE_SELECTED_NDEFFILE6,
	NDEFFILE_SELECTED_NDEFFILE7,
	NDEFFILE_SELECTED_NDEFFILE8,
}NDEF_FILE;

/* select command Identifiers  ------------------------------------------------------------*/
typedef enum {
 	PICCPROTOCOL_SELECTCMD_UNKNOWN 	 = 0,
	PICCPROTOCOL_SELECTCMD_SELECTNFCT4APPLICATION,
	PICCPROTOCOL_SELECTCMD_SELECTCCFILE,
	PICCPROTOCOL_SELECTCMD_SELECTNDEFFILE,
	PICCPROTOCOL_SELECTCMD_FT,
}PICCEMULATOR_SELECTCMD_ID;


typedef enum {
 	PICCNFCT4_TYPEA 	 = 0,
	PICCNFCT4_TYPEB,
}PICCNFCT4_SELECT_TYPE;

/* select application Identifiers   -------------------------------------------------------*/
typedef enum {
 	PICCNFCT4_APPLI_UNKNOWN 	 = 0,
	PICCNFCT4_APPLI_NFCTYPE4,
	PICCNFCT4_APPLI_BMPXHG,
	PICCNFCT4_APPLI_FWU,
}PICCNFCT4_SELECTCMD_APPL;

/* public function	 ---------------------------------------------------------------------*/
void 		PICCNFCT4_Init 									( PICCNFCT4_SELECT_TYPE SelectedType );
void 		PICCNFCT4_DeInit 								( PICCNFCT4_SELECT_TYPE SelectedType );
int8_t 	PICCNFCT4_ReplyCommand					( uc8 *pData );
void 		PICCNFCT4_SelectApplication			( uc8 *pData );
void 		PICCNFCT4_SelectNDEFfile				( uc8 *pData );
void 		PICCNFCT4_SelectCCfile					( uc8 *pData );
int8_t 	PICCNFCT4_ReadBinary 						( uc8 *pData );
int8_t 	PICCNFCT4_UpdateBinary 					( uc8 *pData );


#endif


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
