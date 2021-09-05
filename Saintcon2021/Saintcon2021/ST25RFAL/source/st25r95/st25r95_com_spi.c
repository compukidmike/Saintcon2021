
/******************************************************************************
  * @attention
  *
  * COPYRIGHT 2018 STMicroelectronics, all rights reserved
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/


/*
 *      PROJECT:   ST25R95 firmware
 *      $Revision: $
 *      LANGUAGE:  ISO C99
 */

/*! \file st25r95_com_spi.c
 *
 *  \author 
 *
 *  \brief Implementation of ST25R95 communication.
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
 
#include "st25r95_com.h"
#include "st25r95.h"
#include "string.h"
#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_rf.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */
 
 #if !(ST25R95_INTERFACE_UART) /* ST25R95_INTERFACE_SPI */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Pos           (3U)                                                                                       /*!< SPI poll flag bit 3: Data can be read when set */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk           (0x1U << ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Pos)                                           /*!< Mask 0x08 */  
#define ST25R95_POLL_FLAG_DATA_CAN_BE_READ               ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk                                                     /*!< 0x08 */
#define ST25R95_POLL_DATA_CAN_BE_READ(Flags)             (((Flags) & ST25R95_POLL_FLAG_DATA_CAN_BE_READ_Msk) == ST25R95_POLL_FLAG_DATA_CAN_BE_READ) /*!< SPI read poll flag test */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Pos           (2U)                                                                                       /*!< SPI poll flag bit 2: Data can be send when set */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk           (0x1U << ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Pos)                                           /*!< Mask 0x04 */
#define ST25R95_POLL_FLAG_DATA_CAN_BE_SEND               ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk                                                     /*!< 0x04 */
#define ST25R95_POLL_DATA_CAN_BE_SEND(Flags)             (((Flags) & ST25R95_POLL_FLAG_DATA_CAN_BE_SEND_Msk) == ST25R95_POLL_FLAG_DATA_CAN_BE_SEND) /*!< SPI send poll flag test*/

#define ST25R95_DEBUG false

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static uint8_t EchoCommand[1] = {ST25R95_COMMAND_ECHO};
static uint8_t Idle[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x0A, 0x21, 0x00, 0x38, 0x01, 0x18, 0x00, 0x20, 0x60, 0x60, 0x74, 0x84, 0x3F, 0x00};

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
 
st25r95SPIRxContext st25r95SPIRxCtx;

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
void st25r95SPIRxTx(uint8_t *txData, uint8_t *rxData, uint16_t length)
{
    uint8_t txByte = 0;
    uint8_t rxByte;
    
    while (length != 0)
    {
        platformSpiTxRx((txData == NULL) ? &txByte : txData++, (rxData == NULL) ? &rxByte : rxData++, 1);
        length--;
    }
}

/*******************************************************************************/
uint8_t st25r95SPISendReceiveByte(uint8_t data)
{
    uint8_t received_byte;
    
    platformSpiTxRx(&data, &received_byte, 1);
    return (received_byte);
}

/*******************************************************************************/
ReturnCode st25r95SPIPollRead(uint32_t timeout)
{
    uint32_t timer;
    ReturnCode retCode = ERR_NONE;   
    
    timer = platformTimerCreate(timeout);
	//char data[10] = {0};
	//itoa(timer,data,10);
	//platformLog(data);
	//platformGpioClear(1,NFC_IRQ_IN_PIN);
    while (platformGpioIsHigh(ST25R95_N_IRQ_OUT_PORT, ST25R95_N_IRQ_OUT_PIN) && (timeout != 0) && !platformTimerIsExpired(timer)) {;}
	//while (platformGpioIsHigh(ST25R95_N_IRQ_OUT_PORT, ST25R95_N_IRQ_OUT_PIN) && !platformTimerIsExpired(timer)) {;}
    //platformGpioSet(1,NFC_IRQ_IN_PIN);
    if (platformGpioIsHigh(ST25R95_N_IRQ_OUT_PORT, ST25R95_N_IRQ_OUT_PIN))
    {
        retCode = ERR_TIMEOUT;
    }
    
    platformTimerDestroy(timer);
    
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95SPIPollSend(void)
{
    ReturnCode retCode = ERR_NONE;
    uint8_t response;
    
    
    platformSpiSelect();
    st25r95SPISendReceiveByte(ST25R95_CONTROL_POLL);
    response = st25r95SPISendReceiveByte(ST25R95_CONTROL_POLL);
    platformSpiDeselect();
    if (!ST25R95_POLL_DATA_CAN_BE_SEND(response))
    {
        retCode = ERR_TIMEOUT;
    }
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95SPISendCommandTypeAndLen(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen)
{
    ReturnCode retCode = ERR_NONE;
    uint32_t len;
    
    if (respBuffLen < 2)
    {
        retCode = ERR_NOMEM;
    }
    else
    {
        resp[ST25R95_CMD_RESULT_OFFSET] = ST25R95_ERRCODE_COMERROR;
        resp[ST25R95_CMD_LENGTH_OFFSET] = 0x00;
        
        /* 1 - Send the  command */
        platformSpiSelect();
        st25r95SPISendReceiveByte(ST25R95_CONTROL_SEND);
        st25r95SPIRxTx(cmd, NULL, cmd[ST25R95_CMD_LENGTH_OFFSET] + 2);
        platformSpiDeselect();
        #if ST25R95_DEBUG
        platformLog("[%10d] >>>> %s\r\n", platformGetSysTick(), hex2Str(cmd, cmd[ST25R95_CMD_LENGTH_OFFSET] + 2));
        #endif /* ST25R95_DEBUG */
        /* 2 - Poll the ST25R95 until it is ready to transmit */
        retCode = st25r95SPIPollRead(ST25R95_CONTROL_POLL_TIMEOUT);

        if (retCode == ERR_NONE) 
        {
            platformSpiSelect();
            st25r95SPISendReceiveByte(ST25R95_CONTROL_READ);    
            resp[ST25R95_CMD_RESULT_OFFSET] = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);
            resp[ST25R95_CMD_LENGTH_OFFSET] = st25r95SPISendReceiveByte(resp[ST25R95_CMD_RESULT_OFFSET]);
            len = resp[ST25R95_CMD_LENGTH_OFFSET];
            /* compute len according to CR95HF DS § 4.4 */
            if ((resp[ST25R95_CMD_RESULT_OFFSET] & 0x8F) == 0x80)
            {
                len |= (((uint32_t)resp[ST25R95_CMD_RESULT_OFFSET]) & 0x60U) << 3U;
            }
            /* read the len-bytes frame */
            if (respBuffLen >= (len + 2))
            {
                if (len != 0)
                {
                    st25r95SPIRxTx(NULL, &resp[ST25R95_CMD_DATA_OFFSET], len);
                }
            }
            else
            {
                st25r95FlushChipSPIBuffer();
                retCode = ERR_NOMEM;
            }
            platformSpiDeselect();
            #if ST25R95_DEBUG
            platformLog("[%10d] <<<< %s\r\n", platformGetSysTick(), hex2Str(resp, len + 2));
            #endif /* ST25R95_DEBUG */
        }
        else
        {
            platformSpiSelect();
            st25r95FlushChipSPIBuffer();
            platformSpiDeselect();
            retCode = ERR_SYSTEM;
        }
    }
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95SPICommandEcho(void)
{
    ReturnCode retCode = ERR_NONE;
    uint8_t respBuffer[ST25R95_ECHO_RESPONSE_BUFLEN];
    
    /* 0 - Poll the ST25R95 to make sure data can be send */
    /* Used only in cas of ECHO Command as this command is sent just after the ST25R95 reset */
    retCode = st25r95SPIPollSend();
    delay_ms(1);
    if (retCode == ERR_NONE)
    {
        /* 1 - Send the echo command */
        platformSpiSelect();
        st25r95SPISendReceiveByte(ST25R95_CONTROL_SEND);    
        st25r95SPISendReceiveByte(EchoCommand[0]);    
        platformSpiDeselect();
#if ST25R95_DEBUG        
        platformLog("[%10d] >>>> %2.2x\r\n", platformGetSysTick(), ST25R95_COMMAND_ECHO);
#endif /* ST25R95_DEBUG */
        
        /* 2 - Poll the ST25R95 until it is ready to transmit */
        retCode = st25r95SPIPollRead(ST25R95_CONTROL_POLL_TIMEOUT);
        
        /* 3 - Read echo response */
        if (retCode == ERR_NONE) 
        {
            platformSpiSelect();
            st25r95SPISendReceiveByte(ST25R95_CONTROL_READ);    
            respBuffer[ST25R95_CMD_RESULT_OFFSET] = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);
            /* Read 2 additional bytes. See  ST95HF DS §5.7 :
             * The ECHO command (0x55) allows exiting Listening mode. 
             * In response to the ECHO command, the ST25R95 sends 0x55 + 0x8500 (error code of the Listening state cancelled by the MCU).
             */
            respBuffer[1] = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);  
            respBuffer[2] = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);
            platformSpiDeselect();
#if ST25R95_DEBUG            
            platformLog("[%10d] <<<< %s\r\n", platformGetSysTick(), hex2Str(respBuffer, 3));
#endif /* ST25R95_DEBUG */

            if (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_COMMAND_ECHO)
            {
#if ST25R95_DEBUG  
                platformLog("%s: unexepected echo response: %2.2x\r\n", __FUNCTION__, respBuffer[ST25R95_CMD_RESULT_OFFSET]);
#endif /* ST25R95_DEBUG */
                st25r95FlushChipSPIBuffer();
                retCode = ERR_SYSTEM;
            }
        }
    }
    #if RFAL_FEATURE_LISTEN_MODE
    st25r95SPIRxCtx.inListen = false;
    #endif /* RFAL_FEATURE_LISTEN_MODE */
    
#if ST25R95_DEBUG  
    platformLog("%s: retCode: %2.2x\r\n", __FUNCTION__, retCode);
#endif /* ST25R95_DEBUG */   
    return (retCode);
}

/*******************************************************************************/
void st25r95SPISendData(uint8_t *buf, uint8_t bufLen, uint8_t protocol, uint32_t flags)
{
    uint8_t len;
 
#if ST25R95_DEBUG
    platformLog("[%10d] DATA >>>> %s", platformGetSysTick(), hex2Str(buf, bufLen));  
#endif /* ST25R95_DEBUG */

    platformSpiSelect();
    st25r95SPISendReceiveByte(ST25R95_CONTROL_SEND);
    if (protocol == ST25R95_PROTOCOL_CE_ISO14443A)
    {
        /* Card Emulation mode */
        st25r95SPISendReceiveByte(ST25R95_COMMAND_SEND);
    }
    else
    {
        st25r95SPISendReceiveByte(ST25R95_COMMAND_SENDRECV);
    }
    /* add transmission Flag Len in case of 14443A */
    len = ((protocol == ST25R95_PROTOCOL_ISO14443A) || (protocol == ST25R95_PROTOCOL_CE_ISO14443A)) ? bufLen + 1: bufLen;
    /* add SoD len in case of ISO14443A + NFCIP1 */
    len += ((protocol == ST25R95_PROTOCOL_ISO14443A) && ((flags & RFAL_TXRX_FLAGS_NFCIP1_ON) == RFAL_TXRX_FLAGS_NFCIP1_ON)) ? 2 : 0;
    st25r95SPISendReceiveByte(len);
    if ((protocol == ST25R95_PROTOCOL_ISO14443A) && ((flags & RFAL_TXRX_FLAGS_NFCIP1_ON) == RFAL_TXRX_FLAGS_NFCIP1_ON))
    {
        st25r95SPISendReceiveByte(0xF0U);
        st25r95SPISendReceiveByte(bufLen + 1); /* DP 2.0 17.4.1.3 The SoD SHALL contain a length byte LEN at the position shown in Figure 43 with a value equal to n+1, where n indicates the number of bytes the payload consists of.*/
    }
    st25r95SPIRxTx(buf, NULL, bufLen);

}

/*******************************************************************************/
void st25r95SPISendTransmitFlag(uint8_t protocol, uint8_t transmitFlag)
{
    if ((protocol == ST25R95_PROTOCOL_ISO14443A) || (protocol == ST25R95_PROTOCOL_CE_ISO14443A))
    {
#if ST25R95_DEBUG
        platformLog(" %2.2X", transmitFlag);
#endif /* ST25R95_DEBUG */       
 
        /* send transmission Flag */
        st25r95SPISendReceiveByte(transmitFlag);
    }
#if ST25R95_DEBUG
    platformLog("\r\n"); 
#endif /* ST25R95_DEBUG */ 
    platformSpiDeselect();
    
}

/*******************************************************************************/
void st25r95SPIPrepareRx(uint8_t protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes)
{
    st25r95SPIRxCtx.protocol            = protocol;
    st25r95SPIRxCtx.rxBuf               = rxBuf;
    st25r95SPIRxCtx.rxBufLen            = rxBufLen;
    st25r95SPIRxCtx.rxRcvdLen           = rxRcvdLen;
    st25r95SPIRxCtx.rmvCRC              = ((flags & RFAL_TXRX_FLAGS_CRC_RX_KEEP) != RFAL_TXRX_FLAGS_CRC_RX_KEEP);
    st25r95SPIRxCtx.NFCIP1              = ((protocol == ST25R95_PROTOCOL_ISO14443A) && ((flags & RFAL_TXRX_FLAGS_NFCIP1_ON) == RFAL_TXRX_FLAGS_NFCIP1_ON));
    st25r95SPIRxCtx.additionalRespBytes = additionalRespBytes;
}

/*******************************************************************************/
ReturnCode st25r95SPICompleteRx(void)
{
    uint8_t Result;
#if ST25R95_DEBUG
    uint8_t initialResult;
    int16_t initialLen;
#endif /* ST25R95_DEBUG */
    uint16_t len;
    uint16_t rcvdLen;
    rfalBitRate rxBr;
    ReturnCode retCode = ERR_NONE;
    uint16_t additionalRespBytesNb = 1;
    
    
#if ST25R95_DEBUG   
    NO_WARNING(initialResult); /* debug purposes */
    NO_WARNING(initialLen);    /* debug purposes */
#endif /* ST25R95_DEBUG */
    
    platformSpiSelect();
    st25r95SPISendReceiveByte(ST25R95_CONTROL_READ);    
    Result = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);
    len = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);
#if ST25R95_DEBUG
    initialResult = Result;
    initialLen = len;
#endif /* ST25R95_DEBUG */

    /* compute len according to CR95HF DS § 4.4 */
    if ((Result & 0x8F) == 0x80)
    {
        len |= (((uint32_t)Result) & 0x60) << 3;
        Result &= 0x9F;
    }
    rcvdLen = 0;
        
    switch (Result)
    {
        case ST25R95_ERRCODE_NONE:
        case ST25R95_ERRCODE_FRAMEOKADDITIONALINFO:
        case ST25R95_ERRCODE_RESULTSRESIDUAL:
            break;
        case ST25R95_ERRCODE_COMERROR:
            retCode = ERR_INTERNAL;
            break;
        case ST25R95_ERRCODE_FRAMEWAITTIMEOUT:
            retCode = ERR_TIMEOUT;
            break;
        case ST25R95_ERRCODE_OVERFLOW:
            retCode = ERR_HW_OVERRUN;
            break;
        case ST25R95_ERRCODE_INVALIDSOF:
        case ST25R95_ERRCODE_RECEPTIONLOST:
        case ST25R95_ERRCODE_FRAMING:
        case ST25R95_ERRCODE_EGT:
        case ST25R95_ERRCODE_61_SOF:
        case ST25R95_ERRCODE_63_SOF_HIGH:
        case ST25R95_ERRCODE_65_SOF_LOW:
        case ST25R95_ERRCODE_66_EGT:
        case ST25R95_ERRCODE_67_TR1TOOLONG:
        case ST25R95_ERRCODE_68_TR1TOOSHORT:
            retCode = ERR_FRAMING;
            break; 
        case ST25R95_ERRCODE_62_CRC:   
            retCode = ERR_CRC;
            break; 
        case ST25R95_ERRCODE_NOFIELD:
            retCode = ERR_LINK_LOSS;
            break;
        default:
            retCode = ERR_SYSTEM;
            break;
    }
    
    if ((retCode != ERR_NONE) && (len != 0))
    {
        st25r95FlushChipSPIBuffer();
        len = 0;
    }
    
    
    /* In ISO14443A 106kbps 2 additional bytes of collision information are provided */
    rfalGetBitRate( NULL, &rxBr );
    if( (st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_ISO14443A) && (rxBr == RFAL_BR_106) )
    {
        additionalRespBytesNb += 2;
    }
    
    
    /* read the frame */
    do {
        if (len == 0)
        {
            additionalRespBytesNb = 0;
            break;
        }
        if (len < additionalRespBytesNb)
        {
            /* Flush ST25R95 fifo */
            st25r95FlushChipSPIBuffer();
            retCode = ERR_SYSTEM;
            break;
        }
        len -= additionalRespBytesNb;
        if ((Result == ST25R95_ERRCODE_RESULTSRESIDUAL) && (st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_ISO14443A))
        {
                st25r95SPIRxCtx.rmvCRC = false;
        }
        if ((st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol != ST25R95_PROTOCOL_ISO18092))
        {
            if (len < 2)
            {
                /* Flush ST25R95 fifo */
                st25r95FlushChipSPIBuffer();
                additionalRespBytesNb = 0;
                retCode = ERR_SYSTEM;
                break;
            }
            len -= 2;
        }
        if ((st25r95SPIRxCtx.NFCIP1) && (len >= 1))
        {
            st25r95SPIRxTx(NULL, st25r95SPIRxCtx.NFCIP1_SoD, 1);
            len -= 1;
        }
        if ((len > st25r95SPIRxCtx.rxBufLen) ||
            ((st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_ISO18092) && ((len + 1U) > st25r95SPIRxCtx.rxBufLen)) || /* Need one extra byte room to prepend Len byte in rxBuf in case of Felica */
            ((!st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_ISO18092) && ((len + 3U) > st25r95SPIRxCtx.rxBufLen))) /* same + 2 extra bytes room to append CRC */
        {
            /* Flush ST25R95 fifo */
            st25r95FlushChipSPIBuffer();
            additionalRespBytesNb = 0;
            retCode = ERR_NOMEM;
            break;
        }
        rcvdLen = len;
        if (len != 0)
        {
            if (st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_ISO18092)
            {
                st25r95SPIRxTx(NULL, &st25r95SPIRxCtx.rxBuf[RFAL_NFCF_LENGTH_LEN], len);
                rcvdLen += RFAL_NFCF_LENGTH_LEN;
                len += RFAL_NFCF_LENGTH_LEN;
                st25r95SPIRxCtx.rxBuf[0] = (uint8_t)(rcvdLen & 0xFFU);
            }
            else
            {
                st25r95SPIRxTx(NULL, st25r95SPIRxCtx.rxBuf, len);
            }
        }
        if ((st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol != ST25R95_PROTOCOL_ISO18092))
        {
            st25r95SPIRxTx(NULL, st25r95SPIRxCtx.BufCRC, 2);
        }
        st25r95SPIRxTx(NULL, st25r95SPIRxCtx.additionalRespBytes, additionalRespBytesNb);
     
        /* check collision and CRC error */
        switch (st25r95SPIRxCtx.protocol)
        {
            case (ST25R95_PROTOCOL_ISO15693):
                retCode = ST25R95_IS_PROT_ISO15693_COLLISION_ERR(st25r95SPIRxCtx.additionalRespBytes[0]) ? ERR_RF_COLLISION : (ST25R95_IS_PROT_ISO15693_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0]) ? ERR_CRC : retCode);
                break;
            case (ST25R95_PROTOCOL_ISO14443A):
                retCode = (Result == ST25R95_ERRCODE_RESULTSRESIDUAL) ? ((ReturnCode)(ERR_INCOMPLETE_BYTE + ((st25r95SPIRxCtx.additionalRespBytes[0] & 0xFU) % 8U))) : (ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(st25r95SPIRxCtx.additionalRespBytes[0]) ? ERR_RF_COLLISION : (ST25R95_IS_PROT_ISO14443A_PARITY_ERR(st25r95SPIRxCtx.additionalRespBytes[0]) ? ERR_PAR : (ST25R95_IS_PROT_ISO14443A_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0]) ? ERR_CRC : retCode)));
                break;
            case (ST25R95_PROTOCOL_ISO14443B):
                if (ST25R95_IS_PROT_ISO14443B_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
                {
                    retCode = ERR_CRC;
                }
                break;
            case (ST25R95_PROTOCOL_ISO18092):
                if (ST25R95_IS_PROT_ISO18092_CRC_ERR(st25r95SPIRxCtx.additionalRespBytes[0]))
                {
                    retCode = ERR_CRC;
                }
                break;                        
            default:
                break;
        }
    } while (0);
   
    platformSpiDeselect();
#if ST25R95_DEBUG
    platformLog("[%10d] DATA <<<<(0x%2.2X%2.2X) %s%s", platformGetSysTick(), initialResult, initialLen, (st25r95SPIRxCtx.NFCIP1) ? hex2Str(st25r95SPIRxCtx.NFCIP1_SoD, 1) : "", (rcvdLen == len) ? hex2Str(st25r95SPIRxCtx.rxBuf, (rcvdLen)): "<error>");
    if ((st25r95SPIRxCtx.rmvCRC) && (additionalRespBytesNb != 0) && (st25r95SPIRxCtx.protocol != ST25R95_PROTOCOL_ISO18092))
    {
        platformLog("[%s]", hex2Str(st25r95SPIRxCtx.BufCRC, 2));
    }
#endif /* ST25R95_DEBUG */

    if ((!st25r95SPIRxCtx.rmvCRC) && (st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_ISO18092) && (rcvdLen == len))
    {
        /* increase room for CRC*/
        st25r95SPIRxCtx.rxBuf[rcvdLen++] = 0x00;
        st25r95SPIRxCtx.rxBuf[rcvdLen++] = 0x00;
#if ST25R95_DEBUG
        platformLog("{%4.4X}", 0x0000);
#endif /* ST25R95_DEBUG */
        
    }
    
#if ST25R95_DEBUG
    platformLog(" %s", hex2Str(st25r95SPIRxCtx.additionalRespBytes, additionalRespBytesNb));
    platformLog(" (retCode=%d)\r\n", retCode);
#endif /* ST25R95_DEBUG */
    
    /* update *rxRcvdLen if not null pointer */
    if (st25r95SPIRxCtx.rxRcvdLen != NULL)
    {
        (*st25r95SPIRxCtx.rxRcvdLen) = rcvdLen;
    }
    #if RFAL_FEATURE_LISTEN_MODE
    if (st25r95SPIRxCtx.protocol == ST25R95_PROTOCOL_CE_ISO14443A)
    {
        st25r95SPIRxCtx.inListen = false;
        st25r95SPIGetLmState(); /* store lmState */
    }
    #endif /* RFAL_FEATURE_LISTEN_MODE */
    st25r95SPIRxCtx.retCode = retCode;
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95SPIGetRxStatus(void)
{
    return (st25r95SPIRxCtx.retCode);
}

bool st25r95SPIIsTransmitCompleted(void)
{
    return (true);
}

bool st25r95SPIIsInListen(void)
{
    return (st25r95SPIRxCtx.inListen);
}

/*******************************************************************************/
void st25r95SPIIdle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod)
{
    Idle[ST25R95_IDLE_WUPERIOD_OFFSET] = WUPeriod;
    Idle[ST25R95_IDLE_DACDATAL_OFFSET] = dacDataL;
    Idle[ST25R95_IDLE_DACDATAH_OFFSET] = dacDataH;
    platformSpiSelect();
    st25r95SPISendReceiveByte(ST25R95_CONTROL_SEND);
    st25r95SPIRxTx(Idle, NULL, Idle[ST25R95_CMD_LENGTH_OFFSET] + 2);
    platformSpiDeselect();
    #if ST25R95_DEBUG
    platformLog("[%10d] >>>> %s\r\n", platformGetSysTick(), hex2Str(Idle, Idle[ST25R95_CMD_LENGTH_OFFSET] + 2));
    #endif /* ST25R95_DEBUG */
}

/*******************************************************************************/
void st25r95SPIGetIdleResponse(void)
{
    uint8_t respBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];
    
    platformSpiSelect();
    st25r95SPISendReceiveByte(ST25R95_CONTROL_READ);    
    respBuffer[ST25R95_CMD_RESULT_OFFSET] = st25r95SPISendReceiveByte(ST25R95_SPI_DUMMY_BYTE);
    respBuffer[ST25R95_CMD_LENGTH_OFFSET] = st25r95SPISendReceiveByte(respBuffer[ST25R95_CMD_RESULT_OFFSET]);
    if ((sizeof(respBuffer)) >= (respBuffer[ST25R95_CMD_LENGTH_OFFSET] + 2U))
    {
        if (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0)
        {
            st25r95SPIRxTx(NULL, &respBuffer[ST25R95_CMD_DATA_OFFSET], respBuffer[ST25R95_CMD_LENGTH_OFFSET]);
        }
    }
    else
    {
        st25r95FlushChipSPIBuffer();
    }
    platformSpiDeselect();
    #if ST25R95_DEBUG
    platformLog("[%10d] <<<< %s\r\n", platformGetSysTick(), hex2Str(respBuffer, respBuffer[ST25R95_CMD_LENGTH_OFFSET] + 2));
    #endif /* ST25R95_DEBUG */
 
}

/*******************************************************************************/
void st25r95SPIKillIdle(void)
{
    ReturnCode retCode = ERR_NONE;
    
    st25r95SPI_nIRQ_IN_Pulse();
    /* Poll the ST25R95 until it is ready to transmit */
    retCode = st25r95SPIPollRead(ST25R95_CONTROL_POLL_TIMEOUT);
        
    if (retCode == ERR_NONE)
    {
        st25r95SPIGetIdleResponse();
    }
    
}
#endif /* ST25R95_INTERFACE_SPI */
