/**
  ******************************************************************************
  * @file    lib_nfctype2pcd.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Generates the NFC type2 commands
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
#ifndef _LIB_NFCTYPE2PCD_H
#define _LIB_NFCTYPE2PCD_H

#include "lib_PCD.h"

/* Error codes */
#define PCDNFCT2_OK 											PCDNFC_OK
#define PCDNFCT2_ERROR 										PCDNFC_ERROR
#define PCDNFCT2_ERROR_MEMORY_TAG					PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT2_ERROR_MEMORY_INTERNAL		PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT2_ERROR_LOCKED 						PCDNFC_ERROR_LOCKED
#define PCDNFCT2_ERROR_NOT_FORMATED				PCDNFC_ERROR_NOT_FORMATED

/* Command List */
#define PCDNFCT2_READ								0x30
#define PCDNFCT2_WRITE							0xA2
#define PCDNFCT2_SECTOR_SEL					0xC2

/* Size */
#define PCDNFCT2_READ_SIZE					16
#define PCDNFCT2_READ_SIZE_BUFFER		18
#define PCDNFCT2_SECTOR_SIZE				1024

/* Mask */
#define PCDNFCT2_READ_MSK						0xF0
#define PCDNFCT2_WRITE_MSK					0x0F

/* TLV */
#define PCDNFCT2_TLV_EMPTY 					0x00
#define PCDNFCT2_TLV_LOCK 					0x01
#define PCDNFCT2_TLV_MEM	 					0x02
#define PCDNFCT2_TLV_NDEF	 					0x03

#define PCDNFCT2_NDEF_MNB						0xE1

/* Functions */
uint8_t PCDNFCT2_ReadNDEF( void );
uint8_t PCDNFCT2_WriteNDEF( void );

#endif
