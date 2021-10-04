/**
  ******************************************************************************
  * @file    lib_iso7816pcd.h
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso 7816 commands
  ******************************************************************************
  * @copy
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

/* Define to prevent recursive inclusion --------------------------------------------------------*/
#ifndef __LIB_ISO7816_H
#define __LIB_ISO7816_H

/* Includes -------------------------------------------------------------------------------------*/
#include "lib_iso14443Apcd.h"
#include "lib_PCD.h"

/*  status and error code ---------------------------------------------------------------------- */
#define ISO7816_SUCCESSCODE											RESULTOK
#define ISO7816_ERRORCODE_DEFAULT								0x71
#define ISO7816_ERRORCODE_RESPONSE							0x72
#define ISO7816_ERRORCODE_SENDERRORCODE					0x73

/*  Iblock ------------------------------------------------------------------------------------- */
#define ISO7816_IBLOCK02												0x02
#define ISO7816_IBLOCK03												0x03

#define ISO7816_SELECT_FILE     								0xA4
#define ISO7816_UPDATE_BINARY   								0xD6
#define ISO7816_READ_BINARY     								0xB0

#define ISO7816_CLASS_0X00											0x00
#define ISO7816_CLASS_STM												0xA2

// offset of the different field of the APDU
#define ISO7816_ADPUOFFSET_BLOCK		0x00
#define ISO7816_ADPUOFFSET_CLASS		ISO7816_ADPUOFFSET_BLOCK 		+ 1
#define ISO7816_ADPUOFFSET_INS			ISO7816_ADPUOFFSET_BLOCK 		+ 2
#define ISO7816_ADPUOFFSET_P1				ISO7816_ADPUOFFSET_BLOCK		+ 3
#define ISO7816_ADPUOFFSET_P2				ISO7816_ADPUOFFSET_BLOCK 		+ 4
#define ISO7816_ADPUOFFSET_LC				ISO7816_ADPUOFFSET_BLOCK 		+	5
#define ISO7816_ADPUOFFSET_DATA			ISO7816_ADPUOFFSET_BLOCK 		+ 6

/* ADPU-Header command structure ---------------------------------------------*/
typedef struct
{
  uint8_t CLA;  /* Command class */
  uint8_t INS;  /* Operation code */
  uint8_t P1;   /* Selection Mode */
  uint8_t P2;   /* Selection Option */
} Header;

/* ADPU-Body command structure -----------------------------------------------*/
typedef struct 
{
  uint8_t LC;         						  /* Data field length */	
  uint8_t pData[256];  							/* Command parameters */ // pointer on the transceiver buffer = *(ReaderRecBuf[CR95HF_DATA_OFFSET + ISO7816_ADPUOFFSET_DATA])
  uint8_t LE;          						 /* Expected length of data to be returned */
} Body;

/* ADPU Command structure ----------------------------------------------------*/
typedef struct
{
  Header Header;
  Body Body;
} APDU_Commands;

/* SC response structure -----------------------------------------------------*/
typedef struct
{
  uint8_t SW1;         						 /* Command Processing status */
  uint8_t SW2;          						/* Command Processing qualification */
} APDU_Responce;


int8_t 	ISO7816_SelectFile 	( uc8 P1byte , uc8 P2byte , uc8 LCbyte , uint8_t *PData );
int8_t 	ISO7816_ReadBinary		( uc8 P1byte , uc8 P2byte , uc8 LEbyte , uint8_t *pDataRead );
int8_t 	ISO7816_UpdateBinary	( uc8 P1byte , uc8 P2byte , uc8 LCbyte , uint8_t *pData);


#endif /* __SMARTCARD_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
