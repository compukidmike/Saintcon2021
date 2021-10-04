/**
  ******************************************************************************
  * @file    lib_PICC.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides set of firmware functions to manages picc device. 
  * @brief   The commands as defined in RX95HF or ST95HF datasheet
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

/* Includes ------------------------------------------------------------------------------ */
#include "lib_PICC.h"

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


/** @addtogroup PICC_Device
 * 	@{
 *	@brief  This set of function use the device commands to control both RX95HF & ST95HF <br />
 *					This layer is the bridge beetween the library and the 95HF driver. 
 */

extern uint8_t				u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

#ifdef USE_MSD_DRIVE
FATFS ff;      /* Work area (file system object) for logical drives */
FIL fdft;      /* file objects */
extern FRESULT res;
extern UINT bw, br;
#endif /*USE_MSD_DRIVE*/


/** @addtogroup PICC_Device_Private_Functions
 *  @{
 */

static uint8_t IsAnAvailableProtocol 		(uint8_t Protocol);

/**
 *	@brief  this functions returns PICC_SUCCESSCODE if the protocol is available, otherwise PICC_ERRORCODE_PARAMETER
 *  @param  Protocol : RF protocol (ISO/IEC 14443 A or 14443 B or 15693 or Fellica)
 *  @retval PICC_SUCCESSCODE	: the protocol is available
 *  @retval PICC_ERRORCODE_PARAMETER : the protocol isn't available
 */
static uint8_t IsAnAvailableProtocol (uint8_t Protocol) 
{
	switch(Protocol)
	{
		case PICC_PROTOCOL_FIELDOFF:
			return PICC_SUCCESSCODE;
		case PICC_PROTOCOL_ISO14443A:
			return PICC_SUCCESSCODE;
		case PICC_PROTOCOL_ISO14443B:
			return PICC_SUCCESSCODE;
		case PICC_PROTOCOL_ISO18092:
			return PICC_SUCCESSCODE;
		
		default: return PICC_ERRORCODE_PARAMETER;
	}	
}

/**
  * @}
  */

/** @addtogroup PICC_Device_Public_Functions
 *  @{
 */

/**
 *	@brief  This function sends POR sequence. It can be use to initialize PICC after a POR.
 *  @retval PICC_ERRORCODE_PORERROR : the POR sequence doesn't succeded
 *  @retval PICC_SUCCESSCODE : PICC is ready
 */
int8_t PICC_PORsequence( void )
{
	uint16_t NthAttempt=0;
	uc8 command[]= {ECHO};

	if(drv95HF_GetSerialInterface() == RFTRANS_95HF_INTERFACE_SPI)
	{
		drv95HF_ResetSPI();		
	}
	
	do{
	
		/* send an ECHO command and checks response */
		drv95HF_SendReceive(command, u95HFBuffer);

		if (u95HFBuffer[0]==ECHORESPONSE)
			return PICC_SUCCESSCODE;	

		/* if the SPI interface is selected then send a reset command*/
		if(drv95HF_GetSerialInterface() == RFTRANS_95HF_INTERFACE_SPI)
		{	
			drv95HF_ResetSPI();				
		}
#ifdef USE_CR95HF_DEVICE		
		/* if the UART interface is selected then send 255 ECHO commands*/
		else if(drv95HF_GetSerialInterface() == RFTRANS_95HF_INTERFACE_UART)
		{
			do {
				/* send an ECHO command and checks response */
				drv95HF_SendReceive(command, u95HFBuffer);
				if (u95HFBuffer[0] == ECHORESPONSE)
					return PICC_SUCCESSCODE;	
			}while(NthAttempt++ < RFTRANS_95HF_MAX_ECHO);
		}
#endif /* USE_CR95HF_DEVICE */
	} while (u95HFBuffer[0]!=ECHORESPONSE && NthAttempt++ <2);

	return PICC_ERRORCODE_PORERROR;
}

/**
 *	@brief  this function sends Echo command to the PICC device 
 *  @param  pResponse : pointer on the PICC device reply
 *  @retval PICC_SUCCESSCODE : the function is successful
 */
int8_t PICC_Echo(uint8_t *pResponse)
{
	uc8 command[]= {ECHO};
	/* send the command to the PICC and retrieve its response */
	return( drv95HF_SendReceive(command, pResponse));
}

/**
 *	@brief  This function retrieve Data from 95HF device 
 *  @param  pData : pointer on the buffer to fill with data
 *  @retval PICC_SUCCESSCODE : the function is successful
 */
int8_t PICC_PollData(uint8_t *pData)
{
	return (drv95HF_PoolingReading (pData));
}

/**
 *	@brief  this function sends a ProtocolSeclect command to the PICC device
 *  @param  Length  : number of byte of protocol select command parameters
 *  @param  Protocol : RF protocol (ISO 14443 A or B or 15 693 or Fellica)
 *  @param  Parameters: prococol parameters (see reader datasheet)
 *  @param  pResponse : pointer on the PICC device response
 *  @retval PICC_SUCCESSCODE : the command was succedfully send
 *  @retval PICC_ERRORCODE_PARAMETERLENGTH : the Length parameter is erroneous
 *  @retval PICC_ERRORCODE_PARAMETER : a parameter is erroneous
 */
int8_t PICC_ProtocolSelect(uc8 Length,uc8 Protocol,uc8 *Parameters,uint8_t *pResponse)
{
	uint8_t *DataToSend = u95HFBuffer;
	int8_t	status; 
	
	/* check xx95HF is ready to receive Protocol select command */
	PICC_Echo(u95HFBuffer);
	
	if (u95HFBuffer[PICC_STATUS_OFFSET] != ECHORESPONSE)
	{
		/* reset the device */
		PICC_PORsequence( );
	}
	
	/* check the function parameters	*/
	*pResponse = PICC_ERRORCODE_DEFAULT;
	*(pResponse+1) = 0x00;
	/* check the function parameters	*/
	errchk(IsAnAvailableProtocol(Protocol));

	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] 	= PROTOCOL_SELECT;
	DataToSend[PICC_LENGTH_OFFSET  ]	= Length;
	DataToSend[PICC_DATA_OFFSET    ]	= Protocol;
	
	/* DataToSend CodeCmd Length Data	*/
	/* Parameters[0] first byte to emmit	*/
	memcpy(&(DataToSend[PICC_DATA_OFFSET +1]),Parameters,Length -1);
	/* send the command to the PICC and retrieve its response */
  drv95HF_SendReceive(DataToSend, pResponse);
	
	/* check the PICC response */
	if ( PICC_IsResultCodeOk ( PROTOCOL_SELECT , pResponse ) != PICC_SUCCESSCODE)
	{
		return PICC_ERRORCODE_DEFAULT;
	}

	return PICC_SUCCESSCODE;

Error:
	/*initialize the result code to 0xFF and length to in case of error */
	return PICC_ERRORCODE_PARAMETER;	
}


/**
 *	@brief  this function sends a Send command to the PICC device. the command to contacless device is embeded in Parameters.
 *  @param  Length 		: Number of bytes
 *  @param	Parameters 	: data depenps on protocl selected
 *  @retval PICC_SUCCESSCODE : the command was succedfully send
 *  @retval PICC_ERRORCODE_PARAMETERLENGTH : Length parameter is erroneous
 */
int8_t PICC_Send(uc8 Length,uc8 *Parameters)
{
	uint8_t *DataToSend = u95HFBuffer,
					*pResponse = u95HFBuffer;

	/* check the function parameters	*/
	if (CHECKVAL (Length,1,255)==false)
		return PICC_ERRORCODE_PARAMETERLENGTH; 
	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] = SEND;
	DataToSend[PICC_LENGTH_OFFSET  ] = Length;

	/* DataToSend CodeCmd Length Data	*/
	/* Parameters[0] first byte to emmit	*/
	memcpy(&(DataToSend[PICC_DATA_OFFSET]),Parameters,Length);
	
	/* send the command to the PICC and retrieve its response */
	drv95HF_SendReceive(DataToSend, pResponse);
	
	/* check the PICC response */
	if ( PICC_IsResultCodeOk ( SEND , pResponse ) != PICC_SUCCESSCODE)
	{
		return PICC_ERRORCODE_DEFAULT;
	}

	return PICC_SUCCESSCODE;
}


/**
 *	@brief  This functions sends an ACfilter command to the PICC device. This function activates  
 *	@brief  an anti-collision filter for Iso14443-A card protocol. 
 *  @param  Length 	: Number of bytes including the code command and length byte
 *  @param  ATQAParam 	: ATQA response of the PICC device (2 bytes) to REQA command
 *  @param  SAKParam 	: SAK byte response of the PICC device during the anticollision process
 *  @param  pUIDData 	: UID of the PICC device
 *  @param	pResponse 	: the PICC device response  
 *  @retval PICC_SUCCESSCODE : the function
 */
int8_t PICC_AcFilter (uc8 Length,uc8 ATQAParam[2], uc8 SAKParam,uc8 *pUIDData	,uint8_t *pResponse)
{
	uint8_t *DataToSend = u95HFBuffer;
	
	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] = AC_FILTER;
	DataToSend[PICC_LENGTH_OFFSET  ]	= Length;
	if (Length > 0x00)
	{	
		DataToSend[PICC_DATA_OFFSET  ]	= ATQAParam[0];
		if (Length > 0x01)
		{
			DataToSend[PICC_DATA_OFFSET+1]	= ATQAParam[1];
			if (Length > 0x02)
			{
			DataToSend[PICC_DATA_OFFSET+2]	= SAKParam;
			memcpy(&(DataToSend[PICC_DATA_OFFSET+3]),pUIDData,Length-3);
			}
		}
	}
	
	drv95HF_SendReceive(DataToSend, pResponse);	
	
	if (PICC_IsResultCodeOk (AC_FILTER,pResponse) != PICC_SUCCESSCODE)
	{
		return PICC_ERRORCODE_DEFAULT;
	}

	return PICC_SUCCESSCODE;

}


/**
 *	@brief  Send a listen command to the PICC device device
 *  @param  pResponse : pointer on the PICC device response 
 *  @retval PICC_SUCCESSCODE : the function
 */
int8_t PICC_Listen (uint8_t *pResponse)
{
	uint8_t *DataToSend = u95HFBuffer;
	int8_t status;

	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] = LISTEN;
	DataToSend[PICC_LENGTH_OFFSET  ]	= 0x00;
	
	status = drv95HF_SendReceive(DataToSend, pResponse);
	if( status != RFTRANS_95HF_SUCCESS_CODE)
		return PICC_ERRORCODE_DEFAULT;
	
	if (PICC_IsResultCodeOk (LISTEN,pResponse) != PICC_SUCCESSCODE)
	{
		return PICC_ERRORCODE_DEFAULT;
	}
	
	return PICC_SUCCESSCODE;
}


/**
 *	@brief  This function sends a Idle command to the PICC device
 *  @param  Length 	: Number of bytes
 *  @param	Data	: pointer on data. Data depends on protocl selected
 *  @retval PICC_SUCCESSCODE : the command was succedfully send
 *  @retval PICC_ERRORCODE_DEFAULT : the PICC device returned an error code
 *  @retval PICC_ERRORCODE_PARAMETERLENGTH : Length parameter is erroneous
 */
int8_t PICC_Idle( uc8 Length, uc8 *Data )
{
	uint8_t *DataToSend = u95HFBuffer;

	/* check the function parameters	*/
	if (Length != 0x0E)
		return PICC_ERRORCODE_PARAMETERLENGTH; 

	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] = IDLE;
	DataToSend[PICC_LENGTH_OFFSET  ]	= Length;

	
	memcpy(&(DataToSend[PICC_DATA_OFFSET]),Data,Length );

	drv95HF_SendCmd(DataToSend);

	return PICC_SUCCESSCODE;	
}

/**
 *	@brief  this function sends a ReadRegister command to the PICC device
 *  @param  Length 		: Number of bytes
 *  @param	Address		: address of first register to read
 *  @param	RegCount	: number of register to read
 *  @param	Flags		: whether to increment address after register read
 *  @param  pResponse : pointer on PICC response
 *  @retval PICC_SUCCESSCODE : the command was succedfully send
 *  @retval PICC_ERRORCODE_DEFAULT : PICC returned an error code
 *  @retval PICC_ERRORCODE_PARAMETERLENGTH : Length parameter is erroneous
 */
int8_t PICC_ReadRegister (uc8 Length,uc8 Address,uc8 RegCount,uc8 Flags,uint8_t *pResponse)
{
	uint8_t DataToSend[RDREG_BUFFER_SIZE];

	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] = READ_REGISTER;
	DataToSend[PICC_LENGTH_OFFSET  ]	= Length;
	DataToSend[PICC_DATA_OFFSET  ]	= Address;
	DataToSend[PICC_DATA_OFFSET +1 ]	= RegCount;
	DataToSend[PICC_DATA_OFFSET +2 ]	= Flags;

	drv95HF_SendReceive(DataToSend, pResponse);
	
	if (PICC_IsResultCodeOk (IDLE,pResponse) != PICC_SUCCESSCODE)
	{
		return PICC_ERRORCODE_DEFAULT;
	}

	return PICC_SUCCESSCODE;

}

/**
 *	@brief  this function sends a WriteRegister command to PICC
 *  @param  Length 		: Number of bytes of WrReg parameters
 *  @param	Address		: address of first register to write
 *  @param	pData 		: pointer data to be write
 *  @param	Flags		: whether to increment address after register read
 *  @param  pResponse : pointer on PICC response
 *  @retval PICC_SUCCESSCODE : the command was succedfully send
 *  @retval PICC_ERRORCODE_DEFAULT : the function is not successfull
 */
int8_t PICC_WriteRegister (uc8 Length,uc8 Address,uc8 Flags,uc8 *pData,uint8_t *pResponse)
{
	uint8_t *DataToSend = u95HFBuffer;
	
	/* build the command */
	DataToSend[PICC_COMMAND_OFFSET ] = WRITE_REGISTER;
	DataToSend[PICC_LENGTH_OFFSET  ]	= Length;
	DataToSend[PICC_DATA_OFFSET  ]	= Address;
	DataToSend[PICC_DATA_OFFSET +1 ]	= Flags;

	/* Parameters[0] first byte to emmit*/
	memcpy(&(DataToSend[PICC_DATA_OFFSET + 2]), pData , Length - 2);

	if (drv95HF_SendReceive(DataToSend, pResponse) == RFTRANS_95HF_SUCCESS_CODE)
	{
		return PICC_SUCCESSCODE;
	}
	else
	{
		return PICC_ERRORCODE_DEFAULT;
	}
}


/**
 *	@brief  This function sets the load of the backscattering of the PICC
 *  @param  bLoad 		: the load of the backscaterring (between 21 and 2F)
 *  @retval PICC_SUCCESSCODE : the command was succedfully send
 *  @retval PICC_ERRORCODE_DEFAULT : the function is not successfull
 */
int8_t PICC_SetBackscatteringLoad (uc8 bLoad)
{
	int8_t result;
	uint8_t				pData [2]= {AFE_ACCONFIGA_OFFSET,0x00};
		
	/* the load parameter shall be between 0x11 and 0x2F*/
	pData[1] |= (bLoad & 0x3F);

	PICC_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,pData,u95HFBuffer);
	result = PICC_GetBackscatteringLoad(); 
  if (result != bLoad)
		return PICC_ERRORCODE_DEFAULT;
  
	return PICC_SUCCESSCODE;

}

/**
 *	@brief  This function return the load of the Backscaterring of the PICC
 *  @param  None
 *  @retval the load of the backscattering
 */
int8_t PICC_GetBackscatteringLoad ( void )
{
	uint8_t				pData [2]= {0x04,0x20};			

	/* set the read pointer of the PICC*/
	PICC_WriteRegister (0x03,AFE_ANALOG_CONF_REG_SELECTION,0x00,pData,u95HFBuffer);
	/* read the register*/
	PICC_ReadRegister(0x03,AFE_ANALOG_CONF_REG_UPDATE,0x01,0x00,u95HFBuffer);
	
	return u95HFBuffer[PICC_DATA_OFFSET];			
}


/**
 *	@brief  This function initialize the PICC
 *  @param  None 
 *  @retval None
 */
void PICC_Enable_Interrupt( void)
{
	/* inform driver to use interrupt mode */
	drv95HF_EnableInterrupt ( );
	
	delay_ms(1);
}

/**
 *	@brief  This function initialize the PICC
 *  @param  None 
 *  @retval None
 */
void PICC_Disable_Interrupt( void)
{
	/* inform driver to use polling mode */
	drv95HF_DisableInterrupt ( );
}

#if 0
/**
 *	@brief  this function flushs the buffer memoty 
 *  @param	pResponse 	: pseudo reply (shall be 0x55)
 *  @retval PICC_SUCCESS_CODE : the command was succedfully send
 */
void PICC_FlushBuffer (uint8_t *pResponse)
{
	uint8_t 	IRQoutstate=0;
	uint16_t	NthByte = 0; 
	
	IRQoutstate = drv95HF_GetIRQOutState ( );

	while (NthByte++ <	0xFF && IRQoutstate == 0x00)
	{
		*(pResponse + NthByte) = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);													
		IRQoutstate = drv95HF_GetIRQOutState ( );
	} 
}

#endif

/**  
* @brief  this function returns PICC_SUCCESSCODE is the command code exists, PICC_ERRORCODE_COMMANDUNKNOWN otherwise
* @param  CmdCode :  	code command send to PICC		
* @retval PICC_SUCCESSCODE : the command code is available
* @retval PICC_ERRORCODE_COMMANDUNKNOWN : the command code isn't available
*/
int8_t PICC_IsCommandExists(uint8_t CmdCode)
{

  CmdCode = CmdCode & 0x0F;
  switch (CmdCode)
	{
		case ECHO: 
			return PICC_SUCCESSCODE;
		case IDN: 
			return PICC_SUCCESSCODE;
		case PROTOCOL_SELECT: 
			return PICC_SUCCESSCODE;
		case SEND: 
				return PICC_SUCCESSCODE;
		case LISTEN: 
				return PICC_SUCCESSCODE;
		case AC_FILTER: 
				return PICC_SUCCESSCODE;
		case IDLE: 
			return PICC_SUCCESSCODE;
		case READ_REGISTER: 
			return PICC_SUCCESSCODE;
		case WRITE_REGISTER: 
			return PICC_SUCCESSCODE;
		case BAUD_RATE: 
			return PICC_SUCCESSCODE;
		default: 
			return PICC_ERRORCODE_COMMANDUNKNOWN;
	}
}


/**  
* @brief  	this function returns PICC_SUCCESSCODE if the picc reply is a succesful code.
* @param  	CmdCode		:  	code command send to the reader
* @param  	ReaderReply	:  	pointer on PICC response	
* @retval  	PICC_SUCCESSCODE :  PICC returned a succesful code
* @retval  	PICC_ERRORCODE_DEFAULT  :  PICC didn't return a succesful code
* @retval  	PICC_NOREPLY_CODE : no PICC response
*/
int8_t PICC_IsResultCodeOk ( uint8_t CmdCode, uc8 *ReaderReply )
{
	CmdCode = CmdCode & 0xFF;

  if (ReaderReply[READERREPLY_STATUSOFFSET] == PICC_ERRORCODE_DEFAULT)
		return PICC_NOREPLY_CODE;

 	switch (CmdCode)
	{
		case ECHO: 
			if (ReaderReply[PSEUDOREPLY_OFFSET] == ECHO)
				return PICC_SUCCESSCODE;
			else 
				return PICC_ERRORCODE_DEFAULT;
		case IDN: 
			if (ReaderReply[READERREPLY_STATUSOFFSET] == IDN_RESULTSCODE_OK)
				return PICC_SUCCESSCODE;
			else 
				return PICC_ERRORCODE_DEFAULT;
		case PROTOCOL_SELECT: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case IDN_RESULTSCODE_OK :
					return PICC_SUCCESSCODE;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case SEND :
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case SEND_RESULTSCODE_OK:
						return PICC_SUCCESSCODE;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case AC_FILTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case ACFILTER_RESULTSCODE_OK:
						return PICC_SUCCESSCODE;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case LISTEN : 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case LISTEN_RESULTSCODE_OK:
						return PICC_SUCCESSCODE;
				case LISTEN_RESULTSCODE_OK_0X00:
						return PICC_SUCCESSCODE;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case IDLE: 
			
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case IDLE_RESULTSCODE_OK :
					return PICC_SUCCESSCODE;
				case IDLE_ERRORCODE_LENGTH :
					return PICC_ERRORCODE_DEFAULT;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case READ_REGISTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case READREG_RESULTSCODE_OK :
					return PICC_SUCCESSCODE;
				case READREG_ERRORCODE_LENGTH :
					return PICC_ERRORCODE_DEFAULT;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case WRITE_REGISTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case WRITEREG_RESULTSCODE_OK :
					return PICC_SUCCESSCODE;
				default : return PICC_ERRORCODE_DEFAULT;
			}
		case BAUD_RATE: 
			return PICC_ERRORCODE_DEFAULT;
		default: 
			return ERRORCODE_GENERIC;
	}
}

#ifdef USE_POOL_FIELD
/**  
 * @brief  this function pool for field around the PICC?
 * @param  Pool_Field_States : pDataRead[2] = 1 if a field is detetected, pDataRead[2] = 0 in other case
 * @retval PICC_SUCCESSCODE
 */
int8_t PICC_Pool_Field (uint8_t *pDataRead)
{
	uint8_t Pool_Field[] = {0x03, 0x00};

	if(RFTRANS_95HF_CheckSendReceive(Pool_Field, pDataRead)!= RESULTOK)
	{
		return ERRORCODE_GENERIC;
	}

	if(pDataRead[PICC_DATA_OFFSET] == 0x01)
	{
	/*Implement The PICC routine for TAG EMULATION*/
	}

	else
	{
	}

	return PICC_SUCCESSCODE;
}

#endif /*USE POOL_FIELD*/
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
