
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

/*! \file st25r95_com_uart.c
 *
 *  \author 
 *
 *  \brief Implementation of ST25R95 communication (UART layer).
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
 
#if ST25R95_INTERFACE_UART

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*
 * States for the UART RXx Callback
 */
typedef enum
{
    ST25R95_CB_RESLEN          = 0U, /* Result and Len bytes received */
    ST25R95_CB_DATA            = 1U, /* Data received */
    ST25R95_CB_CRC             = 2U, /* CRC received */
    ST25R95_CB_ADDITIONALBYTES = 3U, /* Rx flags received */
    ST25R95_CB_FLUSHUART       = 4U, /* Flush ongoing */
    ST25R95_CB_SOD             = 5U  /* SoD received */
} st25r95_RxState;

typedef struct
{
    bool rmvCRC;
    bool NFCIP1;
    uint16_t rxBufLen;
    uint16_t *rxRcvdLen;
    uint8_t protocol;
    uint8_t *rxBuf;
    uint8_t *additionalRespBytes;
    ReturnCode retCode;
    st25r95_RxState RxState;
    uint8_t ResultAndLen[2];
    uint8_t BufCRC[2];
    uint16_t DataLen;
    uint8_t flushBuf;
    bool InIdle;
    uint8_t NFCIP1_SoD[1];
    uint16_t PayloadLen;    
} st25r95UARTRxContext;

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#define ST25R95_DEBUG false

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static uint8_t EchoCommand[1] = {ST25R95_COMMAND_ECHO};

static uint32_t st25r95UARTRwdogTimer;
static bool st25r95TransmitRxCompleted = true;
static bool st25r95TransmitTxCompleted = true;
static st25r95UARTRxContext st25r95UARTRxCtx;

static uint8_t Idle[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x12, 0x21, 0x00, 0x38, 0x01, 0x18, 0x00, 0x20, 0x60, 0x60, 0x74, 0x84, 0x3F, 0x00};
static uint8_t IdleRespBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];
/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void st25r95UartFlush(void);

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode st25r95UARTSendCommandTypeAndLen(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen)
{
    ReturnCode retCode = ERR_NONE;
    uint32_t len;
    
    platformUartReset();
    if (respBuffLen < 2)
    {
        retCode = ERR_NOMEM;
    }
    else
    {
#if ST25R95_DEBUG
    platformLog("[%10d] >>>> %s\r\n", platformGetSysTick(), hex2Str(cmd, cmd[ST25R95_CMD_LENGTH_OFFSET] + 2));
#endif /* ST25R95_DEBUG */
        resp[ST25R95_CMD_RESULT_OFFSET] = ST25R95_ERRCODE_COMERROR;
        resp[ST25R95_CMD_LENGTH_OFFSET] = 0x00;
        platformUartTx(cmd, cmd[ST25R95_CMD_LENGTH_OFFSET] + 2);
        platformUartRx(resp, 2);
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
                platformUartRx(&resp[ST25R95_CMD_DATA_OFFSET], len);
            }
        }
        else
        {
            retCode = ERR_NOMEM;
        }
        if ((resp[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) && (resp[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_FRAMEOKADDITIONALINFO))
        {
            retCode = ERR_SYSTEM;
        }
        
#if ST25R95_DEBUG
        platformLog("[%10d] <<<< %s\r\n", platformGetSysTick(), hex2Str(resp, len + 2));
#endif /* ST25R95_DEBUG */
    }
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95UARTCommandEcho(void)
{
    ReturnCode retCode = ERR_NONE;
    uint8_t respBuffer[1] = {0};
    
    platformUartReset();
    platformUartTx(EchoCommand, 1);
    platformUartRx(respBuffer, 1);
    
    if (respBuffer[0] != EchoCommand[0])
    {
#if ST25R95_DEBUG  
        platformLog("%s: unexepected echo response: %2.2x\r\n", __FUNCTION__, respBuffer[0]);
#endif /* ST25R95_DEBUG */
        retCode = ERR_SYSTEM;
    }
#if ST25R95_DEBUG  
    platformLog("%s: retCode: %2.2x\r\n", __FUNCTION__, retCode);
#endif /* ST25R95_DEBUG */ 
    return (retCode);
}

/*******************************************************************************/
void st25r95UARTSendData(uint8_t *buf, uint8_t bufLen, uint8_t protocol, uint32_t flags)
{
    uint8_t CmdAndLen[2];
    uint8_t SoD[2];
    
 
#if ST25R95_DEBUG
    platformLog("[%10d] DATA >>>> %s", platformGetSysTick(), hex2Str(buf, bufLen));  
#endif /* ST25R95_DEBUG */ 
    
    CmdAndLen[ST25R95_CMD_COMMAND_OFFSET] = ST25R95_COMMAND_SENDRECV;
  
    /* add transmission Flag Len in case of 14443A */
    CmdAndLen[ST25R95_CMD_LENGTH_OFFSET] = (protocol == ST25R95_PROTOCOL_ISO14443A) ? bufLen + 1: bufLen;
    /* add SoD len in case of ISO14443A + NFCIP1 */
    CmdAndLen[ST25R95_CMD_LENGTH_OFFSET] += ((protocol == ST25R95_PROTOCOL_ISO14443A) && ((flags & RFAL_TXRX_FLAGS_NFCIP1_ON) == RFAL_TXRX_FLAGS_NFCIP1_ON)) ? 2 : 0;
    platformUartTx(CmdAndLen, 2);
    if ((protocol == ST25R95_PROTOCOL_ISO14443A) && ((flags & RFAL_TXRX_FLAGS_NFCIP1_ON) == RFAL_TXRX_FLAGS_NFCIP1_ON))
    {
        SoD[0] = 0xF0U;
        SoD[1] = bufLen + 1;
        platformUartTx(SoD, 2);
    }
    if (bufLen != 0) 
    {
        st25r95TransmitTxCompleted = false;
        platformUartTxIT(buf, bufLen);
    }
    else
    {
        st25r95TransmitTxCompleted = true;
    }
}

/*******************************************************************************/
void st25r95UARTSendTransmitFlag(uint8_t protocol, uint8_t transmitFlag)
{
    if (protocol == ST25R95_PROTOCOL_ISO14443A)
    {
#if ST25R95_DEBUG
        platformLog(" %2.2X", transmitFlag);
#endif /* ST25R95_DEBUG */       
 
        /* send transmission Flag */
        platformUartTx(&transmitFlag, 1);
    }
#if ST25R95_DEBUG
    platformLog("\r\n"); 
#endif /* ST25R95_DEBUG */ 
}

/*******************************************************************************/
void st25r95UARTPrepareRx(uint8_t protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes)
{
    platformUartReset();
    st25r95UARTRwdogTimer = platformTimerCreate(ST25R95_COMMUNICATION_UART_WDOGTIMER);
    st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_RESULT_OFFSET] = ST25R95_ERRCODE_COMERROR;
    st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_LENGTH_OFFSET] = 0x00U;
    st25r95TransmitRxCompleted           = false;
    st25r95UARTRxCtx.protocol            = protocol;
    st25r95UARTRxCtx.rxBuf               = rxBuf;
    st25r95UARTRxCtx.rxBufLen            = rxBufLen;
    st25r95UARTRxCtx.rxRcvdLen           = rxRcvdLen;
    st25r95UARTRxCtx.rmvCRC              = ((flags & RFAL_TXRX_FLAGS_CRC_RX_KEEP) != RFAL_TXRX_FLAGS_CRC_RX_KEEP);
    st25r95UARTRxCtx.NFCIP1              = ((protocol == ST25R95_PROTOCOL_ISO14443A) && ((flags & RFAL_TXRX_FLAGS_NFCIP1_ON) == RFAL_TXRX_FLAGS_NFCIP1_ON));
    st25r95UARTRxCtx.additionalRespBytes = additionalRespBytes;
    st25r95UARTRxCtx.RxState             = ST25R95_CB_RESLEN;
    st25r95UARTRxCtx.InIdle              = false;
    platformUartRxIT(st25r95UARTRxCtx.ResultAndLen, 2);
}

/*******************************************************************************/
static void st25r95UartFlush(void)
{
    if (st25r95UARTRxCtx.DataLen != 0)
    {
        st25r95UARTRxCtx.DataLen--;
        st25r95UARTRxCtx.RxState = ST25R95_CB_FLUSHUART;
        platformUartRxIT(&st25r95UARTRxCtx.flushBuf, 1);
    }
    else
    {
        st25r95TransmitRxCompleted = true;
    }           
}

/*******************************************************************************/
void st25r95UartTxCpltCallback(void)
{
    st25r95TransmitTxCompleted = true;
}

/*******************************************************************************/
/*
 * Uart Rx Callback
 * this callback is called for the various part of the ST25R95 response frame:
 * - when the Result code and the Len are received
 * - when the payload data are received
 * - optionally when the CRC is receive
 * - when the Rx flags are received
 * - optionally when the Start Byte is received
 */
void st25r95UartRxCpltCallback(void)
{
    uint8_t result;
    uint16_t len;
    rfalBitRate rxBr;
    uint16_t additionalRespBytesNb = 1;
    
    
    /* In ISO14443A 106kbps 2 additional bytes of collision information are provided */
    rfalGetBitRate( NULL, &rxBr );
    if( (st25r95UARTRxCtx.protocol == ST25R95_PROTOCOL_ISO14443A) && (rxBr == RFAL_BR_106) )
    {
        additionalRespBytesNb += 2;
    }
    
    if (st25r95UARTRxCtx.InIdle)
    {
        st25r95UARTRxCtx.InIdle = false;
        st25r95TransmitRxCompleted = true;
        return;
    }

    result = st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_RESULT_OFFSET] & 0x9FU;
    switch (st25r95UARTRxCtx.RxState)
    {
        case ST25R95_CB_RESLEN: /*  Result and Len received */
            len = st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_LENGTH_OFFSET];
            st25r95UARTRxCtx.retCode = ERR_NONE;
            
            /* compute len according to CR95HF DS § 4.4 */
            if ((result & 0x8FU) == 0x80U)
            {
                len |= (((uint16_t)st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_RESULT_OFFSET]) & 0x60U) << 3U;
            }
            st25r95UARTRxCtx.DataLen = len;
            switch (result)
            {
                case ST25R95_ERRCODE_NONE:
                case ST25R95_ERRCODE_FRAMEOKADDITIONALINFO:
                case ST25R95_ERRCODE_RESULTSRESIDUAL:
                    break;
                case ST25R95_ERRCODE_COMERROR:
                    st25r95UARTRxCtx.retCode = ERR_INTERNAL;
                    break;
                case ST25R95_ERRCODE_FRAMEWAITTIMEOUT:
                    st25r95UARTRxCtx.retCode = ERR_TIMEOUT;
                    break;
                case ST25R95_ERRCODE_OVERFLOW:
                    st25r95UARTRxCtx.retCode = ERR_HW_OVERRUN;
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
                    st25r95UARTRxCtx.retCode = ERR_FRAMING;
                    break; 
                case ST25R95_ERRCODE_62_CRC:   
                    st25r95UARTRxCtx.retCode = ERR_CRC;
                    break; 
                case ST25R95_ERRCODE_NOFIELD:
                    st25r95UARTRxCtx.retCode = ERR_LINK_LOSS;
                    break;
                default:
                    st25r95UARTRxCtx.retCode = ERR_SYSTEM;
                    break;
            }
            if (st25r95UARTRxCtx.retCode != ERR_NONE)
            {
                /* Flush data buffer in case Result is different from ST25R95_ERRCODE_NONE or ST25R95_ERRCODE_FRAMEOKADDITIONALINFO or ST25R95_ERRCODE_RESULTSRESIDUAL */
                st25r95UartFlush();
                break;
            }
            if (len == 0)
            {
                if (st25r95UARTRxCtx.rxRcvdLen != NULL)
                {
                    *(st25r95UARTRxCtx.rxRcvdLen) = 0;
                }
                st25r95TransmitRxCompleted = true;
                break;
            }
            if (len < additionalRespBytesNb)
            {
                st25r95UARTRxCtx.retCode = ERR_SYSTEM;
                st25r95UartFlush();
                break;
            }
            len -= additionalRespBytesNb;
            if ((result == ST25R95_ERRCODE_RESULTSRESIDUAL) && (st25r95UARTRxCtx.protocol == ST25R95_PROTOCOL_ISO14443A))
            {
                st25r95UARTRxCtx.rmvCRC = false;
            }
            if ((st25r95UARTRxCtx.rmvCRC) && (st25r95UARTRxCtx.protocol != ST25R95_PROTOCOL_ISO18092))
            {
                if (len < 2)
                {
                    st25r95UARTRxCtx.retCode = ERR_SYSTEM;
                    st25r95UartFlush();
                    break;
                }
                len -= 2;
            }
            if ((len > st25r95UARTRxCtx.rxBufLen) ||
                ((st25r95UARTRxCtx.protocol == ST25R95_PROTOCOL_ISO18092) && ((len + 1U) > st25r95UARTRxCtx.rxBufLen)) || /* Need one extra byte room to prepend Len byte in rxBuf in case of Felica */
                ((!st25r95UARTRxCtx.rmvCRC) && (st25r95UARTRxCtx.protocol == ST25R95_PROTOCOL_ISO18092) && ((len + 3U) > st25r95UARTRxCtx.rxBufLen))) /* same + 2 extra bytes room to append CRC */
            {
                st25r95UARTRxCtx.retCode = ERR_NOMEM;
                st25r95UartFlush();
                break;
            }     
            /* update *rxRcvdLen if not null pointer */
            if (st25r95UARTRxCtx.rxRcvdLen != NULL)
            {
                (*st25r95UARTRxCtx.rxRcvdLen) = len;
            }
            if (len != 0)
            {
                st25r95UARTRxCtx.RxState = ST25R95_CB_DATA;
                if (st25r95UARTRxCtx.protocol == ST25R95_PROTOCOL_ISO18092)
                {
                    /* Len is not part of the payload in ST25R95 ISO18092: keep room for 1 byte and read the payload */
                    platformUartRxIT(&st25r95UARTRxCtx.rxBuf[RFAL_NFCF_LENGTH_LEN], len);
                    len += RFAL_NFCF_LENGTH_LEN;
                    /* Prepend Len in the rxbuf*/
                    st25r95UARTRxCtx.rxBuf[0] = (uint8_t)(len & 0xFFU);
                    if (st25r95UARTRxCtx.rxRcvdLen != NULL)
                    {           
                        (*st25r95UARTRxCtx.rxRcvdLen) = len;
                    }
                    if (!st25r95UARTRxCtx.rmvCRC)
                    {
                        /* add a dummy CRC */
                        st25r95UARTRxCtx.rxBuf[len]     = 0x00;
                        st25r95UARTRxCtx.rxBuf[len + 1] = 0x00;
                    }
                }
                else
                {
                    if (st25r95UARTRxCtx.NFCIP1)
                    {
                        if (len >= 1)
                        {
                            /* Read SoD_SB. Keep SoD_Len as part of the payload for rfal upper layers  */
                            st25r95UARTRxCtx.RxState = ST25R95_CB_SOD;
                            len -= 1;
                            st25r95UARTRxCtx.PayloadLen = len;
                            platformUartRxIT(st25r95UARTRxCtx.NFCIP1_SoD, 1);   
                        }
                        else
                        {
                            /* Buffer too small: error */
                            st25r95UartFlush();
                            st25r95UARTRxCtx.retCode = ERR_PROTO;
                        }
                    }
                    else
                    {
                        /* Read data payload */
                        platformUartRxIT(st25r95UARTRxCtx.rxBuf, len);
                    }
                }
                break;
            }
            /* fall through */            
        case ST25R95_CB_DATA: /* data received */
            if ((st25r95UARTRxCtx.rmvCRC) && (st25r95UARTRxCtx.protocol != ST25R95_PROTOCOL_ISO18092))
            {
                /* Read the CRC*/
                st25r95UARTRxCtx.RxState = ST25R95_CB_CRC;
                platformUartRxIT(st25r95UARTRxCtx.BufCRC, 2);
                break;
            }
            if ((!st25r95UARTRxCtx.rmvCRC) && (st25r95UARTRxCtx.protocol == ST25R95_PROTOCOL_ISO18092) && ((st25r95UARTRxCtx.rxRcvdLen != NULL)))
            {
                /* increase room for CRC*/
                (*st25r95UARTRxCtx.rxRcvdLen) += 2;
            }
            /* fall through */
        case ST25R95_CB_CRC: /*  CRC received*/
            /* Read the Rx Flag additional bytes */
            st25r95UARTRxCtx.RxState = ST25R95_CB_ADDITIONALBYTES;
            platformUartRxIT(st25r95UARTRxCtx.additionalRespBytes, additionalRespBytesNb);
            break;
        case ST25R95_CB_ADDITIONALBYTES: /* Rx flags received */
            /* check collision and CRC error */
            switch (st25r95UARTRxCtx.protocol)
            {
                case (ST25R95_PROTOCOL_ISO15693):
                    st25r95UARTRxCtx.retCode = ST25R95_IS_PROT_ISO15693_COLLISION_ERR(st25r95UARTRxCtx.additionalRespBytes[0]) ? ERR_RF_COLLISION : (ST25R95_IS_PROT_ISO15693_CRC_ERR(st25r95UARTRxCtx.additionalRespBytes[0]) ? ERR_CRC : st25r95UARTRxCtx.retCode);
                    break;
                case (ST25R95_PROTOCOL_ISO14443A):
                    st25r95UARTRxCtx.retCode = (result == ST25R95_ERRCODE_RESULTSRESIDUAL) ? ((ReturnCode)(ERR_INCOMPLETE_BYTE + ((st25r95UARTRxCtx.additionalRespBytes[0] & 0xFU) % 8U))) : (ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(st25r95UARTRxCtx.additionalRespBytes[0]) ? ERR_RF_COLLISION : (ST25R95_IS_PROT_ISO14443A_PARITY_ERR(st25r95UARTRxCtx.additionalRespBytes[0]) ? ERR_PAR : (ST25R95_IS_PROT_ISO14443A_CRC_ERR(st25r95UARTRxCtx.additionalRespBytes[0]) ? ERR_CRC : st25r95UARTRxCtx.retCode)));
                    break;
                case (ST25R95_PROTOCOL_ISO14443B):
                    if (ST25R95_IS_PROT_ISO14443B_CRC_ERR(st25r95UARTRxCtx.additionalRespBytes[0]))
                    {
                        st25r95UARTRxCtx.retCode = ERR_CRC;
                    }
                    break;
                case (ST25R95_PROTOCOL_ISO18092):
                    if (ST25R95_IS_PROT_ISO18092_CRC_ERR(st25r95UARTRxCtx.additionalRespBytes[0]))
                    {
                        st25r95UARTRxCtx.retCode = ERR_CRC;
                    }
                    break;                        
                default:
                    break;
            }
            st25r95TransmitRxCompleted = true;
            break;
        case ST25R95_CB_FLUSHUART: /* Flush uart Rx ongoing */
            /* Continue flushing until Len = 0*/
            st25r95UartFlush();
            break;   
        case ST25R95_CB_SOD: /* SoD_SB received */
            st25r95UARTRxCtx.RxState = ST25R95_CB_DATA;
            platformUartRxIT(st25r95UARTRxCtx.rxBuf, st25r95UARTRxCtx.PayloadLen);
            break;
        default:
            st25r95UARTRxCtx.retCode = ERR_SYSTEM;
            st25r95TransmitRxCompleted = true;
            break;
    }            

}

/*******************************************************************************/
void st25r95UartErrorCallback(void)
{
    st25r95UARTRxCtx.retCode = ERR_SYSTEM;
    st25r95TransmitRxCompleted = true;
    st25r95TransmitTxCompleted = true;
}

/*******************************************************************************/
ReturnCode st25r95UARTCompleteRx(void)
{
#if ST25R95_DEBUG
    platformLog("[%10d] DATA <<<<(0x%2.2X%2.2X) ", platformGetSysTick(), st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_RESULT_OFFSET], st25r95UARTRxCtx.ResultAndLen[ST25R95_CMD_LENGTH_OFFSET]);
    if ((st25r95UARTRxCtx.rxRcvdLen != NULL) && ((*st25r95UARTRxCtx.rxRcvdLen) != 0))
    {
        platformLog("%s", hex2Str(st25r95UARTRxCtx.rxBuf, (*st25r95UARTRxCtx.rxRcvdLen)));
        if ((st25r95UARTRxCtx.rmvCRC) && (st25r95UARTRxCtx.protocol != ST25R95_PROTOCOL_ISO18092))
        {
            platformLog("[%s]", hex2Str(st25r95UARTRxCtx.BufCRC, 2));
        }
        platformLog(" %s", hex2Str(st25r95UARTRxCtx.additionalRespBytes, 1));
    }
    platformLog(" (retCode=%d)\r\n", st25r95UARTRxCtx.retCode);
#endif /* ST25R95_DEBUG */
    return (st25r95UARTRxCtx.retCode);
}

/*******************************************************************************/
bool st25r95UARTIsTransmitTxCompleted(void)
{
    if ((st25r95TransmitTxCompleted == false) && platformTimerIsExpired(st25r95UARTRwdogTimer))
    {
        st25r95TransmitTxCompleted = true;
        st25r95TransmitRxCompleted = true;
        st25r95UARTRxCtx.retCode = ERR_TIMEOUT;
#if ST25R95_DEBUG
        platformLog("UART Transmit Tx timeout: tranmit aborted\r\n");
#endif /* ST25R95_DEBUG */        
    }
    return (st25r95TransmitTxCompleted);
}

/*******************************************************************************/
ReturnCode st25r95UARTPollRead(uint32_t timeout)
{
    NO_WARNING(timeout);
    
    if ((st25r95TransmitRxCompleted == false) && !st25r95UARTRxCtx.InIdle && platformTimerIsExpired(st25r95UARTRwdogTimer))
    {
        st25r95TransmitTxCompleted = true;
        st25r95TransmitRxCompleted = true;
        st25r95UARTRxCtx.retCode = ERR_TIMEOUT;
#if ST25R95_DEBUG
        platformLog("UART Transmit Rx timeout: tranmit aborted\r\n");
#endif /* ST25R95_DEBUG */        
    }
    return ((st25r95TransmitRxCompleted == false) ? (ERR_TIMEOUT) : ERR_NONE);
}

/*******************************************************************************/
void st25r95UARTIdle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod)
{
    Idle[ST25R95_IDLE_WUPERIOD_OFFSET] = WUPeriod;
    Idle[ST25R95_IDLE_DACDATAL_OFFSET] = dacDataL;
    Idle[ST25R95_IDLE_DACDATAH_OFFSET] = dacDataH;
    
    IdleRespBuffer[ST25R95_CMD_RESULT_OFFSET] = ST25R95_ERRCODE_COMERROR;
    IdleRespBuffer[ST25R95_CMD_LENGTH_OFFSET] = 0x00;
    st25r95UARTRxCtx.InIdle    = true;
    st25r95TransmitRxCompleted = false;
    platformUartTx(Idle, Idle[ST25R95_CMD_LENGTH_OFFSET] + 2);
    platformUartRxIT(IdleRespBuffer, 3);
    
    #if ST25R95_DEBUG
    platformLog("[%10d] >>>> %s\r\n", platformGetSysTick(), hex2Str(Idle, Idle[ST25R95_CMD_LENGTH_OFFSET] + 2));
    #endif /* ST25R95_DEBUG */
}

/*******************************************************************************/
void st25r95UARTGetIdleResponse(void)
{
    #if ST25R95_DEBUG
    platformLog("[%10d] <<<< %s\r\n", platformGetSysTick(), hex2Str(IdleRespBuffer, IdleRespBuffer[ST25R95_CMD_LENGTH_OFFSET] + 2));
    #endif /* ST25R95_DEBUG */
}

/*******************************************************************************/
void st25r95UARTKillIdle(void)
{
    uint32_t attempt = 50;

    platformGpioClear(ST25R95_N_SS_PORT, ST25R95_N_SS_PIN);
    platformDelay(1);
    platformGpioSet(ST25R95_N_SS_PORT, ST25R95_N_SS_PIN);
    
    while ((st25r95PollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ERR_TIMEOUT) && (attempt != 0))
    {
        platformDelay(1);
        attempt--;
    }
    st25r95UARTGetIdleResponse();
}
#endif /* ST25R95_INTERFACE_UART */

