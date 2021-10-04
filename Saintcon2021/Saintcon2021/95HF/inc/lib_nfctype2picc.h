/**
  ******************************************************************************
  * @file    lib_nfctype2picc.h
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the NFC type2 commands
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
#ifndef _LIB_NFCTYPE2PICC_H
#define _LIB_NFCTYPE2PICC_H

#include "lib_PICC.h"

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
#define RECORD_FLAGS																			0xD1
#define	TYPE_LENGTH 																			0x01
#define PAYLOAD_LENGTH 																		0x0D
#define TYPE 																							0x55
#define HTTP_HEADER 																			0x01

/* instruction byte ----------------------------------------------------------------------*/
#define PICCNFCT2_READ																		0x30
#define PICCNFCT2_WRITE																		0xA2
#define PICCNFCT2_SECTOR_SELECT														0xC2

/* max buffer size ----------------------------------------------------------------------*/

#define PICCNFCT2_READ_BUFFER_SIZE												16
#define PICCNFCT2_WRITE_BUFFER_SIZE												4
#define PICCNFCT2_SECTOR_SIZE															1024
	
/* status and error code  ----------------------------------------------------------------*/
#define PICCNFCT2_SUCCESSCODE															RESULTOK
#define PICCNFCT2_ERRORCODE_GENERIC												0x61
#define PICCNFCT2_ERRORCODE_PARAMETER											0x62
#define PICCNFCT2_ERRORCODE_COMMANDUNKNOWN								0x63
#define PICCNFCT2_ERRORCODE_WRONG_ID											0x64


/* public function	 ---------------------------------------------------------------------*/
void PICCNFCT2_Init ( void );
int8_t PICCNFCT2_ReplyCommand( uc8 *pData );
int8_t PICCNFCT2_SelectSector (uc8 *pData);
int8_t PICCNFCT2_Read (uc8 *pData);
int8_t PICCNFCT2_Write (uc8 *pData );
void PICCNFCT2_ACK (void);
void PICCNFCT2_NACK (void);

#endif


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
