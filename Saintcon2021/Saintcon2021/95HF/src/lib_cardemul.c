/**
  ******************************************************************************
  * @file    lib_cardemul.c
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file contain the fonction that manage data transfer from
	* @brief   95HF device to MCU. The data are received during a firmeware upgrade. 
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
/* Includes ------------------------------------------------------------------*/
#include "lib_iso7816picc.h"
#include "lib_cardemul.h"

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
 
/** @addtogroup Proprietary_picc
 * 	@{
 *  @brief  This part of the library is used to implement proprietary functionalities
 *  @brief  For instance it's possible to perform a Firmeware Upgrade
 */
 
int8_t									FileStatus;
uint32_t								NbByteReceived ;
/**
 * @brief  This buffer contains the data send/received by xx95HF
 */
extern uint8_t								u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];
//uint8_t 								Firmware_Buffer[FIRMEWARE_SIZE];
uint8_t 								Firmware_Buffer[512];

uint32_t 							FileSize = 0;
uint32_t 							FileSizeReceive = 0;
uint32_t 							CRCFile = 0;
uint32_t 							CRCCalc = 0;
uint32_t 							ElapsedTimeMs = 0;
uint8_t								FILE_TRANSFER_END=0;
bool AppliFeedBackReady = false;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t											LastFrameId=0xFFFF;
uint16_t 											Offset;

/* Private function prototypes -----------------------------------------------*/

/** @addtogroup lib_userpicc_Private_Functions
 * 	@{
 *	@brief  No private function
 */
 
 /**
  * @}
  */

/** @addtogroup lib_userpicc_Public_Functions
 * 	@{
 */

/**
 * @brief  this function decodes the Select CCfile command and emits the response to the PCD
 * @param  pData : RF command received by the PICC
 * @retval 	none
 */
void RFT_SelectApplication ( uc8 *pData )
{
	uint8_t Length =0,
	
	*pDataToToEmit = &(u95HFBuffer [PICC_DATA_OFFSET]);
			
	/* creates the card response  */
	/* add the IBklock*/
	pDataToToEmit [Length++] = pData[PICC_DATA_OFFSET];
	/* add the command status */
	pDataToToEmit [Length++] = ((PICC7816_STATUS_STATUSOK & 0xFF00) >> 8);
	pDataToToEmit [Length++] = PICC7816_STATUS_STATUSOK & 0x00FF;

	/* control byte append CRC + 8 bits	*/
	pDataToToEmit [Length++] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	

	/* emits the card response to the reader*/
	PICC_Send (Length,pDataToToEmit);
}


/** 
 * @brief this function copy the RX95HF's FIFO inside internal RAM of the MCU. 
 * @brief when all data is received a flag: RFT_FILECLOSE is set.
 * @param none
 * @retval RFT_SUCCESSCODE  : the function is succcesful 
 * @retval RFT_ERROR_DEFAULT  : the function is not succcesful 
*/
int8_t RFT_ReceiveFile (void)
{
	uint8_t 	*DataBuffer 	= &(u95HFBuffer		[PICC_DATA_OFFSET + ISO7816_ADPUOFFSET_DATA]),
						LCbyte 				= 	u95HFBuffer		[PICC_DATA_OFFSET + ISO7816_ADPUOFFSET_LC],
						pDataToSend [4],
						AppendFile =0;

	Offset = *(uint16_t *)(u95HFBuffer + PICC_DATA_OFFSET + ISO7816_ADPUOFFSET_P1);

	switch (Offset)
 {
	  /* open the file */
		case 0x0000: 
			/* intialization of the global variables */
			FileStatus = RFT_FILEUNKNOWN;
		
			if(LCbyte==0x04)
			{
				FileStatus = RFT_FILEWRITE;
				pDataToSend[0] = (uint8_t) 0;
				pDataToSend[1] = (uint8_t) 0;
				pDataToSend[2] = (uint8_t) GETLSB(LastFrameId);
				pDataToSend[3] = (uint8_t) GETMSB(LastFrameId);
				AppendFile = 1;
			}		
			else
			{
				NbByteReceived = 0;
				FileSizeReceive = 0;
				FileStatus = RFT_FILEWRITE;
			}
			break;

		/* close the file */
		case 0xFFFF: 
			FileStatus = RFT_FILECLOSE;
			LastFrameId = 0xFFFF;
			break;

		/* data transfer */
		default :					
			FileStatus = RFT_FILEWRITE;
			break;
	}
	
	if( (Offset==0 && LCbyte==0x04)  || Offset == 0xFFFF)
	{
		/* nothing to retrieve, information to know where we are in the download or end of transfer */
	}
	else /* retrieve data from PCD */
	{		
		/* this is to manage case where PCD believes there was an error and resend the data */ 
		if(	LastFrameId != Offset)
		{
			memcpy( &Firmware_Buffer[NbByteReceived] , DataBuffer , LCbyte);
			LastFrameId = Offset;
			NbByteReceived +=LCbyte;
			FileSizeReceive = NbByteReceived;
		}
	}
	 
	if (AppendFile==1)
	{
		PICC7816_SendBuffer ( u95HFBuffer , pDataToSend ,4,PICC7816_STATUS_STATUSOK );
		return RFT_SUCCESSCODE;
	}
	else
	{
		PICC7816_SendAStatusCode ( u95HFBuffer , PICC7816_STATUS_STATUSOK );
		return RFT_SUCCESSCODE;
	}

}


/** 
 * @brief this function copy the RX95HF's FIFO inside internal RAM of the MCU. 
 * @brief when all data is received a flag: RFT_FILECLOSE is set.
 * @retval RFT_SUCCESSCODE  : the function is succcesful 
 * @retval RFT_ERROR_DEFAULT  : the function is not succcesful 
*/
void RFT_OperationFeedback (uc8 *pData )
{
	uint8_t Length =0,
	
	*pDataToToEmit = &(u95HFBuffer [PICC_DATA_OFFSET]);
			
	/* creates the card response  */
	/* add the IBklock*/
	pDataToToEmit [Length++] = pData[PICC_DATA_OFFSET];
	/* add the command status */
	if( AppliFeedBackReady)
	{	
		pDataToToEmit [Length++] = ((PICC7816_WARNING_NOINFORMATION & 0xFF00) >> 8);
		pDataToToEmit [Length++] = PICC7816_WARNING_NOINFORMATION & 0x00FF;
	}
	else
	{	
		pDataToToEmit [Length++] = ((PICC7816_STATUS_STATUSOK & 0xFF00) >> 8);
		pDataToToEmit [Length++] = PICC7816_STATUS_STATUSOK & 0x00FF;
	
		/* add CRC info */
		pDataToToEmit [Length++] = ((CRCCalc & 0xFF000000) >> 24);
		pDataToToEmit [Length++] = ((CRCCalc & 0x00FF0000) >> 16);
		pDataToToEmit [Length++] = ((CRCCalc & 0x0000FF00) >> 8);
		pDataToToEmit [Length++] = ((CRCCalc & 0x000000FF) );
		
		/* add Time info */
		pDataToToEmit [Length++] = ((ElapsedTimeMs & 0xFF000000) >> 24);
		pDataToToEmit [Length++] = ((ElapsedTimeMs & 0x00FF0000) >> 16);
		pDataToToEmit [Length++] = ((ElapsedTimeMs & 0x0000FF00) >> 8);
		pDataToToEmit [Length++] = ((ElapsedTimeMs & 0x000000FF) );
		
		/* add File size */
		pDataToToEmit [Length++] = ((FileSizeReceive & 0xFF000000) >> 24);
		pDataToToEmit [Length++] = ((FileSizeReceive & 0x00FF0000) >> 16);
		pDataToToEmit [Length++] = ((FileSizeReceive & 0x0000FF00) >> 8);
		pDataToToEmit [Length++] = ((FileSizeReceive & 0x000000FF) );
	}
	
	/* control byte append CRC + 8 bits	*/
	pDataToToEmit [Length++] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	

	/* emits the card response to the reader*/
	PICC_Send (Length,pDataToToEmit);
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


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
