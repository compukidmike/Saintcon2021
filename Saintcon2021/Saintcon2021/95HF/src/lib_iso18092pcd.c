/**
  ******************************************************************************
  * @file    lib_iso18092pcd.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso18092 communication
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
#include "lib_iso18092pcd.h"

#define uc8 uint8_t
#define uc16 uint16_t
#define u8 uint8_t
#define u16 uint16_t
 
 /* Variables for the different modes */
extern DeviceMode_t devicemode;
extern TagType_t nfc_tagtype;

extern PCD_PROTOCOL TechnoSelected;

FELICA_CARD 	FELICA_Card;

static uc8 REQC[] = {SEND_RECEIVE ,0x05,0x00,0x12,0xFC,0x01,0x03};

static int8_t FELICA_Init( uint8_t *pDataRead );
static int8_t FELICA_REQC( uint8_t *pDataRead );

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup PCD
 * 	@{
 *	@brief  This part of the library enables PCD capabilities of CR95HF & ST95HF.
 */


 /** @addtogroup ISO18092_pcd
 * 	@{
 *	@brief  This part of the library is used to follow ISO18092.
 */


/** @addtogroup lib_iso18092pcd_Private_Functions
 *  @{
 */

/**
 * @brief  Initializes the xx95HF for the FELICA protocol
 * @param  void
 * @return TRUE (if well configured) / FALSE (Communication issue)
 */
static int8_t FELICA_Init( uint8_t *pDataRead )
{
	int8_t  status;
	u8     ProtocolSelectParameters [4]  = {0x51, 0x13, 0x01,0x0D};
	u8     WriteAmpliGain [2]  = {PCD_TYPEF_ARConfigA, PCD_TYPEF_ARConfigB};
	u8     AutoFDet [2]  = {0x02,0xA1};

	
	/* sends a protocol Select command to the pcd to configure it */
	errchk(PCD_ProtocolSelect(0x05,PCD_PROTOCOL_FELICA,ProtocolSelectParameters,pDataRead));
	TechnoSelected = PCDPROTOCOL_18092;
	
	/* in order to adjust the demodulation gain of the PCD*/
  errchk(PCD_WriteRegister    ( 0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,WriteAmpliGain,pDataRead)); 

	/* in order to adjust the auto detect of the PCD*/
	errchk(PCD_WriteRegister    ( 0x04,0x0A,0x01,AutoFDet,pDataRead)); 
	
	/* GT min time to respect before sending REQ_C */
	delay_us(20400);

	return ISO18092_SUCCESSCODE;
Error:
 return ISO18092_ERRORCODE_DEFAULT;
	
}

/**
 * @brief  Handles the REQC command
 * @param  *pDataRead	: Pointer on the response
 * @return TRUE (if CR95HF answered ATQB) / FALSE (No ISO14443B in Field)
 */
static int8_t FELICA_REQC( uint8_t *pDataRead )
{
	if(PCD_CheckSendReceive(REQC, pDataRead)!= ISO18092_SUCCESSCODE)
		return ISO18092_ERRORCODE_DEFAULT;
		
	if (pDataRead[2] != 0x01) // If the answer is not an answer to REQC
		return ISO18092_ERRORCODE_DEFAULT;
	else
		return ISO18092_SUCCESSCODE;
}


/**
  * @}
  */ 


/** @addtogroup lib_iso18092pcd_Public_Functions
 *  @{
 */

void FELICA_Initialization( void )
{
	uint8_t DataRead[MAX_BUFFER_SIZE];
	
	/* Init the FeliCa communication */
	FELICA_Init(DataRead);
}

/**
 * @brief  Checks if a FELICA card is in the field
 * @param  void
 * @return ISO18092_SUCCESSCODE (A card is present) / ISO18092_ERRORCODE_DEFAULT (No card)
 */
int8_t FELICA_IsPresent( void )
{
	uint8_t DataRead[MAX_BUFFER_SIZE];

	/* Initializing buffer */
	memset(DataRead,0,MAX_BUFFER_SIZE);

	/* REQC attempt */
	if(FELICA_REQC(DataRead) != ISO18092_SUCCESSCODE  )
		return ISO18092_ERRORCODE_DEFAULT;

	/* Filling of the data structure */
	FELICA_Card.IsDetected = true;
	memcpy(FELICA_Card.ATQC, &DataRead[PCD_DATA_OFFSET],  ATQC_SIZE);
	memcpy(FELICA_Card.UID , &FELICA_Card.ATQC[1]  , UID_SIZE_FELICA);
	
	devicemode = PCD;
	nfc_tagtype = TT3;

	/* An ISO14443 card is in the field */	
  return ISO18092_SUCCESSCODE;
}

/**
 * @brief Processing of the Anticolision for FELICA cards
 * @param *message : contains message on the CR95HF replies
 * @return ISO18092_SUCCESSCODE : the function is succesful
 */
int8_t FELICA_Anticollision( void )
{
	/* Anticollision process done ! */	
  return ISO18092_SUCCESSCODE;
}

/**
 * @brief  Checks if a FELICA card is still in the field
 * @param  void
 * @return ISO18092_SUCCESSCODE : the function is succesful
 * @return ISO18092_ERRORCODE_DEFAULT : No card in the RF field
 */
int8_t FELICA_CardTest( void )
{
	uint8_t DummyBuffer[MAX_BUFFER_SIZE];

	if(PCD_CheckSendReceive(REQC, DummyBuffer)!= ISO18092_SUCCESSCODE)
		return ISO18092_ERRORCODE_DEFAULT;

	return ISO18092_SUCCESSCODE;
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

