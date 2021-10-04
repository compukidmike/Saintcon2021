/**
  ******************************************************************************
  * @file    lib_iso14443Apicc.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso14443A communication in the card emulator mode.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
	******************************************************************************
  */ 
#include "lib_iso14443Apicc.h"

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup PICC
 * 	@{
 *	@brief  This part of the library enables PICC capabilities of RX95HF & ST95HF.
 */ 
 

 /** @addtogroup ISO14443A_picc
 * 	@{
 *	@brief  This part of the library is used to follow ISO14443A. 
 */

static ISO14443A_CARD 				ISO14443A_Card; 

uint8_t TimeSlot = 0;

 /* extern variables ---------------------------------------------------------------- */
/**
 * @brief  This buffer contains the data send/received by xx95HF
 */
extern uint8_t								u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

/** @addtogroup lib_iso14443Apicc_Private_Functions
 * 	@{
 *	@brief  No private function
 */

 /**
  * @}
  */ 

/** @addtogroup lib_iso14443Apicc_Public_Functions
 * 	@{
 */

/**
 * @brief  this function decodes and returns a response to RATS command
 * @param  pData : RF command received by 95HF device
 * @retval 	LIB14443APICC_SUCESSSCODE : function succesful
 * @retval 	LIB14443APICC_ERRORCODE_CODECOMMAND : the command code doesn't match with this function
 * @retval 	LIB14443APICC_ERRORCODE_RATSCOMMAND	: 95HF device returns an error code
 */
int8_t PICC14A_RATS( uc8 *pData )
{
	uint8_t 	Length =0,
						*pDataToToEmit = &(u95HFBuffer [PICC_DATA_OFFSET]);
	int8_t 		status;

	if (pData[PICC_DATA_OFFSET] != COMMAND_RATS)
		return LIB14443APICC_ERRORCODE_CODECOMMAND;  
	
	/* save the RF commmand parameters */
	ISO14443A_Card.CID = pData[PICC_DATA_OFFSET+1] & ISO14443A_CID_MASK; 	
	ISO14443A_Card.FSDI = (pData[PICC_DATA_OFFSET+1] & ISO14443A_FSDI_MASK)>>4; 

	/* creates the card response */
	/* 06 78 B3 81 00 80 02F0 */
	pDataToToEmit [Length++] = LIB14443APICC_TL;	   		/* Tl : number of bytes */
	pDataToToEmit [Length++] = LIB14443APICC_T0;	   		/* T0 : format */
	pDataToToEmit [Length++] = LIB14443APICC_TA1;	    	/* TA1 : datarates */
	pDataToToEmit [Length++] = LIB14443APICC_TB1;	    	/* TB1 : timing */
	pDataToToEmit [Length++] = LIB14443APICC_TC1;	    	/* TC1 : interface */

	pDataToToEmit [Length++] = 0x80;	   

	/* control byte append CRC + 8 bits */
	pDataToToEmit [Length++] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 
	
	/* add a delay of 80us*/
	delay_us(80);
	/* emits the response */
	errchk(PICC_Send (Length,pDataToToEmit));

	return LIB14443APICC_SUCESSSCODE ;
Error:
	return LIB14443APICC_ERRORCODE_RATSCOMMAND;

}

/**
 * @brief  this function decodes and returns a response to PPS command
 * @param  pData : RF command received by 95HF device
 * @retval 	LIB14443APICC_SUCESSSCODE : function succesful
 * @retval 	LIB14443APICC_ERRORCODE_CODECOMMAND : the command code doesn't match with this function
 * @retval 	LIB14443APICC_ERRORCODE_RATSCOMMAND	: 95HF device returns an error code
 */
 
int8_t PICC14A_PPS( uc8 *pData )
{
	uint8_t Length =0, ParametersByte =0,
					*pDataToToEmit = &(u95HFBuffer [PICC_DATA_OFFSET]);
	int8_t 	status;

	if (pData[PICC_DATA_OFFSET] != COMMAND_PPS)
		return LIB14443APICC_ERRORCODE_CODECOMMAND;  
	
	/* check the CID value */
	if (pData[PICC_DATA_OFFSET] & ISO14443A_CID_MASK != ISO14443A_Card.CID)
		return LIB14443APICC_ERRORCODE_CODECOMMAND ;
	
 	/* save the RF commmand parameters */
	if ( (pData[PICC_DATA_OFFSET+1] & 0x10) == 0x10)
	{
		ISO14443A_Card.DSI = (pData[PICC_DATA_OFFSET+2]& 0x0C) >> 2; 
		ISO14443A_Card.DRI = pData[PICC_DATA_OFFSET+2]& 0x03; 
		
		ParametersByte = (pData[PICC_DATA_OFFSET+2] & 0x0F)<< 4 | 0x0A;
	}
	
	/* creates the card response */
	/* 0xD0 CRC */
	pDataToToEmit [Length++] = COMMAND_PPS;	   
	/* control byte append CRC + 8 bits	*/
	pDataToToEmit [Length++] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 
		
	/* emits the response */
	errchk(PICC_Send (Length,pDataToToEmit));
		
	/* Modify the config according to the received parameters */
	/* send a Select command to configure 95HF device as a card emulator*/
	errchk(PICC_ProtocolSelect(0x02,PICC_PROTOCOL_ISO14443A,&(ParametersByte),u95HFBuffer));
	
	return LIB14443APICC_SUCESSSCODE ;
Error:
	return LIB14443APICC_ERRORCODE_RATSCOMMAND;

}


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
 
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

