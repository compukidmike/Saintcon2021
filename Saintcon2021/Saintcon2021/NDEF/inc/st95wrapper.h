/**
  ******************************************************************************
  * @file    st95wrapper.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    25-February-2014
  * @brief   This file help to have upper layer independent from HW
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  ******************************************************************************  
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIB_WRAPPER_H
#define __LIB_WRAPPER_H

/* Includes ------------------------------------------------------------------*/
#include "lib_95HF.h"

/* The maximum size of a NDEF will be 64kBits */
/* if smaller memory used, update this define to save space */
#define NDEF_MAX_SIZE								NFCT4_MAX_NDEFMEMORY`

#define NDEF_ACTION_COMPLETED				XX95_ACTION_COMPLETED

/* Error codes for Higher level */
#define NDEF_OK 										RESULTOK
#define NDEF_ERROR 									ERRORCODE_GENERIC
#define NDEF_ERROR_MEMORY_TAG 			2
#define NDEF_ERROR_MEMORY_INTERNAL 	3
#define NDEF_ERROR_LOCKED 					4
#define NDEF_ERROR_NOT_FORMATED			5

/* Wrapper to have upper layer independent from HW */

#define ReadData										XX95HF_ReadData	
#define WriteData										XX95HF_WriteData							

u16 XX95HF_ReadData ( u16 Offset , u16 DataSize , u8* pData);
u16 XX95HF_WriteData ( u16 Offset , u32 DataSize , u8* pData);
		
#endif /* __LIB_WRAPPER_H */


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
