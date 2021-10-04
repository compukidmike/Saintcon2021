/**
  ******************************************************************************
  * @file    lib_nfctype1pcd.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Generates the NFC type1 commands
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
#ifndef _LIB_NFCTYPE1PCD_H
#define _LIB_NFCTYPE1PCD_H

#include "lib_PCD.h"

/* Error codes */
#define PCDNFCT1_OK 										PCDNFC_OK
#define PCDNFCT1_ERROR 									PCDNFC_ERROR
#define PCDNFCT1_ERROR_MEMORY_TAG 			PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT1_ERROR_MEMORY_INTERNAL 	PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT1_ERROR_NOT_FORMATED			PCDNFC_ERROR_NOT_FORMATED

/* Command List */
#define PCDNFCT1_READALL						0x00
#define PCDNFCT1_WRITE_E						0x53

/* Mask */
#define PCDNFCT1_NDEF_CAPABLE_MSK 	0xF0

/* TLV */
#define PCDNFCT1_TLV_EMPTY 					0x00
#define PCDNFCT1_TLV_LOCK 					0x01
#define PCDNFCT1_TLV_MEM	 					0x02
#define PCDNFCT1_TLV_NDEF	 					0x03

#define PCDNFCT1_NDEF_MNB						0xE1
#define PCDNFCT1_NDEF_DISABLE				0x00

#define PCDNFCT1_TOPAZ_MODE					0xA8

/* Functions */
uint8_t PCDNFCT1_ReadNDEF( void );
uint8_t PCDNFCT1_WriteNDEF( void );

#endif
