/**
  ******************************************************************************
  * @file    lib_iso18092pcd.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso18092 communication
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FELICA_H
#define __FELICA_H

#include "lib_PCD.h"


/* 	-------------------------------------------------------------------------- */
#define ATQC_SIZE																				0x20
#define UID_SIZE_FELICA																	8

/* command code -------------------------------------------------------------- */
#define ISO18092_COMMAND_REQC														0x00
#define ISO18092_RESPONSE_REQC													0x01

/* code status	-------------------------------------------------------------------------- */
#define ISO18092_SUCCESSCODE														RESULTOK
#define ISO18092_ERRORCODE_DEFAULT											0xC1

typedef struct{
	uint8_t ATQC[ATQC_SIZE];
	uint8_t UID	[UID_SIZE_FELICA];
	bool 		IsDetected;
//	char		LogMsg[120];
}FELICA_CARD;

//extern FELICA_CARD 	FELICA_Card;

/* ---------------------------------------------------------------------------------
 * --- Local Functions  
 * --------------------------------------------------------------------------------- */
void 	 FELICA_Initialization( void );
int8_t FELICA_IsPresent		( void );
int8_t FELICA_CardTest			( void );
int8_t FELICA_Anticollision( void );


#endif /* __FELICA_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
