/**
  ******************************************************************************
  * @file    lib_wrapper.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    27-May-2015
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
#include "common.h"


uint16_t ReadData ( uint16_t Offset , uint16_t DataSize , uint8_t* pData);
uint16_t WriteData ( uint16_t Offset , uint32_t DataSize , uint8_t* pData);
		
#endif /* __LIB_WRAPPER_H */


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
