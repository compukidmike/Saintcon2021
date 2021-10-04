/**
  ******************************************************************************
  * @file    lib_nfctype2picc.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the NFC type2 commands
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
#include "lib_nfctype2picc.h"

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup PICC
 * 	@{
 *	@brief  This part of the library enables PICC capabilities of ST95HF.
 */

/** @addtogroup NFC_type2_picc
 * 	@{
 *	@brief  This part of the library is used to follow NFC Type2.
 */
 
 /** @addtogroup NFCtype2_Private_Functions
 * 	@{
 */

/* memory structure with memory size equal to 2048 bytes */
extern uint8_t TT2Tag[];

/* 10 UID bytes */
/* 2 lock bytes */
//static uint8_t 	TT2InternalAndLockByte[NFCT2_MAX_CONFIG] ={0x02,0x80,0x74,0x7E,0x4A,0xEF,0x22,0x80,0x07,0x00,0x00,0x00};
//static uint8_t 	TT2InternalAndLockByte[NFCT2_MAX_CONFIG] ={0x04,0xD0,0x86,0xDA,0x1A,0x43,0x28,0x80,0xF1,0x48,0x00,0x00};
static uint8_t 	TT2InternalAndLockByte[NFCT2_MAX_CONFIG] ={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x00,0x00};

/* CC0 indicates that NDEF data is present inside the tag */
/* CC1 indicates support of version 1.0 */
/* CC2 indicates the memory size assigned to the data area, value *8 = Nb of bytes */	
/* CC3 indicates read and write access granted without any security */	
static uint8_t 	TT2CCFile[NFCT2_MAX_CC_SIZE] ={0xE1, 0x10, (NFCT2_MAX_NDEFFILE/8), 0x00};

/* TLV check T=0x03, contain NDEF message */
/* if  0<L<0xFE, L is composed of one byte */
/* if L=0xFF then L is composed of 3 bytes the last 2 bytes indicates the size of V field */
/* V is not present if L=0, for other L values, V = size indicated by L */
/* 2 TLV blocks NDEF Message TLV and terminator TLV 
 1st TLV T=0x03 NDEF Message, Length L=0x0B, V="www.st.com"
 2nd TLV T=0xFE the teminator TLV */
static uint8_t 	TT2NDEFFile[] ={0x03, 0x11, RECORD_FLAGS,TYPE_LENGTH,PAYLOAD_LENGTH,TYPE,HTTP_HEADER,'s','a','i','n','t','c','o','n','.','o','r','g',0xFE};			

/**
 * @brief  This buffer contains the data send/received by xx95HF
 */
extern uint8_t		u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

/* Flag used to write the tag into the flash */
extern bool updateFlash;

/* Flags used in TT2 functions */
uint8_t PICCNFCT2_sector = 0;
static uint8_t flag_sector_select = 0;
static uint8_t writing = 0;

/**
  * @}
  */ 

/** @addtogroup NFCtype2_Public_Functions
 * 	@{
 */

 /**
 * @brief  this function initializes the Tag Type 2 structure
 */
void PICCNFCT2_Init ( void )
{	
	/* After a RF cut we have to reset all flags */
	writing = 0;
	flag_sector_select = 0;
	PICCNFCT2_sector = 0;
	memcpy(TT2Tag,TT2InternalAndLockByte,12);
	memcpy(&TT2Tag[12], TT2CCFile,4);
	memcpy(&TT2Tag[16], TT2NDEFFile,sizeof(TT2NDEFFile));
}


/**
 * @brief  This function reads from the interface bus the command sent by the reader and returns a response
 * @param  pData : Pointer on the PICC buffer which contain the RF command
 * @retval PICCNFCT2_SUCCESSCODE : Command success
 * @retval PICCEMUL_ERRORCODE_UNKNOWNCOMMAND : Error if command sent by reader was not appropriate
 */
int8_t PICCNFCT2_ReplyCommand( uc8 *pData )
{
	uint8_t	InsCode = pData[PICC_DATA_OFFSET];
	
	switch (InsCode)
	{
		case PICCNFCT2_SECTOR_SELECT :
			PICCNFCT2_SelectSector ( pData );
		break;
		
		case PICCNFCT2_READ  :
			PICCNFCT2_Read(pData);
		break;
		
		case PICCNFCT2_WRITE  :
			PICCNFCT2_Write(pData);
		break;
			
		default:
			if (flag_sector_select == 1) /* Catches the SELECT_SECTOR packet 2 */
				PICCNFCT2_SelectSector ( pData );
			else
				return PICCNFCT2_ERRORCODE_COMMANDUNKNOWN;
	}
	
	return PICCNFCT2_SUCCESSCODE; 
}


/**
 * @brief  this function decodes the SelectSector commands and emits the response to the PCD
 * @param  pData : RF command received by the PICC
 * @retval 	PICCNFCT2_SUCCESSCODE : command succesfully executed
 */
int8_t PICCNFCT2_SelectSector (uc8 *pData)
{	
	/* SelectSector packet 1 [0xC2 0xFF] */
	if (pData[PICC_DATA_OFFSET] == PICCNFCT2_SECTOR_SELECT && pData[PICC_DATA_OFFSET+1] == 0xFF && flag_sector_select == 0)
	{
		#if NFCT2_MAX_TAGMEMORY > PICCNFCT2_SECTOR_SIZE
			flag_sector_select = 1;
			PICCNFCT2_ACK ();
		#else
			PICCNFCT2_NACK ();
		#endif
	}
	/* SelectSector packet 2 [length RFU RFU RFU] */
	else if (flag_sector_select == 1)
	{
		flag_sector_select = 0;
		if (pData[PICC_DATA_OFFSET] > 1)
			PICCNFCT2_NACK ();
		else
			PICCNFCT2_sector = pData[PICC_DATA_OFFSET];
	}
	else
		flag_sector_select = 0;

	return PICCNFCT2_SUCCESSCODE ;
}

/**
 * @brief  this function decodes the read commands and emits the response to the PCD
 * @param  pData : RF command received by the PICC
 * @retval 	PICCNFCT2_SUCCESSCODE : command succesfully executed
 * @retval 	PICCNFCT2_ERRORCODE_COMMANDUNKNOWN : the command byte is unknown
 */
int8_t PICCNFCT2_Read (uc8 *pData)
{	
	//memcpy(TT2Tag,TT2NDEFFile,sizeof(TT2NDEFFile));
	uint16_t addr;
	uint8_t PageNumber;
	uint8_t	*pDataToEmit = &(u95HFBuffer [PICC_DATA_OFFSET]);
	
	if (pData[PICC_DATA_OFFSET] != PICCNFCT2_READ)
		return PICCNFCT2_ERRORCODE_COMMANDUNKNOWN;	
	
	PageNumber = pData[PICC_DATA_OFFSET+1];
	
	addr = PageNumber*4+PICCNFCT2_sector*PICCNFCT2_SECTOR_SIZE;
	
	/* creates the card response  */
	//if( (addr+PICCNFCT2_READ_BUFFER_SIZE) > (NFCT2_MAX_TAGMEMORY - PICCNFCT2_READ_BUFFER_SIZE) )
	//{
		//if( addr < NFCT2_MAX_TAGMEMORY)
		//{
			///* or SAK case? */
			//memcpy( pDataToEmit,&TT2Tag[addr],(NFCT2_MAX_TAGMEMORY-addr));
			///* remaining byte set to 0 */
			//for (int i=0; i< PICCNFCT2_READ_BUFFER_SIZE - (NFCT2_MAX_TAGMEMORY-addr) ; i++)
			//{
				//pDataToEmit[NFCT2_MAX_TAGMEMORY-addr+i] = 0;
			//}
		//}
		//else
		//{
			///* or SAK case? */
			//for (int i=0; i< PICCNFCT2_READ_BUFFER_SIZE; i++)
			//{
				//pDataToEmit[i] = 0;
			//}
		//}
	//}
	//else
	//{
		memcpy( pDataToEmit,&TT2Tag[addr],PICCNFCT2_READ_BUFFER_SIZE);
	//}
	/* control byte append CRC + 8 bits	*/
    pDataToEmit [PICCNFCT2_READ_BUFFER_SIZE] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;

	/* emits the card response to the reader*/
	PICC_Send ((PICCNFCT2_READ_BUFFER_SIZE+1),pDataToEmit);
	
	return PICCNFCT2_SUCCESSCODE ;
	
}

/**
 * @brief  this function decodes the write commands and emits the response to the PCD
 * @param  pData : RF command received by the PICC
 * @param  FileSelected : the Id of the file selected by the Select command
 * @retval 	PICCNFCT2_SUCCESSCODE : command succesfully executed
 * @retval 	PICCNFCT2_ERRORCODE_COMMANDUNKNOWN : the command byte is unknown
  */
int8_t PICCNFCT2_Write (uc8 *pData )
{
	uint16_t addr=0;
	
	if (pData[PICC_DATA_OFFSET] != PICCNFCT2_WRITE)
		return PICCNFCT2_ERRORCODE_COMMANDUNKNOWN;
	
	addr = pData[PICC_DATA_OFFSET+1]*4 + PICCNFCT2_sector*PICCNFCT2_SECTOR_SIZE;
	
	/* Check if addr is correct */
	if (addr >= NFCT2_MAX_TAGMEMORY)
	{
		PICCNFCT2_NACK();
		return PICCNFCT2_ERRORCODE_GENERIC;
	}
	
	memcpy(&TT2Tag[addr],&pData[PICC_DATA_OFFSET+2],PICCNFCT2_WRITE_BUFFER_SIZE);
		
	/* Send an ACK */
	PICCNFCT2_ACK();
	
	/* This part allows to write the NDEF to the flash only when it is finished */
	if (addr == 16 && writing == 0)
		writing = 1;
	else if (addr == 16)
	{
		writing = 0;
		updateFlash = true;
	}
	
	return PICCNFCT2_SUCCESSCODE ;	
}

/**
 * @brief  this function sends an active ACK 1010)2 to the PCD
  */
void PICCNFCT2_ACK (void)
{
	uc8 pDataToEmit[2] = {0xAA,0x14};
	PICC_Send(2,pDataToEmit);
}

/**
 * @brief  this function sends an active NACK 0000)2 to the PCD
  */
void PICCNFCT2_NACK (void)
{
	uc8 pDataToEmit[2] = {0x00,0x14};
	PICC_Send(2,pDataToEmit);
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
