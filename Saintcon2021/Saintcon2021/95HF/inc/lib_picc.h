/**
  ******************************************************************************
  * @file    lib_PICC.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides set of firmware functions to manages PICC device. 
	* @brief   The commands as defined in RX95HF/ST95HF datasheet
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
#ifndef _LIB_PICC_H
#define _LIB_PICC_H

/* Includes ------------------------------------------------------------------*/
#include "lib_95HF.h"

/* Parameter Byte send when selecting the protocol-----------------------------*/
// ParametersByte 8 bits:
// 
// Bit	Function
// 7:6  Transmission data rate 
//			00: 106 Kbps
//			01: RFU
//			10: RFU
//			11: RFU
//
// 5:4  Reception data rate
//			00: 106 Kbps
//			01: RFU
//			10: RFU
//			11: RFU
//	
// 3 		0: Return an error, if no RF field
//			1: Wait for RF field
//
// 2	  RFU
//	
// 1 		0: HFO
// 			1: ClkRec
//
// 0 		Append CRC if ISO14443B or ISO18092

// do not return error if RF field is not detected (bit 3 = 1)
// avoid looping sending listen command
#define PICC_PARAMETER_BYTE_ISO14443A 0x0A  
#define PICC_PARAMETER_BYTE_ISO14443B 0x0B
#define PICC_PARAMETER_BYTE_ISO18092  0x5B

/* customs command	 ------------------------------------------------------------------------*/
#define PICC_15693ANTICOL 														0xA0
#define PICC_INVENTORY16SLOTS													0xA1
#define PICC_ISWAKEUP																	0xA2
#define PICC_GOTOTAGDETECTINGSTATE										0xA3
#define PICC_CALIBRATETHETAGDETECTION									0xA4

#define PICC_GETHARDWAREVERSION												0xB2
#define PICC_TRANSPARENT															0xB7
#define PICC_PULSE_POOLINGREADING	   							   	0xB8
#define PICC_PULSE_SPINSS			     							  		0xB9
#define PICC_GETINTERFACEPINSTATE   							   	0xBA
#define PICC_SETUSBDISCONNECTSTATE   							  	0xBB
#define PICC_GETMCUVERSION														0xBC
#define PICC_RESETSEQUENCE														0xBD
#define PICC_PULSE_IRQIN															0xBE

#define PICC_POR																			0xC3
#define PICC_TAG_TRAKING															0xC4
#define PICC_CRYPTO_GENERIC														0xC5
#define PICC_NDEF_MESSAGE															0xC6

/* PICC status and erroc code -------------------------------------------------------- */
#define PICC_SUCCESSCODE															RESULTOK
#define PICC_ERRORCODE_DEFAULT												0x31
#define PICC_ERRORCODE_PARAMETER											0x32
#define PICC_ERRORCODE_PARAMETERLENGTH								0x33
#define PICC_NOREPLY_CODE															0x34
#define PICC_ERRORCODE_UARTDATARATEUNCHANGED					0x35
#define PICC_ERRORCODE_PORERROR												0x36
#define PICC_ERRORCODE_COMMANDUNKNOWN									0x37

	
/* PICC polling status ----------------------------------------------------------------*/
#define PICC_POLLING_RFTRANS_95HF											0x00
#define PICC_POLLING_TIMEOUT													0x01

/* PICC polling status -----------------------------------------------------------------*/
#define PICC_IDLERES_LENGTH														0x01
#define PICC_WAKEUPSOURCE_TIMEOUT											0x01
#define PICC_WAKEUPSOURCE_TAGDETECTING								0x02
#define PICC_WAKEUPSOURCE_IRRQINN											0x08


/* Nb of bytes of reader response ----------------------------------------------------------*/
#define PICC_RESPONSEBUFFER_SIZE		 									RFTRANS_95HF_MAX_BUFFER_SIZE
#define PICC_SENDRESPONSE_BUFFER_SIZE									0x10
#define PICC_ATSRESPONSE_BUFFER_SIZE									0x10


/* mask of steclect commnan ----------------------------------------------------------------*/
#define PICC_SELECTMASK_DATARATE											0x30
#define PICC_SELECTMASK_SUBCARRIER										0x02
#define PICC_SELECTMASK_WAITORSOF											0x08


/* protocol allowed -----------------------------------------------------------------------*/
#define PICC_PROTOCOL_FIELDOFF												0x00
#define PICC_PROTOCOL_ISO14443A												0x12
#define PICC_PROTOCOL_ISO14443B												0x13
#define PICC_PROTOCOL_ISO18092												0x14
								

#define PROTOCOL_TAG_ISO14443_SR											0x05
#define PROTOCOL_TAG_TOPAZ														0x06

	
/* RF transceiver Offset of the command and the response -------------------------------- */
#define PICC_COMMAND_OFFSET														RFTRANS_95HF_COMMAND_OFFSET
#define PICC_STATUS_OFFSET														RFTRANS_95HF_COMMAND_OFFSET
#define PICC_LENGTH_OFFSET														RFTRANS_95HF_LENGTH_OFFSET
#define PICC_DATA_OFFSET															RFTRANS_95HF_DATA_OFFSET

/* Command parameter  -------------------------------------------------------------------*/
#define SEND_MASK_APPENDCRC														0x20
#define SEND_MASK_DONTAPPENDCRC												0x00
#define SEND_MASK_8BITSINFIRSTBYTE										0x08


#define PICC_TRANSPARENTCOMMAND_OFFSET								0x01

#define ECHOREPLY_OFFSET															0x00
	
#define PSEUDOREPLY_OFFSET														0x00
#define PSEUDOREPLY_LENGTH														0x01

#define READERREPLY_STATUSOFFSET											0x00

#define IDN_RESULTSCODE_OK														0x00


#define SENDRECV_ERRORCODE_SOFT												0xFF


/*  Listen command field status ----------------------------------------------------------*/
#define LISTEN_RESULTSCODE_OK													0x80
#define LISTEN_RESULTSCODE_OK_0X00										0x00
#define LISTEN_ERRORCODE_CMDLENGTH										0x82
#define LISTEN_ERRORCODE_PROTOCOL											0x83
#define LISTEN_ERRORCODE_NOFIELD											0x8F
#define LISTEN_ERRORCODE_COM													0x86
#define LISTEN_ERRORCODE_CANCELED											0x87
#define LISTEN_ERRORCODE_SOF													0x88
#define LISTEN_ERRORCODE_OVERFLOW											0x89
#define LISTEN_ERRORCODE_FRAMING											0x8A
#define LISTEN_ERRORCODE_EGT													0x8B
#define LISTEN_ERRORCODE_CRC													0x8D
#define LISTEN_ERRORCODE_RECEPTIONLOST								0x8E


/* control byte according to protocol  -----------------------------------------------------*/

#define PICC_CONTROL_14443A_NBBYTE									0x01
// #define PICC_CONTROL_14443A_COLISIONMASK						0x80
#define PICC_CONTROL_14443A_CRCMASK									0x20
// #define PICC_CONTROL_14443A_PARITYMASK							0x10
// #define PICC_CONTROL_14443A_NBSIGNIFICANTBITMASK		0x0F
// #define PICC_CONTROL_14443A_FIRSTCOLISIONBITMASK		0x0F

#define PICC_CONTROL_14443B_NBBYTE									0x01
#define PICC_CONTROL_14443B_CRCMASK									0x02
// #define PICC_CONTROL_14443B_COLISIONMASK						0x01

#define PICC_CONTROL_FELICA_NBBYTE									0x01
#define PICC_CONTROL_FELICA_CRCMASK									0x02
// #define PICC_CONTROL_FELICA_COLISIONMASK						0x01


#define PICC_FLAG_DATA_READY									0x08
#define PICC_FLAG_DATA_READY_MASK							0xF8

#define RDREG_BUFFER_SIZE														5

/* Functions ---------------------------------------------------------------- */
int8_t PICC_PORsequence( void );

int8_t PICC_IsResultCodeOk 					( uint8_t CmdCode,uc8 *ReaderReply);
int8_t PICC_IsCommandExists					( uint8_t CmdCode);

int8_t 	PICC_CheckSendReceive				( uc8 *pCommand, uint8_t *pResponse);


int8_t PICC_Echo										( uint8_t *pResponse);
int8_t PICC_SendEOF									( uint8_t *pResponse);
int8_t PICC_ProtocolSelect					( uc8 Length,uc8 Protocol,uc8 *Parameters,uint8_t *pResponse);
int8_t PICC_Idle										( uc8 Length,uc8 *Data );
int8_t PICC_ReadRegister						( uc8 Length,uc8 Address,uc8 RegCount,uc8 Flags,uint8_t *pResponse);
int8_t PICC_WriteRegister						( uc8 Length,uc8 Address,uc8 Flags,uc8 *pData,uint8_t *pResponse);
int8_t PICC_PollData								( uint8_t *pData);
int8_t PICC_Send										( uc8 Length,uc8 *Parameters);
int8_t PICC_Listen 									( uint8_t *pResponse );
int8_t PICC_AcFilter 								( uc8 Length,uc8 ATQAParam[2], uc8 SAKParam,uc8 *pUIDData	,uint8_t *pResponse);

int8_t PICC_GetBackscatteringLoad 	( void );
int8_t PICC_SetBackscatteringLoad 	( uc8 bLoad );
void PICC_SendCustomCommand		( uc8 *pCommand, uint8_t *pResponse);
void PICC_GetMCUversion				( uint8_t *pResponse );
void PICC_GetHardwareVersion	( uint8_t *pResponse );
void PICC_SetUSBDisconnectPin ( void );


void 	 PICC_Receive_SPI_Response		( uint8_t *pData );

int8_t PICC_ChangeUARTBaudRate 			( uc8 *pCommand, uint8_t *pResponse );
int8_t PICC_CalibrateTagDetection		( uc8 *pCommand, uint8_t *pResponse );

void PICC_Enable_Interrupt( void);
void PICC_Disable_Interrupt( void);

#ifdef USE_POOL_FIELD
int8_t PICC_Pool_Field (uint8_t *pDataRead);
#endif /*USE_POOL_FIELD*/

#endif /* _LIB_PICC_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/



