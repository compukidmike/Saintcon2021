/**
  ******************************************************************************
  * @file    lib_nfctype4picc.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the NFC type4 commands
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
	
#include "lib_nfctype4picc.h"
#include "lib_cardemul.h" /* to add proprietary application */

#define ST95HF

#ifdef ST95HF	
#include "lib_proprietaryP2P.h" /* to add support of proprietary P2P */
#endif

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

/** @addtogroup NFC_type4_picc
 * 	@{
 *	@brief  This part of the library is used to follow NFC Type4.
 */
 
 /** @addtogroup NFCtype4_Private_Functions
 * 	@{
 */
static void 		PICCNFCT4_Select 				( uc8 *pData);
static int8_t 	PICCNFCT4_IsReadWriteParametersOk ( uc8 FileSelected , uc8 FileOffset , uc8 NbByteToRead  );

PICCNFCT4_SELECTCMD_APPL		ApplicationSelected = PICCNFCT4_APPLI_UNKNOWN;
static uint8_t		FileSelected = NDEFFILE_SELECTED_NONE;

/* Variables for the different modes */
extern DeviceMode_t devicemode;
extern TagType_t nfc_tagtype;

/**
 * @brief  This buffer contains the data send/received by xx95HF
 */
extern uint8_t    u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

extern bool				ISO14443A_TT4Used;

/**
 * @brief  This variable contains the Capability Container file of the NFC tag type 4 we emulate
 */	
extern uint8_t CardCCfile[];
#define CC_INIT_LENGTH 16
uc8 CardCCfileInit[] = {CCLEN_UB, CCLEN_LB, MAPVER, MLE_UB, MLE_LB, MLC_UB, MLC_LB, TLV_T, TLV_L,
 															 TLV_V_FILEID_UB, TLV_V_FILEID_LB, TLV_V_FILESIZE_UB, TLV_V_FILESIZE_LB,
 															 TLV_V_FILEREADACCESS, TLV_V_FILEWRITEACCESS};

/**
* @brief  this variable contains the NDEF file of the NFC tag type 4 URL www.st.com
*/
extern uint8_t CardNDEFfileT4A[];
extern uint8_t CardNDEFfileT4B[];

															 
/* Flag used to write the tag into the flash */
extern bool updateFlash;

/* private variables ---------------------------------------------------------------- */
extern uint32_t 							FileSize;
extern uint32_t 							CRCFile;
extern uint8_t								FILE_TRANSFER_END;		
																 
/**
 * @brief  this function returns the idenfier of the received command
 * @param  FileSelected : the Id of the file selected
 * @param  FileOffset : the addres of the first byte
 * @param  NbByteToRead : number of byte to read
 * @retval 	PICCNFCT4_SUCCESSCODE : the receiving command is a Select CC file 
 * @retval 	PICCNFCT4_ERRORCODE_GENERIC : the receiving command is a Select Application
 */
static int8_t PICCNFCT4_IsReadWriteParametersOk ( uc8 FileSelected ,uc8 FileOffset , uc8 NbByteToRead  )
{
	/* Check the file offset and the number of byte to read parameters. */
	switch (FileSelected)
	{
		case NDEFFILE_SELECTED_CCFILE: 
				if (FileOffset + NbByteToRead > NFCT4_MAX_CCMEMORY) 
				{	
					return PICCNFCT4_ERRORCODE_GENERIC	;
				}
		break;
		case NDEFFILE_SELECTED_NDEFFILE1: 
				if (FileOffset + NbByteToRead > NFCT4_MAX_NDEFMEMORY) 
				{	
					return 	PICCNFCT4_ERRORCODE_GENERIC	;
				}
		break;
	}
	return PICCNFCT4_SUCCESSCODE;
}

/**
  * @}
  */ 

/** @addtogroup NFCtype4_Public_Functions
 * 	@{
 */

 /**
 * @brief  this function initiliazes the tag type 4
 */
void PICCNFCT4_Init ( PICCNFCT4_SELECT_TYPE SelectedType )
{			
	memcpy(CardCCfile, CardCCfileInit, CC_INIT_LENGTH);
}

 /**
 * @brief  this function uninitiliazes the tag type 4
 */
void PICCNFCT4_DeInit ( PICCNFCT4_SELECT_TYPE SelectedType )
{		

}

/**
 * @brief  This function reads from the interface bus the command sent by the reader and returns a response
 * @param  pData : Pointer on the PICC buffer which contain the RF command
 * @retval Error if command sent by reader was not appropriate, succes in other cases
 */
int8_t PICCNFCT4_ReplyCommand( uc8 *pData )
{
	uint8_t	State[] = {0x80,0x00},
					InsCode = pData[PICC_DATA_OFFSET];
	
	/* Is it a specific command TT4A command? */
	if( ISO14443A_TT4Used == true)
	{
		//if(InsCode == 0xF0) /* NFC-A reserved for P2P*/
			//return PICCNFCT4_ERRORCODE_GENERIC;
		
		switch (InsCode)
		{		
			/********************************************************/
			/*** ISO14443A Anticollision is managed automatically ***/
			/********************************************************/			
			case COMMAND_RATS  :
				PICC14A_RATS ( pData );
			break;
		
			case COMMAND_PPS  :
				PICC14A_PPS ( pData );
			break;
			
			case COMMAND_SBLOCK :
				PICC7816_Deselect ( pData );
				/* set the PICC to the Idle state */
				PICC_AcFilter (0x01,State, 0x00,0x00,u95HFBuffer);
			break;
	
			default:
				goto Common;
		}
	}
	
	return PICCNFCT4_SUCCESSCODE; 
	
	/* Is it a Common command? */
Common:	
		
	switch (InsCode)
	{		
		case COMMAND_IBLOCK02 :
		case COMMAND_IBLOCK03 :				
			/* check the first control byte  (the CRC and the number of the last bytes) */
			if( (pData[pData[PICC_LENGTH_OFFSET] + PICC_DATA_OFFSET -1 ] != 0x00 && !ISO14443A_TT4Used) ||
				  (pData[pData[PICC_LENGTH_OFFSET] + PICC_DATA_OFFSET -1 ] != 0x08 && ISO14443A_TT4Used)  )
			{
#ifdef ISO14443_SENDERRORCODE_ONCRC
				PICC7816_SendAStatusCode( pData, LIB14443APICC_NFCTYPE4_TRANSMISSIONERORCODE );	
#endif
				return PICCNFCT4_ERRORCODE_GENERIC;
			}
			
			if(pData[PICC_DATA_OFFSET+1]==0xA2)
			{
				switch(pData[PICC_DATA_OFFSET+2])
				{						
					case 0x41:
						FileSize   = *(uint16_t *)(u95HFBuffer + PICC_DATA_OFFSET +6);
						PICC7816_SendAStatusCode ( u95HFBuffer , PICC7816_STATUS_STATUSOK );
						break;
					case 0x42:
						CRCFile= *(uint32_t *)(u95HFBuffer + PICC_DATA_OFFSET +6);
						PICC7816_SendAStatusCode ( u95HFBuffer , PICC7816_STATUS_STATUSOK );
						break;
					case 0x43:
						RFT_OperationFeedback(pData);
						break;
					case 0x44: /* long file management */
						FileSize   = *(uint32_t *)(u95HFBuffer + PICC_DATA_OFFSET +6);
						PICC7816_SendAStatusCode ( u95HFBuffer , PICC7816_STATUS_STATUSOK );
						break;
					case 0xFE:
						FILE_TRANSFER_END=1;
						PICC7816_SendAStatusCode ( u95HFBuffer , PICC7816_STATUS_STATUSOK );
						break;
					
#ifdef ST95HF					
					case CMD_INS: 		// Receive a CMD
						PP2P_GetCMD((uint8_t *)pData);
						break;
					case DAT_INS:	 	// Receive a DATA
						PP2P_GetData((uint8_t *)pData);
						break;
					case BUF_INS:		// Receive a Buf 
						PP2P_GetBuf((uint8_t *)pData);
						break;
					case RTOK_INS:		// Receive the Token
						PP2P_GetPreemptionToken(pData[PICC_DATA_OFFSET]);
						break;
					case SYM_INS:		// Receive a Send SYM
						PP2P_GetSYM(pData[PICC_DATA_OFFSET]);
						break;
					case PTOK_INS:		// Token Preemption Message
						PP2P_ReleaseCommunicationToken(pData[PICC_DATA_OFFSET]);
						break;				
#endif /* ST95HF */	
					
					default :
						PICC7816_SendAStatusCode ( pData, PICC7816_STATUS_INSTRUCTIONCODENOTSUPPORTED );	
						break;
				}
			}
#ifdef ST95HF				
			else if ((pData[PICC_DATA_OFFSET+1]==0x90)&&(pData[PICC_DATA_OFFSET+2]==0x00))	// ACK received
			{
				PP2P_GetACK(pData[PICC_DATA_OFFSET]);
			}
			else if ((pData[PICC_DATA_OFFSET+1]==0x6F)&&(pData[PICC_DATA_OFFSET+2]==0x00))
			{
				PP2P_GetNACK(pData[PICC_DATA_OFFSET]);
			}
#endif /* ST95HF */			
			else
			{		
				if ( (pData[PICC_DATA_OFFSET+1]==0x00))
				{
					/* FGR Do not answer to quickly */
					delay_ms(1);
					/* according to the INS (instruction) byte */
					switch (pData[PICC_DATA_OFFSET+2])
					{
						/* read command */
						case  PICCNFCT4_READCMD : 
							PICCNFCT4_ReadBinary ( pData );	
							break;
						/* update command */
						case PICCNFCT4_UPDATECMD :
							PICCNFCT4_UpdateBinary ( pData );	
							break;
						/* Select command */
						case PICCNFCT4_SELECTCMD :
							PICCNFCT4_Select ( pData );
							break;
						default :
							PICC7816_SendAStatusCode ( pData, PICC7816_STATUS_INSTRUCTIONCODENOTSUPPORTED );	
							break;
					} /* switch*/
				}		
				else
					PICC7816_SendAStatusCode ( pData, PICC7816_STATUS_CLASSNOTSUPPORTED );	
			}
			break;
			
			case COMMAND_NACKBLOCK_B2 :
			case COMMAND_NACKBLOCK_B3 :
				PICC7816_ACK ( pData );
			break;
			
			case COMMAND_ACKBLOCK_A2 :
			case COMMAND_ACKBLOCK_A3 :
				PICC7816_SendAStatusCode ( pData, PICCEMUL_NFCTYPE4_DEBUG );	
				break;
			
			default:
				break;
		}/* switch */
	
		/* Is it a private protocol command */		
	return PICCNFCT4_SUCCESSCODE; 
}

/**
 * @brief  this function returns the idenfier of the received command
 * @param  pData : RF command received by 95HF device
 * @retval 	PICCPROTOCOL_SELECTCMD_SELECTCCFILE : the receiving command is a Select CC file 
 * @retval 	PICCPROTOCOL_SELECTCMD_SELECTAPPLICATION : the receiving command is a Select Application
 * @retval 	PICCPROTOCOL_SELECTCMD_SELECTNDEFFILE	: the receiving command is a Select NDEF file
 * @retval 	PICCPROTOCOL_SELECTCMD_UNKNOWN	: Command unknown
 */
static int8_t PICCNFCT4_GetCommandId (uc8 *pData)
{
	uint16_t FileIdentifier=0x0000;	
	uint8_t		applicationFieldFWU[17] = {0xF0,0x02,0x46,0x57,0x55,0x5F,0x58,0x58,	
																			 0x4F,0x5F,0x76,0x30,0x00,0x00,0x00,0x00};
																			 
	if (pData[PICC_DATA_OFFSET+1] != PICC7816_NFCTYPE4_CLASS)
		return LIB14443APICC_ERRORCODE_COMMANDUNKNOWN;


	else if (pData[PICC_DATA_OFFSET+2] == PICC7816_NFCTYPE4_SELECTCMD)
	{
		FileIdentifier = (uint16_t) ( ((pData[PICC_DATA_OFFSET+6])<<8) | (pData[PICC_DATA_OFFSET+7]) ); 
		
		/* catch the file idenfifier */
		switch (FileIdentifier)
		{			
			case PICCEMUL_NFCTYPE4_CCFILE: 
				return PICCPROTOCOL_SELECTCMD_SELECTCCFILE;
			
			case PICCEMUL_NFCTYPE4_NDEFFILE: 
				return PICCPROTOCOL_SELECTCMD_SELECTNDEFFILE;

			case PICCEMUL_NFCTYPE4_NDEFAPPLICATION: 
				return PICCPROTOCOL_SELECTCMD_SELECTNFCT4APPLICATION;						
						
			case PICCEMUL_NFCTYPE4_STPROPRIETARY:
					if (!strncmp((char *)(&(pData[PICC_DATA_OFFSET+ISO7816_ADPUOFFSET_DATA])),(char *) (applicationFieldFWU),16) )
						return PICCPROTOCOL_SELECTCMD_FT;	
	
			default : 
				return PICCPROTOCOL_SELECTCMD_UNKNOWN;
		}
	}
		 
	return LIB14443APICC_ERRORCODE_COMMANDUNKNOWN;		

}

/**
 * @brief  this function decodes and returns a response to Select commands
 * @param  pData : RF command received by 95HF device
 */
static void PICCNFCT4_Select ( uc8 *pData)
{	
	switch (PICCNFCT4_GetCommandId ( pData ))
	{			
		case PICCPROTOCOL_SELECTCMD_SELECTNFCT4APPLICATION: 
			PICCNFCT4_SelectApplication ( pData );		
			break;	

		case PICCPROTOCOL_SELECTCMD_SELECTCCFILE: 
			PICCNFCT4_SelectCCfile ( pData );	
			break;	
		
		case PICCPROTOCOL_SELECTCMD_SELECTNDEFFILE: 
			PICCNFCT4_SelectNDEFfile( pData );	
			break;
			
		case PICCPROTOCOL_SELECTCMD_FT: 
			RFT_SelectApplication ( pData );	
			ApplicationSelected = PICCNFCT4_APPLI_FWU; 
			break;
	
		default:
			PICC7816_SendAStatusCode ( pData, PICC7816_STATUS_COMMANDNOTALLOWED );
			ApplicationSelected =	PICCNFCT4_APPLI_UNKNOWN;
			break;	
	}
}

/**
 * @brief  this function decodes the Select Application command and emits the response to the PCD 
 * @param  pData : RF command received by 95HF device
 * @retval 	None
 */
void PICCNFCT4_SelectApplication( uc8 *pData )
{
	ApplicationSelected = PICCNFCT4_APPLI_NFCTYPE4; 
	 	
	/* emits the card response to the reader*/
	PICC7816_SendAStatusCode(pData, PICCNFCT4_STATUS_STATUSOK);
}

/**
 * @brief  this function decodes the Select CCfile command and emits the response to the PCD
 * @param  pData : RF command received by 95HF device
 * @retval 	None
 */
void PICCNFCT4_SelectCCfile( uc8 *pData )
{
	FileSelected = NDEFFILE_SELECTED_CCFILE;
	
	/* emits the card response to the reader*/
	PICC7816_SendAStatusCode(pData, PICCNFCT4_STATUS_STATUSOK);
}


/**
 * @brief  this function decodes the Select NDEF file command and emits the response to the PCD
 * @param  pData : RF command received by 95HF device
 */
void PICCNFCT4_SelectNDEFfile( uc8 *pData )
{
	FileSelected = NDEFFILE_SELECTED_NDEFFILE1;
	
	/* emits the card response to the reader*/
	PICC7816_SendAStatusCode(pData, PICCNFCT4_STATUS_STATUSOK);
}

/**
 * @brief  this function decodes the readbinary commands and emits the response to the PCD
 * @param  pData : RF command received by the PICC
 * @retval 	PICCNFCT4_SUCCESSCODE : the receiving command is a Select CC file 
 * @retval 	PICCNFCT4_ERRORCODE_COMMANDUNKNOWN : the command byte is unknown
 * @retval 	PICCNFCT4_ERRORCODE_PARAMETER	: one parameter is erroneous
 * @retval 	PICCNFCT4_ERRORCODE_GENERIC	: the function is not succesful
 */
int8_t PICCNFCT4_ReadBinary (uc8 *pData )
{
	uint16_t 	FileOffset  = ((pData[PICC_DATA_OFFSET+3]<<8 & 0xFF00) | pData[PICC_DATA_OFFSET+4]);
	uint8_t		*pFile,
						NbByteToRead = pData[PICC_DATA_OFFSET+5];
	int8_t 		status;

	if (pData[PICC_DATA_OFFSET+2] != PICCNFCT4_READCMD)
		return PICCNFCT4_ERRORCODE_COMMANDUNKNOWN;
	
	if (FileSelected == NDEFFILE_SELECTED_NONE)
		return PICCNFCT4_ERRORCODE_PARAMETER;	
	
		/* check the parameter of the read binary command */
	errchk(PICCNFCT4_IsReadWriteParametersOk (FileSelected , FileOffset , NbByteToRead  ));
	
	switch (FileSelected)
	{
		case NDEFFILE_SELECTED_CCFILE: 
				pFile = CardCCfile;
		break;
		case NDEFFILE_SELECTED_NDEFFILE1: 
				if (nfc_tagtype == TT4A)
					pFile = CardNDEFfileT4A;
		break;
		default:
				PICC7816_SendAStatusCode( pData, PICCNFCT4_STATUS_FILENNOTFOUND );
			return PICCNFCT4_SUCCESSCODE;
	}
	
	/* emits the card response to the reader*/
	PICC7816_SendBuffer(pData, &pFile[FileOffset], NbByteToRead, PICCNFCT4_STATUS_STATUSOK);
	 
	return PICCNFCT4_SUCCESSCODE;		
Error:
	PICC7816_SendAStatusCode( pData, PICCNFCT4_STATUS_APPLICATIONNOTFOUND );
	return PICCNFCT4_ERRORCODE_PARAMETER;
}

/**
 * @brief  this function decodes the updatebinary commands and emits the response to the PCD
 * @param  pData : RF command received by the PICC
 * @param  FileSelected : the Id of the file selected by the Select command
 * @retval 	PICCNFCT4_SUCCESSCODE : the receiving command is a Select CC file 
 * @retval 	PICCNFCT4_ERRORCODE_COMMANDUNKNOWN : the command byte is unknown
 * @retval 	PICCNFCT4_ERRORCODE_PARAMETER	: one parameter is erroneous
 * @retval 	PICCNFCT4_ERRORCODE_GENERIC	: the function is not succesful
  */
int8_t PICCNFCT4_UpdateBinary (uc8 *pData )
{
	uint16_t 	FileOffset  = (pData[5]<<8 & 0xFF00) | pData[6];
	uint8_t		*pFile,
						NbByteToUpdate = pData[7];
	int8_t		status;
	
	if (FileSelected == NDEFFILE_SELECTED_NONE)
		return PICCNFCT4_ERRORCODE_COMMANDUNKNOWN; /* fgr to change error when State Machine will be added */
			
	if (pData[PICC_DATA_OFFSET+2] != PICCNFCT4_UPDATECMD)
		return PICCNFCT4_ERRORCODE_COMMANDUNKNOWN;
	
	/* according to the Application selected by the SelectApplication command */
	if( ApplicationSelected == PICCNFCT4_APPLI_FWU  )
	{		
		return RFT_ReceiveFile ();
	}
	
	/* check the parameter of the read binary command */
	errchk(PICCNFCT4_IsReadWriteParametersOk (FileSelected , FileOffset , NbByteToUpdate  ));
		
	/* Check the file offset and Number of byte to read parameters.	*/
	switch (FileSelected)
	{
		case NDEFFILE_SELECTED_CCFILE: 
			pFile = CardCCfile;
			if (FileOffset + NbByteToUpdate > NFCT4_MAX_CCMEMORY) 
			{	
				PICC7816_SendAStatusCode( pData, PICCNFCT4_STATUS_WRONGP1P2PARAMETERS );
				return PICCNFCT4_ERRORCODE_GENERIC	;
			}
		break;
			
		case NDEFFILE_SELECTED_NDEFFILE1: 
			if (nfc_tagtype == TT4A)
				pFile = CardNDEFfileT4A;			
			if (FileOffset + NbByteToUpdate > NFCT4_MAX_NDEFMEMORY) 
			{	
				PICC7816_SendAStatusCode( pData, PICCNFCT4_STATUS_WRONGP1P2PARAMETERS );
				return PICCNFCT4_ERRORCODE_GENERIC	;
			}
		break;			
			
		default:
			PICC7816_SendAStatusCode( pData, PICCNFCT4_STATUS_FILENNOTFOUND );
			return PICCNFCT4_SUCCESSCODE;	
	}

	/* update the NDEF file  */ 
	memcpy(&(pFile[FileOffset]),&(pData [8]), NbByteToUpdate);
	
	/* Send the response */
	PICC7816_SendBuffer(pData,NULL,0,PICCNFCT4_STATUS_STATUSOK);

#ifdef TAG_MANAGEMENT	
	/* Write to flash only when the size is written (last updatebinary command)*/
	if (FileOffset == 0 && !(pData[8] == 0 && pData[9] == 0))
	{
		/* Write into flash */
		if (FileSelected == NDEFFILE_SELECTED_NDEFFILE1)
		{
			updateFlash = true;
		}
	}
#endif /* TAG_MANAGEMENT */
	
	return PICCNFCT4_SUCCESSCODE	;		
Error :
	PICC7816_SendAStatusCode( pData, PICCNFCT4_STATUS_WRONGP1P2PARAMETERS );
	return PICCNFCT4_ERRORCODE_PARAMETER	;		

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
