/**
  ******************************************************************************
  * @file    lib_proprietaryP2P.h
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

#ifndef __LIB_PROPRIETARYP2P_H
#define __LIB_PROPRIETARYP2P_H

#include "lib_PCD.h"
#include "lib_PICCEmulator.h"
#include "lib_picc.h"

#define PP2P_SUCCESS													  0x00
#define PP2P_ERRORCODE_DEFAULT  							  0x02

/* Initiator functions */
uint8_t PP2PI_SendRequest(uint8_t *pData, uint16_t *size);
uint8_t PP2PI_ReceiveRequest(uint8_t *pData, uint16_t *size);
uint8_t PP2PI_SendAnswer(uint8_t *pData, uint16_t *size);
uint8_t PP2PI_SendSymm(void);

/* Instruction definition for Proprietary P2P */
#define CMD_INS			0x01		// CMD Instruction
#define DAT_INS			0x07		// Data Instruction
#define RTOK_INS		0x02    // Receive/Give Token instruction (Client/Server)
#define SYM_INS			0x03    // SYMETRIE Instruction
#define BUF_INS			0x04    // Buffer Instruction
#define PTOK_INS		0x05		// Preemption Token Instruction
#define CTOK_INS		0x06		// Client Claim Token Instruction

typedef enum  {
		STATE_NOREQRES_PENDING = 0, // Typically SYM Sequence
		STATE_IN_CMDREQ_PENDING,		// Current Device send a CMD REQ
		STATE_IN_DATREQ_PENDING,		// Current Device send a DATA REQ
		STATE_IN_RELREQ_PENDING,		// Current Device send a release Token REQ
		STATE_IN_PREREQ_PENDING,		// Current Device send a Preemption Token REQ
		STATE_IN_ACK_PENDING,				// A ACK/NACK is expected from WOD
		STATE_OUT_CMDRES_PENDING,		// WOD Device response a CMD REQ
		STATE_OUT_DATRES_PENDING,   // WOD Device response a DATA REQ
		STATE_OUT_GETRES_PENDING,   // WOD Device response a communication REQ
		STATE_OUT_REQRES_PENDING,   // WOD Wait For preemption Token ANS
		STATE_OUT_ACK_PENDING,			// A ACK/NACK is pending from current APP layer to be sent to WOD
		STATE_OUT_NACK_PENDING
} PP2P_STATES;


typedef enum { 
	STD_CMD_TYPE = 0,
	DATA_TYPE,
	UNDEF_TYPE
} frameType, *pframeType;

	
/* Initiator functions (Server) */
void PP2P_ServerInit(void);
uint8_t PP2P_Server(void);	
void P2P_SendACK(bool ack);
uint8_t PP2P_SendCMD(uint8_t *pData, uint16_t size );
uint8_t PP2P_SendDATA(uint8_t *pData, uint16_t size );
uint8_t PP2P_SendBUF(uint8_t *pData, uint16_t *size );
	
/* Target functions (Client) */
void PP2P_TargetInit(void);
void PP2P_GetCMD(uint8_t *pData );
void PP2P_GetData(uint8_t *pData);
void PP2P_GetBuf(uint8_t *pData);
void PP2P_ReleaseCommunicationToken(uint8_t Pcb);
void PP2P_GetSYM(uint8_t Pcb);
void PP2P_GetPreemptionToken(uint8_t Pcb);
void PP2P_UnsupportedREQ(uint8_t Pcb);
void PP2P_GetACK(uint8_t Pcb);
void PP2P_GetNACK(uint8_t Pcb);


#endif //__LIB_PROPRIETARYP2P_H
	
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
