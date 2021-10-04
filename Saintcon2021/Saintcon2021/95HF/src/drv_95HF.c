/**
  ******************************************************************************
  * @file    drv_95HF.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides set of driver functions to manage communication 
  * @brief   between MCU and xx95HF chip (RX95HF, CR95HF or ST95HF) 
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
#include "drv_95HF.h"
#include "drv_interrupt.h"

#define uc8 uint8_t
#define uc16 uint16_t
#define u8 uint8_t
#define u16 uint16_t

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */
 
/** @addtogroup _95HF_Driver
 * 	@{
 *  @brief  <b>This folder contains the driver layer of 95HF family (CR95HF, RX95HF, ST95HF)</b>
 */

/** @addtogroup drv_95HF
 * 	@{
 *  @brief  This file includes the driver of the xx95HF family (CR95HF, RX95HF, ST95HF)
 */

/**
* @brief  buffer to exchange data with the RF tranceiver.
*/
uint8_t				u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

/** 
 *  @brief This uTimeOut variable is used as a timeout duting the communication with the RF tranceiver 
 */
extern volatile bool							uDataReady; 
__IO uint8_t						uTimeOut;
bool EnableTimeOut = true;


/* ConfigStructure */ 										 
drv95HF_ConfigStruct			drv95HFConfig;

/* drv95HF_Private_Functions */
static void drv95HF_RCCConfigSPI						( void );
static void drv95HF_GPIOConfigSPI						( void );
static void drv95HF_StructureConfigSPI			( void );
static void drv95HF_InitializeSPI						( void );
static void drv95HF_SendSPIResetByte				( void );

static int8_t drv95HF_SPIPollingCommand			( void );

#ifdef CR95HF
static void drv95HF_StructureConfigUART ( uc32 BaudRate );
static void drv95HF_RCCConfigUART						( void );
static void drv95HF_GPIOConfigUART					( void );
static void drv95HF_SendUARTCommand		(uc8 *pData);
#endif /* CR95HF */


/** @addtogroup drv_95HF_Private_Functions
 *  @{
 */

/**
 *	@brief  Initializes clock
 *  @param  None
 *  @retval void 
 */
static void drv95HF_RCCConfigSPI ( void )
{	
	/* Enable GPIO clock  */
  //RCC_APB2PeriphClockCmd( 	RFTRANS_95HF_SPI_SCK_GPIO_CLK  | 
														//RFTRANS_95HF_SPI_MISO_GPIO_CLK | 
														//RFTRANS_95HF_SPI_MOSI_GPIO_CLK , 
														//ENABLE);
	
  /* Enable SPI clock  */
 // RCC_APB2PeriphClockCmd(RFTRANS_95HF_SPI_CLK, ENABLE);

#ifdef USE_DMA
	/* Enable DMA1 or/and DMA2 clock */
	RCC_AHBPeriphClockCmd(RFTRANS_95HF_SPI_DMA_CLK, ENABLE);
#endif
	
}

/**
 *	@brief  Initializes GPIO for SPI communication
 *  @param  None
 *  @retval None 
 */
static void drv95HF_GPIOConfigSPI(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Configure SPI pins: SCK, MISO and MOSI */
	//GPIO_InitStructure.GPIO_Pin 				= RFTRANS_95HF_SPI_SCK_PIN;
	//GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Mode 				= GPIO_Mode_AF_PP;
	//GPIO_Init(RFTRANS_95HF_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
	
	//GPIO_InitStructure.GPIO_Pin 				= RFTRANS_95HF_SPI_MOSI_PIN;
	//GPIO_Init(RFTRANS_95HF_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);


	/*	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF_PP; */
	//GPIO_InitStructure.GPIO_Mode 				= GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Pin 				= RFTRANS_95HF_SPI_MISO_PIN;
	//GPIO_Init(RFTRANS_95HF_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
	
	/* Configure I/O for Chip select */		
	//GPIO_InitStructure.GPIO_Pin   			= RFTRANS_95HF_SPI_NSS_PIN;
	//GPIO_InitStructure.GPIO_Mode  			= GPIO_Mode_Out_PP; 
	//GPIO_Init(RFTRANS_95HF_SPI_NSS_GPIO_PORT, &GPIO_InitStructure);
	
	/* SPI_NSS  = High Level  */
	//GPIO_SetBits(RFTRANS_95HF_SPI_NSS_GPIO_PORT, RFTRANS_95HF_SPI_NSS_PIN);  
}

/**
 *	@brief  SET SPI protocol
 *  @param  None
 *  @retval None 
 */ 
static void drv95HF_StructureConfigSPI ( void )
{
	//SPI_InitTypeDef  SPI_InitStructure;

	/* Initialize the SPI with default values */
	//SPI_StructInit(&SPI_InitStructure);

	/* SPI Config master with NSS manages by software using the SSI bit*/
	//SPI_InitStructure.SPI_Mode 				= SPI_Mode_Master;
	//SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; 
	//SPI_InitStructure.SPI_NSS  				= SPI_NSS_Soft;
	//SPI_InitStructure.SPI_CPOL 				= SPI_CPOL_High;
	//SPI_InitStructure.SPI_CPHA 				= SPI_CPHA_2Edge;

	/* Init the SPI BRIDGE */
	//SPI_Init(RFTRANS_95HF_SPI, &SPI_InitStructure);
#ifdef USE_DMA
	SPI_I2S_DMACmd(RFTRANS_95HF_SPI, SPI_I2S_DMAReq_Rx, ENABLE);
	SPI_I2S_DMACmd(RFTRANS_95HF_SPI, SPI_I2S_DMAReq_Tx, ENABLE);
#endif
 	/* Enable SPI */
	//SPI_Cmd(RFTRANS_95HF_SPI, ENABLE);
}

/**
 *	@brief  this functions initializes the SPI in order to communicate with the 95HF device
 *  @param  None
 *  @retval void 
 */
static void drv95HF_InitializeSPI ( void )
{
	//SPI_I2S_DeInit(RFTRANS_95HF_SPI);
	/* enables SPI and GPIO clocks */
	//drv95HF_RCCConfigSPI( );
  /* configures GPIO A PA4-7 as SPI bus  (NSS = HL) */ 
	//drv95HF_GPIOConfigSPI( );	
	//drv95HF_StructureConfigSPI( );
}

/**
 *	@brief  This function sends a reset command over SPI bus
 *  @param  none
 *  @retval None
 */
static void drv95HF_SendSPIResetByte(void)
{
	/* Send reset control byte */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_RESET);
}


#ifdef CR95HF
/**
 *	@brief  Initializes clock	for UART
 *  @param  None
 *  @retval None 
 */
static void drv95HF_RCCConfigUART(void)
{	
	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd( 	RFTRANS_95HF_UART_TX_GPIO_CLK | 
													RFTRANS_95HF_UART_RX_GPIO_CLK, /*| 
													RCC_APB2Periph_AFIO, 	  */
														ENABLE);
	/* enable USART 2 clock */
	RCC_APB1PeriphClockCmd(		RFTRANS_95HF_UART_CLK , ENABLE);
}

/**
 *	@brief  Initializes GPIO for UART configuration
 *  @param  None
 *  @retval None 
 */
static void drv95HF_GPIOConfigUART(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART Tx as ouput alternate function push pull*/
	GPIO_InitStructure.GPIO_Pin 	= RFTRANS_95HF_UART_RX_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_Init(RFTRANS_95HF_UART_TX_GPIO_PORT, &GPIO_InitStructure);
	
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = RFTRANS_95HF_UART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(RFTRANS_95HF_UART_RX_GPIO_PORT, &GPIO_InitStructure); 
}

/**
 *	@brief  Set UART protocol 
 *  @param  BaudRate : value of the Baudrate to use during UART communication
 *  @retval None 
 */ 
static void drv95HF_StructureConfigUART ( uc32 BaudRate )
{
	USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate 			= BaudRate;
  USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
  USART_InitStructure.USART_StopBits 			= USART_StopBits_2;
  USART_InitStructure.USART_Parity 				= USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USART */
  USART_Init(RFTRANS_95HF_UART, &USART_InitStructure);
  
  /* Enable the USART */
  USART_Cmd(RFTRANS_95HF_UART, ENABLE);
}

/**												   
 *	@brief  This function send a command to 95HF device over UART bus
 *  @param  *pData : pointer on data to send to the 95HF device
 *  @retval None
 */
static void drv95HF_SendUARTCommand(uc8 *pData)
{
  uint16_t bufferlength = 0;
	if(pData[0] == ECHO)
  {
		/* send Echo */
		UART_SendByte(RFTRANS_95HF_UART, ECHO);
	}
	else
  {
    if (pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x24)
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET + 256;
    }
    else if (pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x44)
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET + 512;
    }
    else
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET]+RFTRANS_95HF_DATA_OFFSET;
    }

		/* send the buffer over UART */
		UART_SendBuffer(RFTRANS_95HF_UART, pData, bufferlength);
  }
}

/**
 *	@brief  This functions recovers a response from RFTRANS_95HF device over UART bus
 *  @param  *pData : pointer on data received from RFTRANS_95HF device
 *  @retval None
 */
static void drv95HF_ReceiveUARTResponse(uint8_t *pData)
{
	/* Recover the "Command" byte */
	pData[RFTRANS_95HF_COMMAND_OFFSET] = UART_ReceiveByte(RFTRANS_95HF_UART);

	if(pData[RFTRANS_95HF_COMMAND_OFFSET] == ECHO)
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
	
	else
	{
		/* Recover the "Length" byte */
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = UART_ReceiveByte(RFTRANS_95HF_UART);
	
		/* Recover data */
		if(pData[RFTRANS_95HF_LENGTH_OFFSET]	!= 0)
			UART_ReceiveBuffer(RFTRANS_95HF_UART, &pData[RFTRANS_95HF_DATA_OFFSET], pData[RFTRANS_95HF_LENGTH_OFFSET]);
	}
}

#endif /* CR95HF */

/**
 * @}
 */

/** @addtogroup drv_95HF_Public_Functions
 * 	@{
 */

/**
* @brief  	Initilize the 95HF device config structure
* @param  	None
* @retval 	None
*/
void drv95HF_InitConfigStructure (void)
{
	drv95HFConfig.uInterface = RFTRANS_95HF_INTERFACE_SPI;
	drv95HFConfig.uSpiMode = RFTRANS_95HF_SPI_POLLING;
	drv95HFConfig.uState = RFTRANS_95HF_STATE_POWERUP;
	drv95HFConfig.uCurrentProtocol = RFTRANS_95HF_PROTOCOL_UNKNOWN;
	drv95HFConfig.uMode = RFTRANS_95HF_MODE_UNKNOWN;
}


/**
 *	@brief  Send a reset sequence over SPI bus (Reset command ,wait ,negative pulse on IRQin).
 *  @param  None
 *  @retval None
 */
void drv95HF_ResetSPI ( void )
{	
	/* Deselect Rftransceiver over SPI */
	RFTRANS_95HF_NSS_HIGH();
	delayHighPriority_ms(1);
	/* Select 95HF device over SPI */
	RFTRANS_95HF_NSS_LOW();
	/* Send reset control byte	*/
	drv95HF_SendSPIResetByte();
	/* Deselect 95HF device over SPI */
	RFTRANS_95HF_NSS_HIGH();
	delayHighPriority_ms(3);

	/* send a pulse on IRQ_in to wake-up 95HF device */
	drv95HF_SendIRQINPulse();
	delayHighPriority_ms(10);  /* mandatory before issuing a new command */

	drv95HFConfig.uState = RFTRANS_95HF_STATE_READY;
	
}


/**
 *	@brief  returns the value of interface pin. 
 *				  Low level means UART bus was choose.
 *				  High level means SPI bus was choose.
 *  @param  None
 *  @retval Bit_RESET : 0
 *  @retval Bit_SET : 1
 */
int8_t drv95HF_GetInterfacePinState ( void )
{
	//return RFTRANS_95HF_GET_INTERFACE();
	return 1;
}

/**  
 *  @brief  This function returns the IRQout state
 *  @param  None
 *  @retval Pin set : 1
 *  @retval Pin reset : 0
 */
int8_t drv95HF_GetIRQOutState ( void )
{
	return platformGpioIsHigh(ST25R95_N_IRQ_IN_PORT,ST25R95_N_IRQ_IN_PIN);
	/*if ((GPIO_ReadInputData(EXTI_GPIO_PORT) & 0x0008) != 0x00)
	{
		return 0x01;
	}
	else 
	{
		return 0x00;
	}*/
}

/**
 *	@brief  This function initialize MCU serial interface peripheral (SPI or UART)
 *  @param  None
 *  @retval None
 */
void drv95HF_InitilizeSerialInterface ( void )
{
	/* -- Get interface pin state to select UART or SPI mode -- */
	if (drv95HF_GetInterfacePinState () != RFTRANS_95HF_INTERFACE_UART)
	{
		drv95HFConfig.uInterface = RFTRANS_95HF_INTERFACE_SPI;
		/* -- Initialize SPI Interface -- */ 
		drv95HF_InitializeSPI( );
		/* -- IRQout configuration PA2 to send pulse on USART_RX of 95HF device */
		IRQOut_Config( );
	}
#ifdef CR95HF
	else
	{
		drv95HFConfig.uInterface = RFTRANS_95HF_INTERFACE_UART;
		/* -- Initialize UART Interface -- */ 
		drv95HF_InitializeUART(BAUDRATE_DATARATE_DEFAULT );
	}
#endif /* CR95HF */
	
}

/**
 *	@brief  This function enable the interruption
 *  @param  None
 *  @retval None
 */
void drv95HF_EnableInterrupt(void)
{
	/* enable interruption */
	drvInt_Enable_Reply_IRQ();
	
	/* set back driver in polling mode */
	drv95HFConfig.uSpiMode = RFTRANS_95HF_SPI_INTERRUPT;	
	
}

/**
 *	@brief  This function disable the interruption
 *  @param  None
 *  @retval None
 */
void drv95HF_DisableInterrupt(void)
{
	/* disable interruption */
	drvInt_Disable_95HF_IRQ();
	
	/* set back driver in polling mode */
	drv95HFConfig.uSpiMode = RFTRANS_95HF_SPI_POLLING;	
}


/**
 *	@brief  This function returns the Interface selected(UART or SPI)
 *  @param  none
 *  @retval RFTRANS_INTERFACE_UART : the UART interface is selected
 *  @retval RFTRANS_INTERFACE_SPI : the SPI interface is selected
 */
uint8_t drv95HF_GetSerialInterface ( void )
{
	return drv95HFConfig.uInterface;
}

/**
 *	@brief  This function sends a command over SPI bus
 *  @param  *pData : pointer on data to send to the xx95HF
 *  @retval void
 */
void drv95HF_SendSPICommand(uc8 *pData)
{
	uint8_t DummyBuffer[MAX_BUFFER_SIZE];
  uint16_t bufferlength = 0;
	  	
	/*  Select xx95HF over SPI  */
	RFTRANS_95HF_NSS_LOW();

	/* Send a sending request to xx95HF  */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_SEND);

	if(*pData == ECHO)
	{
		/* Send a sending request to xx95HF */ 
		SPI_SendReceiveByte(RFTRANS_95HF_SPI, ECHO);
	}
	else
	{
    if( pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x24 )
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET + 256;
    }
    else if (pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x44)
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET + 512;
    }
    else
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET;
    }

		/* Transmit the buffer over SPI */
#ifdef USE_DMA	
		SPI_SendReceiveBufferDMA(RFTRANS_95HF_SPI, pData, bufferlength, DummyBuffer);
#else
		SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, pData, bufferlength, DummyBuffer);
#endif
	}
	
	/* Deselect xx95HF over SPI  */
	RFTRANS_95HF_NSS_HIGH();
}

/**
 *	@brief  This function polls 95HF chip until a response is ready or
 *				  the counter of the timeout overflows
 *  @retval PCD_POLLING_TIMEOUT : The time out was reached 
 *  @retval PCD_SUCCESS_CODE : A response is available
 */
static int8_t drv95HF_SPIPollingCommand( void )
{
	uint8_t Polling_Status = 0;
	
	uint32_t timeout = 0;

	if(EnableTimeOut) timeout=platformTimerCreate(3000);
		//StartTimeOut(3000);		/* 3sec for LLCP can be improved to adjust it dynamically */

	if (drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_POLLING)
	{
		
		do{
			/* in case of an HID interuption during the process that can desactivate the timeout */
			/* Enable the Time out timer */
			//TIM_Cmd(TIMER_TIMEOUT, ENABLE);
			
			
			RFTRANS_95HF_NSS_LOW();
			
			//delay_ms(2);
					
			/*  poll the 95HF transceiver until he's ready ! */
			Polling_Status  = SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_POLLING);
			
			Polling_Status &= RFTRANS_95HF_FLAG_DATA_READY_MASK;
	
		}	while( Polling_Status 	!= RFTRANS_95HF_FLAG_DATA_READY && platformTimerIsExpired(timeout) != true );
		
		RFTRANS_95HF_NSS_HIGH();
	}	
	else if (drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_INTERRUPT)
	{
		/* Wait a low level on the IRQ pin or the timeout  */
		while( (uDataReady == false) & (uTimeOut == false) )
		{	}		
	}

	if(EnableTimeOut)
		StopTimeOut( );

	if ( platformTimerIsExpired(timeout) == true )
		return RFTRANS_95HF_POLLING_TIMEOUT;

	return RFTRANS_95HF_SUCCESS_CODE;	
}

/**
 *	@brief  This fucntion recovers a response from 95HF device
 *  @param  *pData : pointer on data received from 95HF device
 *  @retval None
 */
void drv95HF_ReceiveSPIResponse(uint8_t *pData)
{
	uint8_t DummyBuffer[MAX_BUFFER_SIZE];
  uint16_t lengthToRead = 0;

	/* Select 95HF transceiver over SPI */
	RFTRANS_95HF_NSS_LOW();

	/* Request a response from 95HF transceiver */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_RECEIVE);

	/* Recover the "Command" byte */
	pData[RFTRANS_95HF_COMMAND_OFFSET] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);

	if(pData[RFTRANS_95HF_COMMAND_OFFSET] == ECHO)
	{
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
		/* In case we were in listen mode error code cancelled by user (0x85 0x00) must be retrieved */
		pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	}
	else if(pData[RFTRANS_95HF_COMMAND_OFFSET] == 0xFF)
	{
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
		pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	}
	else
	{
		/* Recover the "Length" byte */
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		/* Checks the data length */
		if( !( ( ( pData[RFTRANS_95HF_COMMAND_OFFSET] & 0xE0) == 0x80 ) && (pData[RFTRANS_95HF_LENGTH_OFFSET] == 0x00) ) )
    {
      lengthToRead = (uint16_t)(pData[RFTRANS_95HF_COMMAND_OFFSET] & 0x60);
      lengthToRead = (lengthToRead << 3) + pData[RFTRANS_95HF_LENGTH_OFFSET];
			/* Recover data 	*/
#ifdef USE_DMA
      SPI_SendReceiveBufferDMA(RFTRANS_95HF_SPI, DummyBuffer, lengthToRead, &pData[RFTRANS_95HF_DATA_OFFSET]);    
#else
		if(lengthToRead > 0){
      SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, DummyBuffer, lengthToRead, &pData[RFTRANS_95HF_DATA_OFFSET]);
		}
#endif	
		}
	}

	/* Deselect xx95HF over SPI */
	RFTRANS_95HF_NSS_HIGH();
	
}


/**
 *	@brief  This function send a command to 95HF device over SPI or UART bus and receive its response
 *  @param  *pCommand  : pointer on the buffer to send to the 95HF device ( Command | Length | Data)
 *  @param  *pResponse : pointer on the 95HF device response ( Command | Length | Data)
 *  @retval RFTRANS_95HF_SUCCESS_CODE : the function is succesful
 */
int8_t  drv95HF_SendReceive(uc8 *pCommand, uint8_t *pResponse)
{		
	u8 command = *pCommand;
	
	/* if we want to send a command we are not expected a interrupt from RF event */
	if(drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_INTERRUPT)
	{	
		drvInt_Enable_Reply_IRQ();
	}
	
	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
	{
		/* First step  - Sending command 	*/
		drv95HF_SendSPICommand(pCommand);
		/* Second step - Polling	*/
		if (drv95HF_SPIPollingCommand( ) != RFTRANS_95HF_SUCCESS_CODE)
		{	*pResponse =RFTRANS_95HF_ERRORCODE_TIMEOUT;
			return RFTRANS_95HF_POLLING_TIMEOUT;	
		}
		/* Third step  - Receiving bytes */
		drv95HF_ReceiveSPIResponse(pResponse);
	}
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
	{
		/* First step  - Sending command	*/
		drv95HF_SendUARTCommand(pCommand);
		/* Second step - Receiving bytes */
		drv95HF_ReceiveUARTResponse(pResponse);
	}
#endif /* CR95HF */
	
	/* After listen command is sent an interrupt will raise when data from RF will be received */
	if(command == LISTEN)
	{	
		if(drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_INTERRUPT)
		{		
			drvInt_Enable_RFEvent_IRQ( );
		}
	}

	return RFTRANS_95HF_SUCCESS_CODE; 
}

/**
 *	@brief  This function send a command to 95HF device over SPI or UART bus
 *  @param  *pCommand  : pointer on the buffer to send to the 95HF ( Command | Length | Data)
 *  @retval None
 */
void drv95HF_SendCmd(uc8 *pCommand)
{
	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
		/* First step  - Sending command 	*/
		drv95HF_SendSPICommand(pCommand);
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
		/* First step  - Sending command	*/
		drv95HF_SendUARTCommand(pCommand);
#endif /* CR95HF */
}

/**
 *	@brief  This function is a specific command. It's made polling and reading sequence. 
 *  @param  *pResponse : pointer on the 95HF device response ( Command | Length | Data)
 *  @retval RFTRANS_95HF_SUCCESS_CODE : the function is succesful
 *  @retval RFTRANS_95HF_POLLING_RFTRANS_95HF : the polling sequence returns an error
 */
int8_t  drv95HF_PoolingReading (uint8_t *pResponse)
{
	
	*pResponse =RFTRANS_95HF_ERRORCODE_DEFAULT;
	*(pResponse+1) = 0x00;

	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
	{
		/* First step - Polling	*/
		if (drv95HF_SPIPollingCommand( ) != RFTRANS_95HF_SUCCESS_CODE)
		{	*pResponse = RFTRANS_95HF_ERRORCODE_TIMEOUT;
			return RFTRANS_95HF_ERRORCODE_TIMEOUT;	
		}
		
		/* Second step  - Receiving bytes 	*/
		drv95HF_ReceiveSPIResponse(pResponse);
	}
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
	{
		/* Second step - Receiving bytes 	*/
		drv95HF_ReceiveUARTResponse(pResponse);
	}
#endif /* CR95HF */
	return RFTRANS_95HF_SUCCESS_CODE; 
}

/**
 *	@brief  Send a negative pulse on IRQin pin
 *  @param  none
 *  @retval None
 */
void drv95HF_SendIRQINPulse(void)
{
	if (drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
	{
		/* Send a pulse on IRQ_IN */
		RFTRANS_95HF_IRQIN_HIGH() ;
		delayHighPriority_ms(1);
		RFTRANS_95HF_IRQIN_LOW() ;
		delayHighPriority_ms(1);
		RFTRANS_95HF_IRQIN_HIGH() ;
	}
#ifdef CR95HF	
	else if (drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
	{
	 	/* Send a pulse on IRQ_IN (UART_TX) */
		UART_SendByte(RFTRANS_95HF_UART, 0x00);
	}
#endif /* CR95HF */
	
	/* Need to wait 10ms after the pulse before to send the first command */
	delayHighPriority_ms(10);

}


#ifdef CR95HF

/**
 *	@brief  this functions initializes UART in order to communicate with 95HF device
 *  @param  BaudRate : value of the Baudrate to use during UART communication
 *  @retval None 
 */
void drv95HF_InitializeUART (uc32 BaudRate)
{
	USART_DeInit(RFTRANS_95HF_UART);
	/* enables clock	*/
	drv95HF_RCCConfigUART( );
	/* configures RX and TX 	*/
	/* TX : alternate function push pull	*/
	/* RX : floating input  	*/
	drv95HF_GPIOConfigUART( );
	/* set protcol 	*/
	drv95HF_StructureConfigUART(BaudRate );
}
#endif /* CR95HF */

/**
 *	@brief  this functions put the ST95HF in sleep/hibernate mode
 *  @param  WU_source : Source selected to wake up the device (WU_TIMEOUT,WU_TAG,WU_FIELD,WU_IRQ,WU_SPI)
 *  @param  mode : Can be IDLE_SLEEP_MODE or IDLE_HIBERNATE_MODE
 *  @retval None 
 */
void drv95HF_Idle(uc8 WU_source, uc8 mode)
{
	uint8_t pCommand[] = {RFTRANS_95HF_COMMAND_IDLE, IDLE_CMD_LENTH, 0, 0, 0, 0, 0 ,0x18 ,0x00 ,0x00 ,0x60 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00};

	if (mode == IDLE_SLEEP_MODE) /* SLEEP */
	{
		/* Select the wake up source*/
		pCommand[2] = WU_source;
		/* Select SLEEP mode */
		if (WU_source == WU_FIELD)
		{
			pCommand[3] = GETMSB(SLEEP_FIELD_ENTER_CTRL);
			pCommand[4] = GETLSB(SLEEP_FIELD_ENTER_CTRL);
		}
		else
		{
			pCommand[3] = GETMSB(SLEEP_ENTER_CTRL);
			pCommand[4] = GETLSB(SLEEP_ENTER_CTRL);
		}
		pCommand[5] = GETMSB(SLEEP_WU_CTRL);
		pCommand[6] = GETLSB(SLEEP_WU_CTRL);
	}
	else /* HIBERNATE */
	{
		/* Select the wake up source, only IRQ is available for HIBERNATE mode*/
		pCommand[2] = WU_IRQ;
		/* Select HIBERNATE mode */
		pCommand[3] = GETMSB(HIBERNATE_ENTER_CTRL);
		pCommand[4] = GETLSB(HIBERNATE_ENTER_CTRL);
		pCommand[5] = GETMSB(HIBERNATE_WU_CTRL);
		pCommand[6] = GETLSB(HIBERNATE_WU_CTRL);
		pCommand[10] = 0x00;
	}
	
	/* Send the command */
	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
		/* First step  - Sending command 	*/
		drv95HF_SendSPICommand(pCommand);
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
		/* First step  - Sending command	*/
		drv95HF_SendUARTCommand(pCommand);
#endif /* CR95HF */
	
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

