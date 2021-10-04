/**
  ******************************************************************************
  * @file    miscellaneous.h
  * @author  MMY Application Team
  * @version V1.1
  * @date    15/03/2011
  * @brief   miscellaneaous functions
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

#ifndef __RFTRANS_95HF_MISC_H
#define __RFTRANS_95HF_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
	 
#define MAX(x,y) 				((x > y)? x : y)
#define MIN(x,y) 				((x < y)? x : y)  
#define ABS(x) 					((x)>0 ? (x) : -(x))  
#define CHECKVAL(val, min,max) 	((val < min || val > max) ? false : true) 
	 
#define GETMSB(val) 		((val & 0xFF00 )>>8 ) 
#define GETLSB(val) 		( val & 0x00FF ) 
 
#define RESULTOK 							0x00 
#define ERRORCODE_GENERIC 		1 

#ifndef errchk
#define errchk(fCall) if (status = (fCall), status != RESULTOK) \
	{goto Error;} else
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_H */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
