/**
  ******************************************************************************
  * @file    drv_95HF.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides set of firmware functions to manage communication
	* @brief   between MCU and 95HF device (RX95HF, CR95HF or ST95HF) 
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

/* Define to prevent recursive inclusion ------------------------------------------------ */
#ifndef __DRIVER_RFTRANS_95HF_H
#define __DRIVER_RFTRANS_95HF_H

/* Includes ----------------------------------------------------------------------------- */
#include "drv_spi.h"
#ifdef CR95HF
#include "drv_uart.h"
#endif /* CR95HF */
#include "platform.h"

/* RFtransceiver HEADER command definition ---------------------------------------------- */
#define RFTRANS_95HF_COMMAND_SEND															0x00
#define RFTRANS_95HF_COMMAND_RESET														0x01
#define RFTRANS_95HF_COMMAND_RECEIVE													0x02
#define RFTRANS_95HF_COMMAND_POLLING													0x03
#define RFTRANS_95HF_COMMAND_IDLE															0x07

/* RFtransceiver mask and data to check the data (SPI polling)--------------------------- */
#define RFTRANS_95HF_FLAG_DATA_READY													0x08
#define RFTRANS_95HF_FLAG_DATA_READY_MASK											0x08

/* RF transceiver status	--------------------------------------------------------------- */
#define RFTRANS_95HF_SUCCESS_CODE															RESULTOK
#define RFTRANS_95HF_NOREPLY_CODE															0x01
#define	RFTRANS_95HF_ERRORCODE_DEFAULT												0xFE
#define	RFTRANS_95HF_ERRORCODE_TIMEOUT												0xFD
#define RFTRANS_95HF_ERRORCODE_POR														0x44

/* RF transceiver polling status	------------------------------------------------------- */
#define RFTRANS_95HF_POLLING_RFTRANS_95HF											0x00
#define RFTRANS_95HF_POLLING_TIMEOUT													0x01

/* RF transceiver number of byte of the buffers------------------------------------------ */
#define RFTRANS_95HF_RESPONSEBUFFER_SIZE											0x20E
#define RFTRANS_95HF_MAX_BUFFER_SIZE													0x20E
#define RFTRANS_95HF_MAX_ECHO																	768		// Max of 256 (for QJC) and 768 (for QJE. Should be 512 but bug Design)

/* RF transceiver Offset of the command and the response -------------------------------- */
#define RFTRANS_95HF_COMMAND_OFFSET														0x00
#define RFTRANS_95HF_LENGTH_OFFSET														0x01
#define RFTRANS_95HF_DATA_OFFSET															0x02

/* ECHO response ------------------------------------------------------------------------ */
#define ECHORESPONSE																					0x55

/* Sleep parameters --------------------------------------------------------------------- */
#define IDLE_SLEEP_MODE																				0x00
#define IDLE_HIBERNATE_MODE																		0x01

#define IDLE_CMD_LENTH																				0x0E

#define WU_TIMEOUT																						0x01
#define WU_TAG																								0x02
#define WU_FIELD																							0x04
#define WU_IRQ																								0x08
#define WU_SPI																								0x10

#define HIBERNATE_ENTER_CTRL																	0x0400
#define SLEEP_ENTER_CTRL																			0x0100
#define SLEEP_FIELD_ENTER_CTRL																0x0142

#define HIBERNATE_WU_CTRL																			0x0400
#define SLEEP_WU_CTRL																					0x3800

#define LEAVE_CTRL																						0x1800

/* Calibration parameters---------------------------------------------------------------- */
#define WU_SOURCE_OFFSET																			0x02
#define WU_PERIOD_OFFSET																			0x09
#define DACDATAL_OFFSET																				0x0C
#define DACDATAH_OFFSET																				0x0D
#define NBTRIALS_OFFSET																				0x0F

/* baud rate command -------------------------------------------------------------------- */
#define	BAUDRATE_LENGTH						  													0x01
#define	BAUDRATE_DATARATE_DEFAULT			  											57600

#define MAX_BUFFER_SIZE  																			528 

#define ECHO																									0x55
#define LISTEN																								0x05
#define DUMMY_BYTE																						0xFF


// set state on IRQ in pin
//#define RFTRANS_95HF_GET_INTERFACE() 		GPIO_ReadInputDataBit(INTERFACE_GPIO_PORT,INTERFACE_PIN)
#define RFTRANS_95HF_GET_INTERFACE()		platformGpioIsHigh(ST25R95_N_IRQ_IN_PORT, ST25R95_N_IRQ_IN_PIN)


#define READERREPLY_MAX_BUFFER_SIZE			0x40	

/** @addtogroup _95HF_Driver
 * 	@{
 */

/** @addtogroup drv_95HF
 * 	@{
 */

typedef enum {
	RFTRANS_95HF_INTERFACE_UART = 0,
	RFTRANS_95HF_INTERFACE_SPI,
	RFTRANS_95HF_INTERFACE_TWI
}RFTRANS_95HF_INTERFACE;

typedef enum {
	RFTRANS_95HF_SPI_POLLING = 0,
	RFTRANS_95HF_SPI_INTERRUPT,
}RFTRANS_95HF_SPI_MODE;


/**
 *	@brief  the different states of the RF transceiver
 */
typedef enum {
	RFTRANS_95HF_STATE_UNKNOWN = 0,
	RFTRANS_95HF_STATE_HIBERNATE ,
	RFTRANS_95HF_STATE_SLEEP,
	RFTRANS_95HF_STATE_POWERUP,
	RFTRANS_95HF_STATE_TAGDETECTOR,
	RFTRANS_95HF_STATE_READY,
	RFTRANS_95HF_STATE_READER,
	RFTRANS_95HF_STATE_TAGHUNTING,
}RFTRANS_95HF_STATE;

/**
 *	@brief  the RF transceiver can be configured as either a reader or a card emulator
 *	@brief  or as P2P device
 */
typedef enum {
	RFTRANS_95HF_MODE_UNKNOWN = 0,
	RFTRANS_95HF_MODE_READER ,
	RFTRANS_95HF_MODE_CARDEMULATOR ,
	RFTRANS_95HF_MODE_PASSIVEP2P ,
	RFTRANS_95HF_MODE_ACTIVEP2P ,
}RFTRANS_95HF_MODE;

/**
 *	@brief  the Rf transceiver supports the differrent protocols
 */
typedef enum {
	RFTRANS_95HF_PROTOCOL_UNKNOWN = 0,
	RFTRANS_95HF_PCD_14443A,
	RFTRANS_95HF_PCD_14443B,
	RFTRANS_95HF_PCD_15693,
	RFTRANS_95HF_PCD_18092,
	RFTRANS_95HF_PICC_14443A,
	RFTRANS_95HF_PICC_14443B,
	RFTRANS_95HF_PICC_15693,
	RFTRANS_95HF_PICC_18092,
}RFTRANS_95HF_PROTOCOL;		


/**
 *	@brief  structure to store driver information
 */
typedef struct {
	
	RFTRANS_95HF_INTERFACE 			uInterface;
	RFTRANS_95HF_STATE 					uState;
	RFTRANS_95HF_MODE 					uMode;
	RFTRANS_95HF_SPI_MODE 			uSpiMode;
	RFTRANS_95HF_PROTOCOL				uCurrentProtocol;

}drv95HF_ConfigStruct;


/* Functions ---------------------------------------------------------------- */

void 		drv95HF_ResetSPI (void);
int8_t 	drv95HF_GetInterfacePinState ( void );
uint8_t drv95HF_GetSerialInterface ( void );
int8_t 	drv95HF_GetIRQOutState ( void );

void  	drv95HF_InitConfigStructure (void);

void 		drv95HF_InitilizeSerialInterface ( void );
void 		drv95HF_ReceiveSPIResponse ( uint8_t *pData );
void 		drv95HF_SendSPICommand ( uc8 *pData );


int8_t  drv95HF_SendReceive ( uc8 *pCommand, uint8_t *pResponse );
void  	drv95HF_SendCmd ( uc8 *pCommand );
int8_t  drv95HF_PoolingReading ( uint8_t *pResponse );

void drv95HF_Idle(uc8 WU_source, uc8 mode);


void drv95HF_SendIRQINPulse(void);

void drv95HF_EnableInterrupt(void);
void drv95HF_DisableInterrupt(void);

#ifdef CR95HF
void 		drv95HF_InitializeUART ( uc32 BaudRate );
#endif /* CR95HF */

/**
  * @}
  */

/**
  * @}
  */
#endif /* __RFTRANS_95HF_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/



