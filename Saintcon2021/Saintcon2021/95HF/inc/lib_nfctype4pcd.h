/**
  ******************************************************************************
  * @file    lib_nfctype4pcd.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Generates the NFC type4 commands
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
#ifndef _LIB_NFCTYPE4PCD_H
#define _LIB_NFCTYPE4PCD_H

#include "lib_iso7816pcd.h"

#define PCDNFCT4_OK 										PCDNFC_OK
#define PCDNFCT4_ERROR 									PCDNFC_ERROR
#define PCDNFCT4_ERROR_MEMORY_TAG				PCDNFC_ERROR_MEMORY_TAG
#define PCDNFCT4_ERROR_MEMORY_INTERNAL	PCDNFC_ERROR_MEMORY_INTERNAL
#define PCDNFCT4_ERROR_LOCKED 					PCDNFC_ERROR_LOCKED

/* Command List */
#define PCDNFCT4_SELECT_APPLI				{0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00}		
#define PCDNFCT4_CC_ID							{0xE1,0x03}

/* Size */
#define PCDNFCT4_BUFFER_READ				256

#define PCDNFCT4_ACCESS_ALLOWED			0x00

uint8_t PCDNFCT4_ReadNDEF(void);
uint8_t PCDNFCT4_WriteNDEF( void );

#endif
