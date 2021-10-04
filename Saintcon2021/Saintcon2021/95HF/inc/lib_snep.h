/******************** (C) COPYRIGHT 2013 STMicroelectronics *******************
* File Name          : lib_snep.h
* Author             : MMY Application Team 
* Version            : 1.0.0
* Date               : 15/07/2014
* Description        : SNEP library Header
*******************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE 
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT,INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING 
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS 
* OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
******************************************************************************/

#ifndef _LIB_SNEP_H
#define _LIB_SNEP_H

//#include "stm32f10x.h"
#include "lib_llcp.h"

/* SNEP version 1.0 requiered 1024B to be supported */
#define SNEP_MAX_SIZE_BUFFER					(2*1024)

typedef struct{
	
	LLCP_LLC * LLC;
	uint8_t Version;
	uint8_t SNEPFlags;
	uint32_t MessLen;
	uint32_t Offset;
}SNEP_Server;

#define SERVER_SNEP_URI										"urn:nfc:sn:snep"
#define SERVER_SNEP												0x04
#define SNEP_SERVER_VERSION								0x10

#define SERVER_SNEP_URI_SENDER						"urn:nfc:sn:snepsender"
#define SERVER_SNEP_SENDER								0x20

#define SNEP_GET_FLAG_MASK					0x01
#define SNEP_PUT_FLAG_MASK					0x02

#define SET_PUT_FLAG(X) 		((X)	=	(X)| SNEP_PUT_FLAG_MASK )
#define SET_GET_FLAG(X) 		((X)	=	(X)| SNEP_GET_FLAG_MASK )


#define IS_PUT_FLAG(X) 			((X)& SNEP_PUT_FLAG_MASK )
#define IS_GET_FLAG(X) 			((X)& SNEP_GET_FLAG_MASK )


#define CLEAR_PUT_FLAG(X) 	((X)=(X)& !SNEP_PUT_FLAG_MASK )
#define CLEAR_GET_FLAG(X) 	((X)=(X)& !SNEP_GET_FLAG_MASK )


#define	SNEP_SUCCESS_CODE							LLCP_SUCCESS_CODE
#define SNEP_ERROR_CODE								0xE0

#define SNEP_CMD_CONTINUE							0x00
#define SNEP_CMD_GET									0x01
#define SNEP_CMD_PUT									0x02
#define SNEP_CMD_REJECT								0x7F

#define SNEP_RES_CONTINUE							0x80
#define SNEP_RES_SUCCESS							0x81

#define SNEP_RES_NOT_FOUND						0xC0
#define SNEP_RES_EXCESS_DATA					0xC1
#define SNEP_RES_BAD_REQUEST					0xC2

#define SNEP_RES_NOT_IMPLEMENT				0xE0
#define SNEP_RES_UNSUPPORTED_VERS			0xE1
#define SNEP_RES_REJECT								0xFF


#define SNEP_OFFSET_VERSION						0x00
#define SNEP_OFFSET_CODE_CMD					0x01
#define SNEP_OFFSET_LENGTH						0x02
#define SNEP_OFFSET_PUT_NDEF					0x06
#define SNEP_OFFSET_GET_CMD_NDEF			0x0A
#define SNEP_OFFSET_GET_CMD_AL				0x06
#define SNEP_OFFSET_GET_RES_NDEF			0x06

#define SNEP_NO_MESSAGE								0x00
#define SNEP_MESSAGE2SEND							0x01
#define SNEP_MESSAGE_SENDINGONGOING		0x02
#define SNEP_MESSAGE_SENT							0x04
#define SNEP_MESSAGE_RECEIVED					0x08

uint8_t SNEP_Init( uint8_t SAP, char* uri );
uint8_t SNEP_ClientDeInit( uint8_t SAP );
uint8_t SNEP_Connect( uint8_t SSAP );
uint8_t SNEP_RemoveService( uint8_t SSAP  );
uint8_t	SNEP_StartSymm( void );
bool 		SNEP_IsTransmissionComplete(void);
bool SNEP_IsConnectionTerminated(void);

/* SNEP Interface */
uint8_t SNEP_SendNDEF( uint8_t *pNdefMessage, uint16_t size);
uint8_t SNEP_RetrieveNDEF( uint8_t **pNdefMessage);
void Server_SNEP (void);

#endif /* _LIB_SNEP_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
