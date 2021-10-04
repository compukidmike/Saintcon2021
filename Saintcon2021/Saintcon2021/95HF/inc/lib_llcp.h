/******************** (C) COPYRIGHT 2014 STMicroelectronics *******************
* File Name          : lib_llcp.h
* Author             : MMY Application Team 
* Version            : 1.0.0
* Date               : 15/07/2014
* Description        : LLCP Header
*******************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE 
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT,INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING 
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS 
* OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
******************************************************************************/

#ifndef _LIB_LLCP_H
#define _LIB_LLCP_H

//#include "stm32f10x.h"
#include "lib_nfcdeptarget.h"
#include "lib_nfcdepinitiator.h"
#include <string.h>


/*****************************************************************************/
/*															PARAMETERS																	 */
/*****************************************************************************/


#define LLCP_NB_SERVICE_MAX							0x03
#define LLCP_NB_CONNECTION_MAX					0x03
#define LLCP_MAX_LENGTH									0x210 /* 528 */
#define LLCP_POOL_OUT_LENGTH						0x05
#define LLCP_POOL_IN_LENGTH							0x05

#define LLCP_VERSION										0x11
#define LLCP_MAX_MIU									  LLCP_MAX_LENGTH
#define LLCP_LTO												0x96				/* 150 *10 ms = 1.5s */
#define LLCP_RW                         0x09
#define LLCP_CLASS											0x03
#define LLCP_WKS												0x0013      /* 0 LLCP, 1 SDP, 4 SNEP */

/*****************************************************************************/
/*															STRUCTURE																		 */
/*****************************************************************************/


typedef struct{
	uint8_t Version;
	uint16_t MIUX;
	uint16_t WKS;
	uint8_t LTO;
  uint8_t RW;
	uint8_t OPT;
}LLCP_ConfigStruct;


typedef struct{
	uint8_t SSAP;  /* Local */
	uint8_t PTYPE;
	uint8_t DSAP;  /* Remote */
	uint8_t LLCInternalBuff[512];
	uint16_t LLCBuffLength;
	uint8_t Flags;
	uint8_t V_S;  /* Send sequence number local LLCP */
	uint8_t V_SA; /* Send sequence numbered acknowledged by remote LLCP */
	uint8_t V_R;  /* Receive sequence number local LLCP */
	uint8_t V_RA; /* Receive sequence numbered acknowledged by remote LLCP */
	uint8_t MIU;
	uint8_t RW;
	/* Buffer for upper layer */
	uint8_t * InMessage;
	uint16_t	InLength;
	uint8_t * OutMessage;
	uint16_t	OutLength;
	uint8_t ConnectDisconnect;
}LLCP_LLC;


typedef struct{
	uint8_t SAP;
	char* uri;
	uint8_t Class;
	uint8_t (*Connect)(LLCP_LLC *LLC);
	uint8_t (*Disconnect)(LLCP_LLC* LLC);
}LLCP_Service;


/*****************************************************************************/
/*															STATUS CODE																	 */
/*****************************************************************************/


#define LLCP_SUCCESS_CODE								0x00
#define LLCP_ACTION_ONGOING							0x01
#define LLCP_ERRORCODE_FULLSERVICE			0xF0
#define LLCP_ERRORCODE_WRONG_TLV				0xF1
#define LLCP_ERRORCODE_NO_CONN_LEFT			0xF2
#define LLCP_ERRORCODE_ALREADY_ASK			0xF3
#define LLCP_ERRORCODE_NO_LLC						0xF4
#define LLCP_ERRORCODE_REBOOTNEEDED			0xF5
#define LLCP_ERRORCODE_DEFAULT					0xFF

/*****************************************************************************/
/*															FLAGS																				 */
/*****************************************************************************/

#define LLCP_DSAP_MASK									0xFC
#define LLCP_SSAP_MASK									0x3F
#define LLCP_V_R_MASK										0x0F
#define LLCP_V_S_MASK										0xF0
#define LLCP_N_R_MASK										0x0F
#define LLCP_N_S_MASK										0xF0


#define LLCP_TLV_VERSION_F				0x01
#define LLCP_TLV_MIUX_F						0x02
#define LLCP_TLV_WKS_F						0x04
#define LLCP_TLV_LTO_F						0x08
#define LLCP_TLV_RW_F							0x10
#define LLCP_TLV_SN_F							0x20
#define LLCP_TLV_OPT_F						0x40

#define LLCP_NO_ACTION						0x00
#define LLCP_CONNECTION_CLAIM			0x01
#define LLCP_DISCONNECTION_CLAIM	0x02

/*****************************************************************************/
/*															LLCP PDU																		 */
/*****************************************************************************/
#define LLCP_PTYPE_SYMM						0x00
#define LLCP_PTYPE_PAX						0x01
#define LLCP_PTYPE_AGF						0x02
#define LLCP_PTYPE_UI							0x03
#define LLCP_PTYPE_CONNECT				0x04
#define LLCP_PTYPE_DISC						0x05
#define LLCP_PTYPE_CC							0x06
#define LLCP_PTYPE_DM							0x07
#define LLCP_PTYPE_FRMR						0x08
#define LLCP_PTYPE_SNL						0x09
#define LLCP_PTYPE_I							0x0C
#define LLCP_PTYPE_RR							0x0D
#define LLCP_PTYPE_RNR						0x0E


#define LLCP_SYMM_SSAP						0x00
#define LLCP_SYMM_DSAP						0x00

#define LLCP_OFFSET_SEQ						0x02
#define LLCP_OFFSET_I_INFO				0x03
#define LLCP_OFFSET_UI_INFO				0x02

/*****************************************************************************/
/*															LLCP DISCONNECTED MODE											 */
/*****************************************************************************/

#define	LLCP_DM_DISC_PDU							0x00
#define	LLCP_DM_NOT_CONNECTED					0x01
#define	LLCP_DM_NO_SERVICE						0x02
#define	LLCP_DM_CONNECTION_REJECTED		0x03
#define	LLCP_DM_PERM_THIS_DSAP				0x10
#define	LLCP_DM_PERM_ALL_DSAP					0x11
#define	LLCP_DM_TEMP_THIS_DSAP				0x20
#define	LLCP_DM_TEMP_ALL_DSAP					0x21


/*****************************************************************************/
/*															LLCP FRMR FLAG															 */
/*****************************************************************************/

#define LLCP_FRMR_FLAG_BAD_PTYPE			0x80
#define LLCP_FRMR_FLAG_BAD_INFO				0xC0
#define LLCP_FRMR_FLAG_TOO_LONG				0x40
#define LLCP_FRMR_FLAG_BAD_NR					0x20
#define LLCP_FRMR_FLAG_BAD_NS					0x10

#define LLCP_FRMR_OFFSET_WIRS					0x02
#define LLCP_FRMR_OFFSET_SEQ					0x03
#define LLCP_FRMR_OFFSET_VSR					0x04
#define LLCP_FRMR_OFFSET_VSARA				0x05


#define LLCP_FRMR_WIRS_MASK						0xF0
#define LLCP_FRMR_PTYPE_MASK					0x0F


/*****************************************************************************/
/*															LLCP TLV PARAMETER													 */
/*****************************************************************************/

#define LLCP_TLV_VERSION_T				0x01
#define LLCP_TLV_MIUX_T						0x02
#define LLCP_TLV_WKS_T						0x03
#define LLCP_TLV_LTO_T						0x04
#define LLCP_TLV_RW_T							0x05
#define LLCP_TLV_SN_T							0x06
#define LLCP_TLV_OPT_T						0x07
#define LLCP_TLV_SDREQ_T					0x08
#define LLCP_TLV_SDRES_T					0x09

#define LLCP_TLV_VERSION_L				0x01
#define LLCP_TLV_MIUX_L						0x02
#define LLCP_TLV_WKS_L						0x02
#define LLCP_TLV_LTO_L						0x01
#define LLCP_TLV_RW_L							0x01
#define LLCP_TLV_OPT_L						0x01
#define LLCP_TLV_SDRES_L					0x02

#define LLCP_PARAM_VERSION_L			0x03
#define LLCP_PARAM_MIUX_L					0x04
#define LLCP_PARAM_WKS_L					0x04
#define LLCP_PARAM_LTO_L					0x03
#define LLCP_PARAM_RW_L						0x03
#define LLCP_PARAM_OPT_L					0x03
#define LLCP_PARAM_SDRES_L				0x04

#define LLCP_MIUX_MASK						0x0FFF
#define LLCP_RW_MASK							0x0F
#define LLCP_SDRES_MASK						0x0FFF
#define LLCP_OPT_MASK							0x03

#define LLCP_DEFAULT_MIU					128


#define LLCP_CONNECTION_FLAG_MASK				0x01
#define LLCP_CONN_ASK_FLAG_MASK					0x02
#define LLCP_DISC_ASK_FLAG_MASK					0x04
#define LLCP_INTERNAL_MESS_FLAG_MASK		0x08

#define LLCP_D_BUSY_FLAG_MASK						0x10  /* Distant */
#define LLCP_L_BUSY_FLAG_MASK						0x20	/* Local */


#define LLCP_OUT_MESS_FLAG_MASK					0x40
#define LLCP_IN_MESS_FLAG_MASK					0x80


/*****************************************************************************/
/*															MACRO																				 */
/*****************************************************************************/



#define SET_CONNECTION_FLAG(X) 	((X)	=	(X)| LLCP_CONNECTION_FLAG_MASK )
#define SET_ASK_CONN_FLAG(X) 		((X)	=	(X)| LLCP_CONN_ASK_FLAG_MASK )
#define SET_ASK_DISC_FLAG(X) 		((X)	=	(X)| LLCP_DISC_ASK_FLAG_MASK )
#define SET_LLCP_MESS_FLAG(X) 	((X)	=	(X)| LLCP_INTERNAL_MESS_FLAG_MASK )
#define SET_OUT_MESS_FLAG(X) 		((X)	=	(X)| LLCP_OUT_MESS_FLAG_MASK )
#define SET_IN_MESS_FLAG(X) 		((X)	=	(X)| LLCP_IN_MESS_FLAG_MASK )
#define SET_D_BUSY_FLAG(X) 			((X)	=	(X)| LLCP_D_BUSY_FLAG_MASK )
#define SET_L_BUSY_FLAG(X) 			((X)	=	(X)| LLCP_L_BUSY_FLAG_MASK )

#define IS_CONNECTION_FLAG(X) 	((X)& LLCP_CONNECTION_FLAG_MASK )
#define IS_ASK_CONN_FLAG(X) 		((X)& LLCP_CONN_ASK_FLAG_MASK )
#define IS_ASK_DISC_FLAG(X) 		((X)& LLCP_DISC_ASK_FLAG_MASK )
#define IS_LLCP_MESS_FLAG(X) 		((X)& LLCP_INTERNAL_MESS_FLAG_MASK )
#define IS_OUT_MESS_FLAG(X) 		((X)& LLCP_OUT_MESS_FLAG_MASK )
#define IS_IN_MESS_FLAG(X) 			((X)& LLCP_IN_MESS_FLAG_MASK )
#define IS_D_BUSY_FLAG(X) 			((X)& LLCP_D_BUSY_FLAG_MASK )
#define IS_L_BUSY_FLAG(X) 			((X)& LLCP_L_BUSY_FLAG_MASK )

#define CLEAR_CONNECTION_FLAG(X) 	((X)=(X)& (~LLCP_CONNECTION_FLAG_MASK) )
#define CLEAR_ASK_CONN_FLAG(X) 		((X)=(X)& (~LLCP_CONN_ASK_FLAG_MASK) )
#define CLEAR_ASK_DISC_FLAG(X) 		((X)=(X)& (~LLCP_DISC_ASK_FLAG_MASK) )
#define CLEAR_LLCP_MESS_FLAG(X) 	((X)=(X)& (~LLCP_INTERNAL_MESS_FLAG_MASK) )
#define CLEAR_OUT_MESS_FLAG(X) 		((X)=(X)& (~LLCP_OUT_MESS_FLAG_MASK) )
#define CLEAR_IN_MESS_FLAG(X) 		((X)=(X)& (~LLCP_IN_MESS_FLAG_MASK) )
#define CLEAR_D_BUSY_FLAG(X) 			((X)=(X)& (~LLCP_D_BUSY_FLAG_MASK) )
#define CLEAR_L_BUSY_FLAG(X) 			((X)=(X)& (~LLCP_L_BUSY_FLAG_MASK) )


#define IS_TLV_VERSION_FLAG(X) 	((X)& LLCP_TLV_VERSION_F )
#define IS_TLV_MIUX_FLAG(X) 		((X)& LLCP_TLV_MIUX_F )
#define IS_TLV_WKS_FLAG(X) 			((X)& LLCP_TLV_WKS_F )
#define IS_TLV_LTO_FLAG(X) 			((X)& LLCP_TLV_LTO_F )
#define IS_TLV_SN_FLAG(X) 			((X)& LLCP_TLV_SN_F )
#define IS_TLV_RW_FLAG(X) 			((X)& LLCP_TLV_RW_F )
#define IS_TLV_OPT_FLAG(X) 			((X)& LLCP_TLV_OPT_F )




/*****************************************************************************/
/*															LLCP FUNCTIONS															 */
/*****************************************************************************/


uint8_t LLCP_Init						  ( void );
uint8_t LLCP_AddService			  ( LLCP_Service NewService );
uint8_t LLCP_RemoveService     (uint8_t SAP);
uint8_t LLCP_ConnectService   ( uint8_t SAP, LLCP_LLC **LLC );
uint8_t LLCP_RemoveConnection ( LLCP_LLC *LLC );



uint8_t LLCP_Connect( uint8_t DSAP, LLCP_LLC *LLC );
uint8_t LLCP_Disconnect( LLCP_LLC *LLC );

void LLCP_Reply(uint8_t *pData, uint16_t *nbByte);
uint8_t LLCP_Send( void );


uint8_t LLCP_NFCDEPdisconnect(void);


uint8_t LLCP_HandlePDU(	uint16_t *pLength, uint8_t * Buffer);


uint8_t LLCP_ReplyToSYMM      ( void );
uint8_t LLCP_ReplyToPAX 			( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToAGF 			( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToUI 				( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToCONNECT 	( LLCP_LLC * LLC, uint8_t SSAP );
uint8_t LLCP_ReplyToDISC 			( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToCC 				( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToDM	 			( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToFRMR 			( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToSNL 			( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToI 				( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToRR 				( LLCP_LLC * LLC);
uint8_t LLCP_ReplyToRNR 			( LLCP_LLC * LLC);

uint8_t LLCP_SendSYMM 		  ( void );
uint8_t LLCP_SendUI 				( LLCP_LLC * LLC);
uint8_t LLCP_SendCONNECT 		( LLCP_LLC * LLC);
uint8_t LLCP_SendDISC 			( LLCP_LLC * LLC);
uint8_t LLCP_SendCC 				( LLCP_LLC * LLC);
uint8_t LLCP_SendDM 				( LLCP_LLC * LLC, uint8_t DSAP, uint8_t DM);
uint8_t LLCP_SendFRMR 			( LLCP_LLC * LLC, uint8_t Flags, uint8_t Ptype, uint8_t Seq);
uint8_t LLCP_SendSNL 				( uint8_t * URI);
uint8_t LLCP_SendI 					( LLCP_LLC * LLC);
uint8_t LLCP_SendRR 				( LLCP_LLC * LLC );
uint8_t LLCP_SendRNR 				( LLCP_LLC * LLC);

uint8_t LLCP_HandleTLVCONNECT(LLCP_LLC * LLC);

uint8_t LLCP_WritePAXTLV( uint8_t * Buffer , uint8_t TLVFlags );
void LLCP_HandleTLVPAX( uc8 * Buffer,uc16 Length);
void LLCP_SendReceiveTLVPAX( uint8_t * Buffer,uint16_t* Length);

#endif /* _LIB_LLCP_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
