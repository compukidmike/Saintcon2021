/**
  ******************************************************************************
  * @file    lib_nfctype3pcd.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Generates the NFC type3 commands
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
#ifndef _LIB_NFCTYPE3PCD_H
#define _LIB_NFCTYPE3PCD_H

#include "lib_iso18092pcd.h"

#define PCDNFCT3_OK 										PCDNFC_OK
#define PCDNFCT3_ERROR 									PCDNFC_ERROR
#define PCDNFCT3_ERROR_MEMORY_TAG 			PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT3_ERROR_MEMORY_INTERNAL 	PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT3_ERROR_LOCKED 					PCDNFC_ERROR_LOCKED

/* Command List */
#define PCDNFCT3_CHECK							0x06
#define PCDNFCT3_UPDATE							0x08

/* Service code */
#define PCDNFCT3_CODE_READ_MSB			0x00
#define PCDNFCT3_CODE_READ_LSB			0x0B
#define PCDNFCT3_CODE_WRITE_MSB			0x00
#define PCDNFCT3_CODE_WRITE_LSB			0x09
#define PCDNFCT3_FIRST_BLOC_MSB			0x80
#define PCDNFCT3_FIRST_BLOC_LSB			0x00

/* Flag */
#define PCDNFCT3_WRITE_ON						0x0F
#define PCDNFCT3_WRITE_OFF					0x00

/* Size */
#define PCDNFCT3_ATTR_SIZE					16

/* Functions */
uint8_t PCDNFCT3_ReadNDEF( void );
uint8_t PCDNFCT3_WriteNDEF( void );

#endif
