/**
  ******************************************************************************
  * @file    lib_cardemul.h
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file contains all the functions prototypes for card emul demo
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
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIB_CARDEMUL_H
#define __LIB_CARDEMUL_H

#include "lib_PICC.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
#define RFT_SUCCESSCODE									0x00	 

#define FIRMEWARE_SIZE 									(30*1024)

// state of the transfer of the file
typedef enum
{
		RFT_FILEUNKNOWN = 0,
		RFT_FILEOPEN,
		RFT_FILECLOSE,
		RFT_FILEREAD,
		RFT_FILEWRITE,
}RFT_FILESTATUS;
	 
	 
void   RFT_SelectApplication 		( uc8 *pData );
int8_t RFT_ReceiveFile 					( void );
void RFT_OperationFeedback 		  (uc8 *pData );

#ifdef __cplusplus
}
#endif

#endif /* __LIB_CARDEMUL_H */


/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */   
  
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
