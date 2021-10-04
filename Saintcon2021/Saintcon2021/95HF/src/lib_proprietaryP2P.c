/**
  ******************************************************************************
  * @file    lib_proprietaryP2P.c
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    06/08/2014
  * @brief   Allow to do P2P without LLCP and SNEP (not NFC Forum P2P compliant)
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
#include "lib_proprietaryP2P.h"

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup P2P
 * 	@{
 *	@brief  This part of the library enables P2P capabilities of ST95HF.
 */

/** @addtogroup Proprietary P2P
 * 	@{
 *	@brief  This part of the library is used to implement a proprietary P2P communication
 */
 
 extern PICCEMULATOR_PROTOCOL SelectedTechno;
 
 extern struct PP2P_Context pp2pContext;
 
/* Callback function to interact with application */

// Client and Server 
void  (*P2PReceiveCMDCallBack)(uint8_t*pData, uint16_t nbByte);									 // Broadcast to current client new received CMD frame
void  (*P2PReceiveDATCallBack)(uint8_t*pData, uint16_t nbByte);									 // Broadcast to current client new received DATA frame 

//Server specific
void	(*P2PAcknowledgedStatus)(bool);
 
//Client specific
uint8_t  (*P2PPopTXCallBack)(uint8_t*pData, uint16_t *nbByte,pframeType aFrameType); // Retrieve next TX buffer from current client with associated type
bool  	 (*P2PTxPendingCallBack) ();																								 // Check if something has to be sent .. 
bool 		 (*P2PRxAcknowledged)(bool *); // check if last command has been acknowledged. 


/* Buffer used by this layer */ 
uint8_t PP2P_SendBuff[256];
uint16_t PP2P_SendBuff_Size = 0;    // size of valid data in PP2P_SendBuff	
uint8_t PP2P_ReceiveBuff[256];
uint16_t PP2P_ReceiveBuff_Size = 0; // size of valid data in PP2P_ReceiveBuff

uint8_t PP2P_HoldSendBuff[256];
uint16_t PP2P_HoldSendBuff_Size = 0;    // size of valid data in PP2P_SendBuff	

bool OwnCommunicationToken  = false;
bool PP2PSymActivated = false;

static uint8_t Pcb = 0x02;


PP2P_STATES currentState = STATE_NOREQRES_PENDING;

uint8_t SELECTAPPLI_PP2P_CMD[]  = {0x00,0x04,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
uint8_t SELECTAPPLI_PP2P_CMD_SIZE = 10;
uint8_t REQ_CMD_PP2P[] 					= {0xA2,CMD_INS,0x00,0x00}; // Send CMD (need to be token owner) (need to receive ACK when action done)
uint8_t REQ_CMD_PP2P_SIZE 			= 4;
uint8_t REQ_DAT_PP2P[] 					= {0xA2,DAT_INS,0x00,0x00}; // Send DATA (need to be token owner) (need to receive ACK for each packet received)
uint8_t REQ_DAT_PP2P_SIZE 			= 4;
uint8_t REQ_CTOK_PP2P[] 				= {0xA2,CTOK_INS,0x00,0x00}; // Claim token
uint8_t REQ_CTOK_PP2P_SIZE			= 4;
uint8_t REQ_RTOK_PP2P[] 				= {0xA2,RTOK_INS,0x00,0x00}; // Release token
uint8_t REQ_RTOK_PP2P_SIZE			= 4;
uint8_t REQ_PTOK_PP2P[] 				= {0xA2,PTOK_INS,0x00,0x00}; // Preempt token
uint8_t REQ_PTOK_PP2P_SIZE			= 4;
uint8_t REQ_ACK_PP2P[] 					= {0x90,0x00}; // ACK
uint8_t REQ_ACK_PP2P_SIZE				= 2;
uint8_t REQ_NCK_PP2P[] 					= {0x6F,0x00}; // NACK
uint8_t REQ_NCK_PP2P_SIZE				= 2;
uint8_t REQ_SYM_PP2P[] 					= {0xA2,SYM_INS,0x00,0x00}; // SYM to keep connection
uint8_t REQ_SYM_PP2P_SIZE				= 4;


/* Another simple way for simple system (see pong example)*/
uint8_t REQ_BUF_PP2P[] 					= {0xA2,BUF_INS,0x00,0x00}; // Send Buffer (no need to be token owner) (no ACK, receive another buffer as answer)
uint8_t REQ_BUF_PP2P_SIZE 			= 4;
/* need only one callback for server */
uint8_t  (*P2PReceiveBUFCallBack)(uint8_t*pSendReceive, uint16_t *nbByte);					 // Broadcast to current client new received BUF frame 



 /** @addtogroup lib_proprietaryP2P_Private_Functions
 * 	@{
 */
/* Initiator (Server) */
static void PP2P_SendReceive(uc8 pcb, uc8 *pData,uint8_t pDataSize);


/* Target (Client) */
static void PP2PT_Send(uc8 pcb, uc8 *pData,uint8_t pDataSize);

/**
  * @}
  */ 

/** @addtogroup lib_proprietaryP2P_Public_Functions
 * 	@{
 */

/****************************************** Initiator part *****************************************/

void PP2P_ServerInit(void)
{	
	PP2PSymActivated = false;
	OwnCommunicationToken  = true;
	currentState = STATE_NOREQRES_PENDING;
}


uint8_t 	PP2P_Server(void)
{

	Pcb^=0x01;
		
  /* Do we have a CMD to send */
	if( currentState == STATE_IN_CMDREQ_PENDING)
	{	
		if(!OwnCommunicationToken) /* Inform WOD that we take back the token */ 
		{
			memcpy(PP2P_SendBuff+1,REQ_PTOK_PP2P,REQ_PTOK_PP2P_SIZE);
			PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_PTOK_PP2P_SIZE);
			OwnCommunicationToken = true;
		}
		
		PP2P_SendReceive(Pcb,PP2P_SendBuff,PP2P_SendBuff_Size);
		currentState = STATE_IN_ACK_PENDING; /* wait for acknowledge */
	}
	/* Do we have a DATA to send */
	else if( currentState == STATE_IN_DATREQ_PENDING)
	{	
		if(!OwnCommunicationToken) /* Inform WOD that we take back the token */ 
		{
			memcpy(PP2P_SendBuff+1,REQ_PTOK_PP2P,REQ_PTOK_PP2P_SIZE);
			PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_PTOK_PP2P_SIZE);
			OwnCommunicationToken = true;
		}
		
		PP2P_SendReceive(Pcb,PP2P_SendBuff,PP2P_SendBuff_Size);
		currentState = STATE_IN_ACK_PENDING;
	}
  /* Do we have to acknowledge Token request */	
	else if( currentState == STATE_OUT_ACK_PENDING)
	{	
		memcpy(PP2P_SendBuff+1,REQ_ACK_PP2P,REQ_ACK_PP2P_SIZE);
		PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_ACK_PP2P_SIZE);
		currentState = STATE_NOREQRES_PENDING;
	}
	/* Do we have to send NACK due to previous error */	
	else if( currentState == STATE_OUT_NACK_PENDING)
	{	
		memcpy(PP2P_SendBuff+1,REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
		PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_NCK_PP2P_SIZE);
		currentState = STATE_NOREQRES_PENDING;
	}
	else /* Nothing to do we can send SYM */
	{
		PP2PSymActivated = true;
		memcpy(PP2P_SendBuff+1,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
		PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_SYM_PP2P_SIZE);
	}
	
	/* Analyse of the answer */
	if (PP2P_ReceiveBuff[PICC_STATUS_OFFSET]==LISTEN_RESULTSCODE_OK )	
	{		
		if (PP2P_ReceiveBuff[PICC_DATA_OFFSET+1]==0xA2)	
		{/* Check response from target and set currentState accordingly */ 
			switch(PP2P_ReceiveBuff[PCD_DATA_OFFSET+2]) // to check CMD offset
			{		
				case CMD_INS: 		// Receive a CMD
					if( OwnCommunicationToken == false)
					{
						currentState = STATE_OUT_CMDRES_PENDING; //Application will allow to send the ack
						P2PReceiveCMDCallBack((uint8_t *)(&PP2P_ReceiveBuff[PCD_DATA_OFFSET+3]), PP2P_ReceiveBuff[PCD_LENGTH_OFFSET]-(3+2) );				
					}
					else // NAK
					{
						currentState = STATE_OUT_NACK_PENDING;
					}
					break;
				case DAT_INS:	 	// Receive a DATA
					if( OwnCommunicationToken == false)
					{
						P2PReceiveDATCallBack( (uint8_t *)(&PP2P_ReceiveBuff[PCD_DATA_OFFSET+3]), PP2P_ReceiveBuff[PCD_LENGTH_OFFSET]-(3+2));
						currentState = STATE_OUT_ACK_PENDING;
					}
					else // NAK
					{
						currentState = STATE_OUT_NACK_PENDING;
					}
					break;
				case SYM_INS:		// Receive a Send SYM
					/* Nothing to do */
					break;
				case CTOK_INS:		// Token Request Message
					if( currentState == STATE_NOREQRES_PENDING) // only when Initiator has nothing to do
					{	
						currentState = STATE_OUT_ACK_PENDING;
						OwnCommunicationToken = false;
					}
					break;
				default: 			// Command not handle so transmission error
					PP2PSymActivated = false;
					return PP2P_ERRORCODE_DEFAULT;
//					break;
			}
		}
		/* ACK */
		else if ((PP2P_ReceiveBuff[PICC_DATA_OFFSET+1]==0x90)&&(PP2P_ReceiveBuff[PICC_DATA_OFFSET+2]==0x00))
		{		
			if( currentState == STATE_IN_ACK_PENDING)
			{
				currentState = STATE_NOREQRES_PENDING;
				P2PAcknowledgedStatus(true);
			}
		}
		/* NACK */
		else if ((PP2P_ReceiveBuff[PICC_DATA_OFFSET+1]==0x6F)&&(PP2P_ReceiveBuff[PICC_DATA_OFFSET+2]==0x00))
		{
			if( currentState == STATE_IN_ACK_PENDING)
			{
				currentState = STATE_NOREQRES_PENDING;
				P2PAcknowledgedStatus(false);
			}
		}
		else
		{
			PP2PSymActivated = false;
			return PP2P_ERRORCODE_DEFAULT;
		}
	}
	else /* no answer RF com lost */
	{
		PP2PSymActivated = false;
		return PP2P_ERRORCODE_DEFAULT;
	}
	
	return PP2P_SUCCESS;
}

void P2P_SendACK(bool ack)
{
	if(ack == true)
		currentState = STATE_OUT_ACK_PENDING;
	else
		currentState = STATE_OUT_NACK_PENDING;
}
	

uint8_t PP2P_SendCMD(uint8_t *pData, uint16_t size )
{
	if(!OwnCommunicationToken) /* Inform WOD that we take back the token */ 
	{
		memcpy(PP2P_SendBuff+1,REQ_PTOK_PP2P,REQ_PTOK_PP2P_SIZE);
		PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_PTOK_PP2P_SIZE);
		OwnCommunicationToken = true;
		currentState = STATE_NOREQRES_PENDING;
	}
	
	if(	currentState == STATE_NOREQRES_PENDING)
	{	
		memcpy(PP2P_SendBuff+1,REQ_CMD_PP2P,REQ_CMD_PP2P_SIZE);	
		PP2P_SendBuff_Size = REQ_CMD_PP2P_SIZE;
		memcpy(PP2P_SendBuff+1+REQ_CMD_PP2P_SIZE,pData,size);
		PP2P_SendBuff_Size += size;
	
		currentState = STATE_IN_CMDREQ_PENDING;
		return PP2P_SUCCESS;
	}
	else
		return PP2P_ERRORCODE_DEFAULT;
}

uint8_t PP2P_SendDATA(uint8_t *pData, uint16_t size )
{
	if(!OwnCommunicationToken) /* Inform WOD that we take back the token */ 
	{
		memcpy(PP2P_SendBuff+1,REQ_PTOK_PP2P,REQ_PTOK_PP2P_SIZE);
		PP2P_SendReceive(Pcb,PP2P_SendBuff,REQ_PTOK_PP2P_SIZE);
		OwnCommunicationToken = true;
		currentState = STATE_NOREQRES_PENDING;
	}
	
	if(	currentState == STATE_NOREQRES_PENDING)
	{	
		memcpy(PP2P_SendBuff+1,REQ_DAT_PP2P,REQ_DAT_PP2P_SIZE);	
		PP2P_SendBuff_Size = REQ_DAT_PP2P_SIZE;
		memcpy(PP2P_SendBuff+1+REQ_DAT_PP2P_SIZE,pData,size);
		PP2P_SendBuff_Size += size;
	
		currentState = STATE_IN_DATREQ_PENDING;
		return PP2P_SUCCESS;
	}
	else
		return PP2P_ERRORCODE_DEFAULT;
}

uint8_t PP2P_SendBUF(uint8_t *pData, uint16_t *size )
{	
	uint16_t nbByte;
	
	PP2P_SendBuff[0] = Pcb;
	nbByte = 1;
	
	memcpy(PP2P_SendBuff+1,REQ_BUF_PP2P,REQ_BUF_PP2P_SIZE);	
	nbByte += REQ_BUF_PP2P_SIZE;
	memcpy(PP2P_SendBuff+1+REQ_BUF_PP2P_SIZE,pData,*size);
	nbByte += *size;
	
	// control byte append CRC + 8 bits
	PP2P_SendBuff[nbByte] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 
	nbByte++;

	PP2P_SendBuff_Size = nbByte;
	PCD_SendRecv(nbByte,PP2P_SendBuff,PP2P_ReceiveBuff);
	
	if (PP2P_ReceiveBuff[PCD_STATUS_OFFSET]==LISTEN_RESULTSCODE_OK )	
	{		
		*size = PP2P_ReceiveBuff[PCD_LENGTH_OFFSET]-(3+2);
		memcpy(pData,&PP2P_ReceiveBuff[PCD_DATA_OFFSET+1+REQ_BUF_PP2P_SIZE],*size);
	
		return PP2P_SUCCESS;
	}
	else
		return PP2P_ERRORCODE_DEFAULT;
	
}

/**
 * @brief  This function manage send frame
 * @param  Pcb   		 : Pcb value
 * @param  pData 		 : buffer with frame to send
 * @param  pDataSize : Frame Size
 */
void PP2P_SendReceive(uc8 pcb, uc8 *pData,uint8_t pDataSize)
{
	uint16_t nbByte;
	
	PP2P_SendBuff[0] = pcb;
	nbByte = pDataSize+1;	

	// control byte append CRC + 8 bits
	PP2P_SendBuff[nbByte] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 
	nbByte++;

	PP2P_SendBuff_Size = nbByte;
	PCD_SendRecv(PP2P_SendBuff_Size,PP2P_SendBuff,PP2P_ReceiveBuff);
	
}

/************************************** END OF Initiator part **************************************/


/******************************************* Target part *******************************************/

void PP2P_TargetInit(void)
{	
	PP2PSymActivated = false;
	OwnCommunicationToken  = false;
	currentState =  STATE_NOREQRES_PENDING;
}


/**
 * @brief  This function manage send frame
 * @param  Pcb   		 : Pcb value
 * @param  pData 		 : buffer with frame to send
 * @param  pDataSize : Frame Size
 */
static void PP2PT_Send(uc8 pcb, uc8 *pData,uint8_t pDataSize)
{
	uint16_t nbByte = pDataSize+1;
	PP2P_SendBuff[0] = pcb; 
	memcpy(PP2P_SendBuff+1,pData,pDataSize);
	// control byte append CRC + 8 bits
	if( SelectedTechno == PICCPROTOCOL_14443A )
	{	
		PP2P_SendBuff[nbByte] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 
		nbByte++;
	}
	PP2P_SendBuff_Size = nbByte;
	PICC_Send (PP2P_SendBuff_Size,PP2P_SendBuff);
}



void clientFillCMD(uint8_t * pData)
{
	uint16_t nbByte =0, size;
			
	nbByte = pData[PCD_LENGTH_OFFSET];
			
	if( SelectedTechno == PICCPROTOCOL_14443A )
		nbByte -= (	4+1+2); /* 4 protocol bytes + 1 One byte to indicate CRC status + 2Bytes if 106kbits */
	else
		nbByte -= (4+1); /* 4 protocol bytes + 1 One byte to indicate CRC status  */
			
	/* prepare response to caller */
	memcpy(PP2P_ReceiveBuff,&pData[PCD_DATA_OFFSET+4], nbByte);
	size = nbByte;
  
	P2PReceiveCMDCallBack(PP2P_ReceiveBuff, size);
}

void clientFillDAT(uint8_t * pData)
{
	uint16_t nbByte = 0, size;
			
	nbByte = pData[PCD_LENGTH_OFFSET];
			
	if( SelectedTechno == PICCPROTOCOL_14443A )
		nbByte -= (	4+1+2); /* 4 protocol bytes + 1 One byte to indicate CRC status + 2Bytes if 106kbits */
	else
		nbByte -= (4+1); /* 4 protocol bytes + 1 One byte to indicate CRC status  */
			
	/* prepare response to caller */
	memcpy(PP2P_ReceiveBuff,&pData[PCD_DATA_OFFSET+4], nbByte);
	size = nbByte;
	
	P2PReceiveDATCallBack(PP2P_ReceiveBuff, size);
}


/**
	return 0 : 

*/
uint16_t getNextFrame(uint8_t pcb,uint8_t * SendBuff)
{
	frameType aFrameType = UNDEF_TYPE;
	
	uint16_t nbByte=0;
	uint16_t bufferSize;
	
	P2PPopTXCallBack(PP2P_SendBuff+5, &bufferSize, &aFrameType);
	if ((bufferSize == 0) && (aFrameType == UNDEF_TYPE))
	{
		currentState =  STATE_NOREQRES_PENDING;
		return 0;
	}

	PP2P_SendBuff[nbByte++]  = pcb;
	if (aFrameType == STD_CMD_TYPE)
	{
		memcpy(PP2P_SendBuff+1,REQ_CMD_PP2P,REQ_CMD_PP2P_SIZE);
		nbByte += REQ_CMD_PP2P_SIZE+bufferSize;
		currentState = STATE_IN_CMDREQ_PENDING;
	}
	else
	{
		memcpy(PP2P_SendBuff+1,REQ_DAT_PP2P,REQ_DAT_PP2P_SIZE);
		nbByte += REQ_DAT_PP2P_SIZE+bufferSize;
		currentState = STATE_IN_DATREQ_PENDING;
	}	
	// control byte append CRC + 8 bits
	if( SelectedTechno == PICCPROTOCOL_14443A )
	{	
		PP2P_SendBuff[nbByte] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;	 
		nbByte++;
	}
	
	return nbByte;
}

// BEGIN P2P Development
void PP2P_GetCMD(uint8_t *pData )
{
	// Last Buffer received is a CMD	
	if (!OwnCommunicationToken)
	{
		
		// Current state must be STATE_NOREQRES_PENDING
		if(currentState != STATE_NOREQRES_PENDING) 
		{
			PP2PT_Send(pData[2],REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
			//keep the previous currentState
		}
		else
		{
			clientFillCMD(pData);
			PP2PT_Send(pData[2],REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
			currentState = STATE_OUT_CMDRES_PENDING;
		}	
	}
	else // Return Error as CMD can't currently be received 
	{
		PP2PT_Send(pData[2],REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
		// keep the previous current state
	}
}

void PP2P_GetData(uint8_t *pData)
{
	// Last received Buffer is a Data
	if (!OwnCommunicationToken)
	{
		// Current state must be STATE_NOREQRES_PENDING
		if(currentState != STATE_NOREQRES_PENDING) 
		{
			PP2PT_Send(pData[2],REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
			//keep the previous currentState
		}
		else
		{
			// If Communication is handled by WOD.. treat it
			// Send Command to upper layer
			clientFillDAT(pData);
			// Send ACK to WOD
			PP2PT_Send(pData[2],REQ_ACK_PP2P,REQ_ACK_PP2P_SIZE);
		}
	}
	else // Return Error as CMD can't currently be received 
	{
		PP2PT_Send(pData[2],REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
	}
}

void PP2P_GetBuf(uint8_t *pSendReceive)
{
	uint16_t nbByte = 0;
	uint8_t pcb;
			
	nbByte = pSendReceive[PCD_LENGTH_OFFSET];
	pcb = pSendReceive[PCD_DATA_OFFSET];
			
	if( SelectedTechno == PICCPROTOCOL_14443A )
		nbByte -= (	4+1+2); /* 4 protocol bytes + 1 One byte to indicate CRC status + 2Bytes if 106kbits */
	else
		nbByte -= (4+1); /* 4 protocol bytes + 1 One byte to indicate CRC status  */
	
	/*retrieve received data */
	memcpy(PP2P_ReceiveBuff,&pSendReceive[PCD_DATA_OFFSET+5], nbByte);
			
	/* Transmit receive buffer and reply receive answer */	
	P2PReceiveBUFCallBack(PP2P_ReceiveBuff, &nbByte);
	
	/* prepare response to caller */
	PP2P_SendBuff[0] = pcb;
	memcpy(&PP2P_SendBuff[1],REQ_BUF_PP2P, REQ_BUF_PP2P_SIZE);
	memcpy(&PP2P_SendBuff[1+REQ_BUF_PP2P_SIZE],PP2P_ReceiveBuff, nbByte);
	
	nbByte += (1+REQ_BUF_PP2P_SIZE);
	if( SelectedTechno == PICCPROTOCOL_14443A )
	{		 
		PP2P_SendBuff[nbByte] = SEND_MASK_APPENDCRC | SEND_MASK_8BITSINFIRSTBYTE;
		nbByte++;
	}

	// Send Ans
	PP2P_SendBuff_Size = nbByte,
	PICC_Send (PP2P_SendBuff_Size,PP2P_SendBuff);
		
}

void PP2P_GetPreemptionToken(uint8_t Pcb)
{
	if (!OwnCommunicationToken)
	{
		OwnCommunicationToken = true;
		PP2PT_Send(Pcb,REQ_ACK_PP2P,REQ_ACK_PP2P_SIZE);
	}
	else // Current device already helds the Token.. Invalid Command from WOD
	{
		PP2PT_Send(Pcb,REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
	}
}

void PP2P_GetSYM(uint8_t Pcb)
{
	
	uint8_t bufferSize=0;
	bool AckStatus;
	
	/* This inform that PP2P COM started */
	PP2PSymActivated = true;
	
	// Last received buffer is a SYM
	// If WOD owns the communication token
	if (!OwnCommunicationToken)
	{
		if (currentState == STATE_NOREQRES_PENDING)
		{
			if (P2PTxPendingCallBack()) // Do we need to request Communication Token
			{
				currentState = STATE_IN_PREREQ_PENDING;
				PP2PT_Send(Pcb,REQ_CTOK_PP2P,REQ_CTOK_PP2P_SIZE);
			}
			else
			{
				PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
			}
			
		}
		else if (currentState == STATE_OUT_CMDRES_PENDING) 
		{
			if (P2PRxAcknowledged(&AckStatus)) // Does last cmd has been acknowledged
			{
				currentState = STATE_NOREQRES_PENDING;
				if ( AckStatus)
				{
					PP2PT_Send(Pcb,REQ_ACK_PP2P,REQ_ACK_PP2P_SIZE);
				}
				else
				{
					PP2PT_Send(Pcb,REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
				}
			}
		}
		else	
			{
				// keep pending ACK State and Send a SYM
				PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
			}
	}
	else // Device helds communication token
	{
		if ((currentState == STATE_IN_CMDREQ_PENDING) || (currentState == STATE_IN_DATREQ_PENDING))// wait for a ACK Response from a WOD device
		{
			PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
		}
		else 
		{
			bufferSize=getNextFrame(Pcb,PP2P_SendBuff);
			if(bufferSize!=0)
				//PP2PT_Send(Pcb,PP2P_SendBuff,bufferSize);
			 PICC_Send (bufferSize,PP2P_SendBuff);
			else
				PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
		}
	}
}

void PP2P_ReleaseCommunicationToken(uint8_t Pcb)
{
	if (!OwnCommunicationToken)
	{
		// mustn't  be received in this state - but in all the case we answer ACK
		PP2PT_Send(Pcb,REQ_ACK_PP2P,REQ_ACK_PP2P_SIZE);
	}
	else
	{
		OwnCommunicationToken = false;

		// need to store previous CMD to be resent once the token is retrieved
		if (currentState != STATE_NOREQRES_PENDING )
		{
			memcpy(PP2P_HoldSendBuff,PP2P_SendBuff,PP2P_SendBuff_Size);
		  PP2P_HoldSendBuff_Size = PP2P_SendBuff_Size;
			currentState = STATE_NOREQRES_PENDING	;		
		}
		PP2PT_Send(Pcb,REQ_ACK_PP2P,REQ_ACK_PP2P_SIZE);
	
	}
}

void PP2P_GetACK(uint8_t Pcb)
{
	// if WOD get the communication token
	// 	-> if TokenRequest pending -> set the Communication Flag and send the data/cmd 
	//  else if current device hold the communication token
	//  if cmd/data to send proceed 
	//  else send a SYM

	uint8_t bufferSize=0;
	
	if (!OwnCommunicationToken)
	{
		if (currentState == STATE_IN_PREREQ_PENDING)
		{
			OwnCommunicationToken = true;
			currentState = STATE_NOREQRES_PENDING;
			PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
		}
		else
		{
			PP2PT_Send(Pcb,REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
		}
	}
	else // Current Device handle the communication
	{
		if (( currentState == STATE_IN_CMDREQ_PENDING) || ( currentState == STATE_IN_DATREQ_PENDING))
		{
			bufferSize=getNextFrame(Pcb,PP2P_SendBuff);
			if(bufferSize!=0)
				//PP2PT_Send(Pcb,PP2P_SendBuff,bufferSize);
			 PICC_Send (bufferSize,PP2P_SendBuff);
			else
				PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
		}
		else
		{
			PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
		}
	}
}


void PP2P_GetNACK(uint8_t Pcb)
{
	// if WOD get the communication

	if (currentState == STATE_IN_PREREQ_PENDING)
	{
		  memcpy(PP2P_SendBuff, REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
			PP2P_SendBuff_Size = REQ_SYM_PP2P_SIZE;
			PP2PT_Send(Pcb,REQ_SYM_PP2P,REQ_SYM_PP2P_SIZE);
			currentState = STATE_NOREQRES_PENDING;
	}
	else // other use case : 
	{
		// resent the previous command and keep the hold state
		 PICC_Send (PP2P_SendBuff_Size,PP2P_SendBuff);
	}
	
}

void PP2P_UnsupportedREQ(uint8_t Pcb)
{
	// we receive an unsupported REQ. Send NACK
	PP2PT_Send(Pcb,REQ_NCK_PP2P,REQ_NCK_PP2P_SIZE);
}

/*************************************** END OF Target part ****************************************/

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
