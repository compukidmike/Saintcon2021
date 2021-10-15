
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

/*! \file st25r95_com.h
 *
 *  \author
 *
 *  \brief ST25R95 communication declaration file
 *
 */
/*!
 * This driver provides basic abstraction for communication with the ST25R95.
 * It uses the SPI driver for interfacing with the ST25R95.
 *
 * API:
 * 
 *
 * @addtogroup RFAL
 * @{
 *
 * @addtogroup RFAL-HAL
 * @brief RFAL Hardware Abstraction Layer
 * @{
 *
 * @addtogroup ST25R95
 * @brief RFAL ST25R95 Driver
 * @{
 * 
 * @addtogroup ST25R95_Com
 * @brief RFAL ST25R95 Communication
 * @{
 * 
 */

#ifndef ST25R95_COM_H
#define ST25R95_COM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "st_errno.h"
#include "rfal_rf.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
/* See ST95HF DS §4.1.1 or CR95HF DS §4.2.1 */
#define ST25R95_CONTROL_SEND                             0x00 /*!< Send command to the ST25R95 */
#define ST25R95_CONTROL_RESET                            0x01 /*!< Reset the ST25R95           */
#define ST25R95_CONTROL_READ                             0x02 /*!< Read data from the ST25R95  */
#define ST25R95_CONTROL_POLL                             0x03 /*!< Poll the ST25R95            */

#define ST25R95_CONTROL_POLL_TIMEOUT                      100 /*!< Polling timeout             */
#define ST25R95_CONTROL_POLL_NO_TIMEOUT                     0 /*!< non blocking polling        */

/* See ST95HF DS §5.2 or CR95HF DS §5.2 */
#define ST25R95_COMMAND_IDN                              0x01 /*!< Requests short information about the ST25R95 and its revision.                           */
#define ST25R95_COMMAND_PROTOCOLSELECT                   0x02 /*!< Selects the RF communication protocol and specifies certain protocol-related parameters. */
#define ST25R95_COMMAND_POLLFIED                         0x03 /*!< Returns the current value of the FieldDet flag (used in Card Emulation mode).            */
#define ST25R95_COMMAND_SENDRECV                         0x04 /*!< Sends data using the previously selected protocol and receives the tag response.         */
#define ST25R95_COMMAND_LISTEN                           0x05 /*!< Listens for data using previously selected protocol (used in Card Emulation mode).       */
#define ST25R95_COMMAND_SEND                             0x06 /*!< Sends data using previously selected protocol (used in Card Emulation mode).             */
#define ST25R95_COMMAND_IDLE                             0x07 /*!< Switches the ST25R95 into a low consumption                                              */
#define ST25R95_COMMAND_RDREG                            0x08 /*!< Reads Wake-up event register or the Analog Register Configuration (ARC_B) register       */
#define ST25R95_COMMAND_WRREG                            0x09 /*!< Write register                                                                           */
#define ST25R95_COMMAND_BAUDRATE                         0x0A /*!< Sets the UART baud rate.                                                                 */
#define ST25R95_COMMAND_ACFILTER                         0x0D /*!< Enables or disables the anti-collision filter for ISO/IEC 14443 Type A protocol.         */
#define ST25R95_COMMAND_ECHO                             0x55 /*!< ST25R95 performs a serial interface ECHO command                                         */

#define ST25R95_SPI_DUMMY_BYTE                           0x00 /*!< Dummy byte when nothing to transmit on SPI                                               */

#define ST25R95_CMD_COMMAND_OFFSET                       0x00U /*!< CMD Offset. See CR95HF DS § 4.2.1                        */
#define ST25R95_CMD_RESULT_OFFSET                        0x00U /*!< Resp Code Offset. See CR95HF DS § 4.2.1                  */
#define ST25R95_CMD_LENGTH_OFFSET                        0x01U /*!< LEN Offset. See CR95HF DS § 4.2.1                        */
#define ST25R95_CMD_DATA_OFFSET                          0x02U /*!< DATA[0] Offset. See CR95HF DS § 4.2.1                    */

#define ST25R95_COMMUNICATION_BUFFER_SIZE                (528 + 2) /*!< Max received buffer size                            */
#define ST25R95_COMMUNICATION_UART_WDOGTIMER             (5000U)   /*!< Uart watchdog timer                                 */

#define ST25R95_PROTOCOL_FIELDOFF                        0x00U /*!< ProtocolSelect protocol code: Field OFF                  */
#define ST25R95_PROTOCOL_ISO15693                        0x01U /*!< ProtocolSelect protocol code: ISO15693  (Reader)         */
#define ST25R95_PROTOCOL_ISO14443A                       0x02U /*!< ProtocolSelect protocol code: ISO14443A (Reader)         */
#define ST25R95_PROTOCOL_ISO14443B                       0x03U /*!< ProtocolSelect protocol code: ISO14443B (Reader)         */
#define ST25R95_PROTOCOL_ISO18092                        0x04U /*!< ProtocolSelect protocol code: ISO18092  (Reader)         */
#define ST25R95_PROTOCOL_CE_ISO14443A                    0x05U /*!< ProtocolSelect protocol code: ISO14443  (Card Emulation) */

#define ST25R95_PROTOCOL_MAX                             ST25R95_PROTOCOL_CE_ISO14443A /*!< Max value of protocol field */

#define ST25R95_FWT_MAX                                  0x40A8BC0 /*!< Max FWT supported: 5s */

#define ST25R95_ERRCODE_NONE                             0x00 /*!< no error occurred */
#define ST25R95_ERRCODE_FRAMEOKADDITIONALINFO            0x80 /*!< Frame correctly received (additionally see CRC/Parity information) */
#define ST25R95_ERRCODE_INVALIDCCMDCODE                  0x81 /*!< Invalid command code */
#define ST25R95_ERRCODE_INVALIDCMDLENGHT                 0x82 /*!< Invalid command length */
#define ST25R95_ERRCODE_INVALIDPROTOCOL                  0x83 /*!< Invalid protocol */
#define ST25R95_ERRCODE_COMERROR                         0x86 /*!< Hardware communication error */
#define ST25R95_ERRCODE_FRAMEWAITTIMEOUT                 0x87 /*!< Frame wait time out (no valid reception) */
#define ST25R95_ERRCODE_INVALIDSOF                       0x88 /*!< Invalid SOF */
#define ST25R95_ERRCODE_OVERFLOW                         0x89 /*!< Too many bytes received and data still arriving */
#define ST25R95_ERRCODE_FRAMING                          0x8A /*!< if start bit = 1 or stop bit = 0 */
#define ST25R95_ERRCODE_EGT                              0x8B /*!< EGT time out */
#define ST25R95_ERRCODE_FIELDLENGTH                      0x8C /*!< Valid for ISO/IEC 18092, if Length <3 */
#define ST25R95_ERRCODE_CRC                              0x8D /*!< CRC error, Valid only for ISO/IEC 18092 */
#define ST25R95_ERRCODE_RECEPTIONLOST                    0x8E /*!< When reception is lost without EOF received (or subcarrier was lost) */
#define ST25R95_ERRCODE_NOFIELD                          0x8F /*!< When Listen command detects the absence of external field */
#define ST25R95_ERRCODE_RESULTSRESIDUAL                  0x90 /*!< Residual bits in last byte. Useful for ACK/NAK reception of ISO/IEC 14443 Type A. */
#define ST25R95_ERRCODE_61_SOF                           0x61 /*!< SOF error during the EMD process */
#define ST25R95_ERRCODE_62_CRC                           0x62 /*!< CRC error during the EMD process */
#define ST25R95_ERRCODE_63_SOF_HIGH                      0x63 /*!< SOF error in ISO14443B occurs during high part (duration of 2 to 3 Elementary Unit Time, ETU) */
#define ST25R95_ERRCODE_65_SOF_LOW                       0x65 /*!< SOF error in ISO14443B occurs during low part (duration of 10 to 11 Elementary Unit Time, ETU) */
#define ST25R95_ERRCODE_66_EGT                           0x66 /*!< Extra Guard Time (EGT) error in ISO14443B */
#define ST25R95_ERRCODE_67_TR1TOOLONG                    0x67 /*!< TR1 set by card too long in case of protocol ISO14443B */
#define ST25R95_ERRCODE_68_TR1TOOSHORT                   0x68 /*!< TTR1 set by card too short in case of protocol ISO14443B */

#define ST25R95_REG_ARC_B                                0x6801 /*!< ARC_B register address */
#define ST25R95_REG_ACC_A                                0x6804 /*!< ACC_A register address */
#define ST25R95_REG_TIMERW                               0x3A00 /*!< TIMER register address */

#define ST25R95_IDN_RESPONSE_BUFLEN                              (15 + 2) /*!< IDN response buffer len */
#define ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN                   (0  + 2) /*!< ProtocolSelect response buffer len */ 
#define ST25R95_POLLFIELD_RESPONSE_BUFLEN                        (1  + 2) /*!< PollField response buffer len */
#define ST25R95_LISTEN_RESPONSE_BUFLEN                           (0  + 2) /*!< Listen response buffer len */
#define ST25R95_RDREG_RESPONSE_BUFLEN                            (1  + 2) /*!< ReadReg response buffer len */
#define ST25R95_WRREG_RESPONSE_BUFLEN                            (0  + 2) /*!< WriteRead response buffer len */
#define ST25R95_ACFILTER_RESPONSE_BUFLEN                         (1  + 2) /*!< ACFilter response buffer len */
#define ST25R95_IDLE_RESPONSE_BUFLEN                             (1  + 2) /*!< Idle response buffer len */
#define ST25R95_ECHO_RESPONSE_BUFLEN                             (3)      /*!< Echo response buffer len */
#define ST25R95_SEND_RESPONSE_BUFLEN                             (0  + 2) /*!< Send response buffer len */

#define ST25R95_ACSTATE_IDLE                             0x00U /*!< AC Filter state: Idle */
#define ST25R95_ACSTATE_READYA                           0x01U /*!< AC Filter state: ReadyA */
#define ST25R95_ACSTATE_ACTIVE                           0x04U /*!< AC Filter state: Active */
#define ST25R95_ACSTATE_HALT                             0x80U /*!< AC Filter state: Halt */
#define ST25R95_ACSTATE_READYAX                          0x81U /*!< AC Filter state: ReadyA* */
#define ST25R95_ACSTATE_ACTIVEX                          0x84U /*!< AC Filter state: Active* */

#define ST25R95_IS_PROT_ISO15693_CRC_ERR(status)        (((status) & 0x02U) == 0x02U) /*!< Test for CRC flag in SendRcv response additional byte for ISO15693 protocol */
#define ST25R95_IS_PROT_ISO15693_COLLISION_ERR(status)  (((status) & 0x01U) == 0x01U) /*!< Test for Collision flag in SendRcv response additional byte for ISO15693 protocol*/

#define ST25R95_IS_PROT_ISO14443A_COLLISION_ERR(status) (((status) & 0x80U) == 0x80U) /*!< Test for Colission flag in SendRcv response additional byte for ISO14443A protocol */
#define ST25R95_IS_PROT_ISO14443A_CRC_ERR(status)       (((status) & 0x20U) == 0x20U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO14443A protocol */
#define ST25R95_IS_PROT_ISO14443A_PARITY_ERR(status)    (((status) & 0x10U) == 0x10U) /*!< Test for Parity    flag in SendRcv response additional byte for ISO14443A protocol */

#define ST25R95_IS_PROT_ISO14443B_CRC_ERR(status)       (((status) & 0x02U) == 0x02U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO14443B protocol */

#define ST25R95_IS_PROT_ISO18092_CRC_ERR(status)        (((status) & 0x02U) == 0x02U) /*!< Test for CRC       flag in SendRcv response additional byte for ISO18092  protocol */

#define ST25R95_PROTOCOLSELECT_BR_OFFSET                (3U) /*!< Bit Rate offset in ProtocolSelect Command */

#define ST25R95_IDLE_WUPERIOD_OFFSET                    (0x09U) /*!< WUPeriod offset in Idle Command  */
#define ST25R95_IDLE_DACDATAL_OFFSET                    (0x0CU) /*!< DacDataL offset in Idle Command  */
#define ST25R95_IDLE_DACDATAH_OFFSET                    (0x0DU) /*!< DacDataH offset in Idle Command  */
#define ST25R95_DACDATA_MAX                             (0xFCU) /*!< DacData max value (6 bits MSB)   */
#define ST25R95_IDLE_WKUP_TIMEOUT                       (0x01U) /*!< Idle wakeup source: timeout      */
#define ST25R95_IDLE_WKUP_TAGDETECT                     (0x02U) /*!< Idle wakeup source: Tag Detected */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#if !(ST25R95_INTERFACE_UART) /* ST25R95_INTERFACE_SPI */ 
#define st25r95SendCommandTypeAndLen(cmd, resp, respBuffLen)                                st25r95SPISendCommandTypeAndLen((cmd), (resp), (respBuffLen))                                         /*!< UART/SPI wrapper for st25r95SendCommandTypeAndLen */
#define st25r95CommandEcho()                                                                st25r95SPICommandEcho()                                                                               /*!< UART/SPI wrapper for st25r95CommandEcho           */
#define st25r95SendData(buf, bufLen, protocol, flags)                                       st25r95SPISendData((buf), (bufLen), (protocol), (flags))                                              /*!< UART/SPI wrapper for st25r95SendData              */
#define st25r95SendTransmitFlag(protocol, transmitFlag)                                     st25r95SPISendTransmitFlag((protocol), (transmitFlag))                                                /*!< UART/SPI wrapper for st25r95SendTransmitFlag      */
#define st25r95PollRead(timeout)                                                            st25r95SPIPollRead((timeout))                                                                         /*!< UART/SPI wrapper for st25r95PollRead              */
#define st25r95PrepareRx(protocol, rxBuf, rxBufLen, rxRcvdLen, flags, additionalRespBytes)  st25r95SPIPrepareRx((protocol), (rxBuf), (rxBufLen), (rxRcvdLen),  (flags), (additionalRespBytes));   /*!< UART/SPI wrapper for st25r95PrepareRx             */
#define st25r95CompleteRx()                                                                 st25r95SPICompleteRx();                                                                               /*!< UART/SPI wrapper for st25r95CompleteRx            */
#define st25r95IsTransmitTxCompleted()                                                      st25r95SPIIsTransmitCompleted()                                                                       /*!< UART/SPI wrapper for st25r95IsTransmitTxCompleted */
#define st25r95IsInListen()                                                                 st25r95SPIIsInListen()                                                                                /*!< UART/SPI wrapper for st25r95IsInListen            */
#define st25r95GetLmState()                                                                 st25r95SPIGetLmState()                                                                                /*!< UART/SPI wrapper for st25r95GetLmState            */
#define st25r95DeactivateACFilter()                                                         st25r95SPIDeactivateACFilter()                                                                        /*!< UART/SPI wrapper for st25r95DeactivateACFilter    */
#define st25r95SetACState(State)                                                            st25r95SPISetACState(State)                                                                           /*!< UART/SPI wrapper for st25r95SetACState            */
#define st25r95Idle(DacDataL, DacDataH, WUPeriod)                                           st25r95SPIIdle((DacDataL), (DacDataH), (WUPeriod))                                                    /*!< UART/SPI wrapper for st25r95Idle                  */
#define st25r95GetIdleResponse()                                                            st25r95SPIGetIdleResponse()                                                                           /*!< UART/SPI wrapper for st25r95GetIdleResponse       */
#define st25r95KillIdle()                                                                   st25r95SPIKillIdle()                                                                                  /*!< UART/SPI wrapper for st25r95KillIdle              */
#define st25r95FlushChipSPIBuffer()                                                         st25r95SPIRxTx(NULL, NULL, ST25R95_COMMUNICATION_BUFFER_SIZE)                                         /*!< st25r95FlushChipSPIBuffer defined as a macro for better readibility */
#else /* !ST25R95_INTERFACE_SPI */
#define st25r95SendCommandTypeAndLen(cmd, resp, respBuffLen)                                st25r95UARTSendCommandTypeAndLen((cmd), (resp), (respBuffLen))                                        /*!< UART/SPI wrapper for st25r95SendCommandTypeAndLen */
#define st25r95CommandEcho()                                                                st25r95UARTCommandEcho()                                                                              /*!< UART/SPI wrapper for st25r95CommandEcho           */
#define st25r95SendData(buf, bufLen, protocol, flags)                                       st25r95UARTSendData((buf), (bufLen), (protocol), (flags))                                             /*!< UART/SPI wrapper for st25r95SendData              */
#define st25r95SendTransmitFlag(protocol, transmitFlag)                                     st25r95UARTSendTransmitFlag((protocol), (transmitFlag))                                               /*!< UART/SPI wrapper for st25r95SendTransmitFlag      */
#define st25r95PollRead(timeout)                                                            st25r95UARTPollRead((timeout))                                                                        /*!< UART/SPI wrapper for st25r95PollRead              */
#define st25r95PrepareRx(protocol, rxBuf, rxBufLen, rxRcvdLen, flags, additionalRespBytes)  st25r95UARTPrepareRx((protocol), (rxBuf), (rxBufLen), (rxRcvdLen),  (flags), (additionalRespBytes));  /*!< UART/SPI wrapper for st25r95PrepareRx             */
#define st25r95CompleteRx()                                                                 st25r95UARTCompleteRx();                                                                              /*!< UART/SPI wrapper for st25r95CompleteRx            */
#define st25r95IsTransmitTxCompleted()                                                      st25r95UARTIsTransmitTxCompleted()                                                                    /*!< UART/SPI wrapper for st25r95IsTransmitTxCompleted */
#define st25r95IsInListen()                                                                                                                                                                       /*!< UART/SPI wrapper for st25r95IsInListen            */
#define st25r95GetLmState()                                                                                                                                                                       /*!< UART/SPI wrapper for st25r95GetLmState            */
#define st25r95DeactivateACFilter()                                                         ERR_NOT_IMPLEMENTED                                                                                   /*!< UART/SPI wrapper for st25r95DeactivateACFilter    */
#define st25r95SetACState()                                                                                                                                                                       /*!< UART/SPI wrapper for st25r95SetACState            */
#define st25r95Idle(DacDataL, DacDataH, WUPeriod)                                           st25r95UARTIdle((DacDataL), (DacDataH), (WUPeriod))                                                   /*!< UART/SPI wrapper for st25r95Idle                  */
#define st25r95GetIdleResponse()                                                            st25r95UARTGetIdleResponse()                                                                          /*!< UART/SPI wrapper for st25r95GetIdleResponse       */
#define st25r95KillIdle()                                                                   st25r95UARTKillIdle()                                                                                 /*!< UART/SPI wrapper for st25r95KillIdle              */
#endif /* ST25R95_INTERFACE_SPI */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

#if !(ST25R95_INTERFACE_UART) /* ST25R95_INTERFACE_SPI */ 
/*! SPI transceive context definition */
typedef struct
{
    bool rmvCRC;                         /*!< Remove CRC flag                 */
    bool inListen;                       /*!< inListen flags                  */
    bool NFCIP1;                         /*!< NFCIP1 flags                    */
    rfalLmState LmState;                 /*!< LmState                         */
    uint16_t rxBufLen;                   /*!< rxBufLen                        */
    uint16_t *rxRcvdLen;                 /*!< rxRcvdLen                       */
    uint8_t *rxBuf;                      /*!< rxBuf                           */
    uint8_t *additionalRespBytes;        /*!< additionalRespBytes             */
    ReturnCode retCode;                  /*!< retCode                         */
    uint8_t BufCRC[2];                   /*!< BufCRC                          */
    uint8_t NFCIP1_SoD[1];               /*!< NFCIP1_SoD                      */
    uint8_t protocol;                    /*!< protocol                        */

} st25r95SPIRxContext;

extern st25r95SPIRxContext st25r95SPIRxCtx; /*!< Context for SPI transceive     */
#endif /* ST25R95_INTERFACE_SPI */

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 *  \brief  Resets the ST25R95 (SPI interface)
 *
 *  This function is used to reset the ST25R95 as per CR95HF DS § 4.2.1 .
 *
 *
 *****************************************************************************
 */
extern void st25r95SPIResetChip(void);

/*! 
 *****************************************************************************
 *  \brief  Sends a command to ST25R95 and reads the response (SPI Interface)
 *
 *  This function is used to send a \a cmd and read the \a resp. The \a cmd command
 *  must follow the CMD LEN format (i.e. any command except echo)
 *
 *  \param[in]   cmd: Command as per CR95HF DS §5.
 *  \param[out]  resp: buffer for the response.
 *  \param[in]   respBuffLen: response buffer length
 *
 *  \return ERR_NONE    : Operation successful
 *  \return ERR_TIMEOUT : Timeout
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPISendCommandTypeAndLen(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen);

/*! 
 *****************************************************************************
 *  \brief  Sends a command to ST25R95 and reads the response (UART Interface)
 *
 *  This function is used to send a \a cmd and read the \a resp. The \a cmd command
 *  must follow the CMD LEN format (i.e. any command except echo)
 *
 *  \param[in]   cmd: Command as per CR95HF DS §5.
 *  \param[out]  resp: buffer for the response.
 *  \param[in]   respBuffLen: response buffer length
 *
 *  \return ERR_NONE    : Operation successful
 *  \return ERR_TIMEOUT : Timeout
 *
 *****************************************************************************
 */
extern ReturnCode st25r95UARTSendCommandTypeAndLen(uint8_t *cmd, uint8_t *resp, uint16_t respBuffLen);

/*! 
 *****************************************************************************
 *  \brief  Sends a echo command to ST25R95 (SPI Interface)
 *
 *  This function is used to send an echo command
 *
 *  \return ERR_NONE    : Operation successful (echo response received)
 *  \return ERR_TIMEOUT : Timeout
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPICommandEcho(void);

/*! 
 *****************************************************************************
 *  \brief  Sends a echo command to ST25R95 (UART Interface)
 *
 *  This function is used to send an echo command
 *
 *  \return ERR_NONE    : Operation successful (echo response received)
 *  \return ERR_TIMEOUT : Timeout
 *
 *****************************************************************************
 */
extern ReturnCode st25r95UARTCommandEcho(void);

/*! 
 *****************************************************************************
 *  \brief  Sends one byte over SPI and returns the byte received
 *
 *  This function is used to send one byte over SPI and  returns the byte received
 *
 *  \param[in]   data: byte to be sent over SPI
 *
 *  \return ERR_NONE    : Operation successful (echo response received)
 *  \return ERR_TIMEOUT : Timeout
 *
 *****************************************************************************
 */
extern uint8_t st25r95SPISendReceiveByte(uint8_t data);

/*! 
 *****************************************************************************
 *  \brief  Sends ProtocolSelect command
 *
 *  This function is used to send ST25R95 ProtocolSelect command
 *
 *  \param[in]   protocol: value of the protocol
 *
 *  \return ERR_NONE    : Operation successful
 *  \return ERR_TIMEOUT : Timeout
 *  \return ERR_PARAM   : ProtocolSelect failure
 *
 *****************************************************************************
 */
extern ReturnCode st25r95ProtocolSelect(uint8_t protocol);

/*! 
 *****************************************************************************
 *  \brief  Set bit rates
 *
 *  This function is used to set Tx and Rx bit rates
 *
 *  \param[in]   protocol: value of the protocol
 *  \param[in]   txBR: tx bit rate
 *  \param[in]   rxBR: rx bit rate
 *
 *  \return ERR_NONE            : Operation successful
 *  \return ERR_NOT_IMPLEMENTED : unsupported bit rate value
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SetBitRate(uint8_t protocol, rfalBitRate txBR, rfalBitRate rxBR);

/*! 
 *****************************************************************************
 *  \brief  Sends data (SPI interface)
 *
 *  This function is used to send data
 *
 *  \param[in]   buf     : data to be sent
 *  \param[in]   bufLen  : buffer length
 *  \param[in]   protocol: protocol index
 *  \param[in]   flags   : TransceiveFlags indication special handling
 *
 *
 *****************************************************************************
 */
extern void st25r95SPISendData(uint8_t *buf, uint8_t bufLen, uint8_t protocol, uint32_t flags);

/*! 
 *****************************************************************************
 *  \brief  Sends data (UART interface)
 *
 *  This function is used to send data
 *
 *  \param[in]   buf     : data to be sent
 *  \param[in]   bufLen  : buffer length
 *  \param[in]   protocol: protocol index
 *  \param[in]   flags   : TransceiveFlags indication special handling
 *
 *
 *****************************************************************************
 */
extern void st25r95UARTSendData(uint8_t *buf, uint8_t bufLen, uint8_t protocol, uint32_t flags);
/*! 
 *****************************************************************************
 *  \brief  Prepares Rx (SPI Interface)
 *
 *  This function is used to send data
 *  
 *   \warning This method should be called only if polling is successful
 *
 *  \param[in]   protocol           : protocol index
 *  \param[in]   rxBuf              : buffer to place the response
 *  \param[in]   rxBufLen           : length of rxBuf
 *  \param[in]   rxRcvdLen          : received length
 *  \param[in]   flags              : TransceiveFlags indication special handling
 *  \param[in]   additionalRespBytes: additionnal response bytes
 *
 *****************************************************************************
 */
extern void st25r95SPIPrepareRx(uint8_t protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes);

/*! 
 *****************************************************************************
 *  \brief  Prepares Rx (UART Interface)
 *
 *  This function is used to send data
 *
 *  \param[in]   protocol           : protocol index
 *  \param[in]   rxBuf              : buffer to place the response
 *  \param[in]   rxBufLen           : length of rxBuf
 *  \param[in]   rxRcvdLen          : received length
 *  \param[in]   flags              : TransceiveFlags indication special handling
 *  \param[in]   additionalRespBytes: additionnal response bytes
 *
 *****************************************************************************
 */
extern void st25r95UARTPrepareRx(uint8_t protocol, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxRcvdLen, uint32_t flags, uint8_t *additionalRespBytes);

/*! 
 *****************************************************************************
 *  \brief  Polls the ST25R95 for incomming data (SPI Interface)
 *
 *  This function is used to poll the ST25R95 for incomming data
 *  
 *  \param[in]   timeout: timeout value
 *
 *  \return ERR_NONE    : Data ready to be read
 *  \return ERR_TIMEOUT : No data available
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPIPollRead(uint32_t timeout);

/*! 
 *****************************************************************************
 *  \brief  Polls the ST25R95 for incomming data (UART Interface)
 *
 *  This function is used to poll the ST25R95 for incomming data
 * 
 *  \param[in]   timeout: timeout value
 * 
 *  \return ERR_NONE    : Data ready to be read
 *  \return ERR_TIMEOUT : No data available
 *
 *****************************************************************************
 */
extern ReturnCode st25r95UARTPollRead(uint32_t timeout);

/*! 
 *****************************************************************************
 *  \brief  Polls the ST25R95 for sending (SPI Interface)
 *
 *  This function is used to poll the ST25R95 before sending data
 *  
 *  \return ERR_NONE    : ST25R95 ready to receive data
 *  \return ERR_TIMEOUT : ST25R95 not ready to receive data
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPIPollSend(void);

/*! 
 *****************************************************************************
 *  \brief  set FWT
 *
 *  This function is used to set the FWT for the next frames
 *
 *  \param[in]   protocol: protocol index
 *  \param[in]   fwt: frame waiting time
 * 
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SetFWT(uint8_t protocol, uint32_t fwt);

/*! 
 *****************************************************************************
 *  \brief  set slot counter
 *
 *  This function is used to set the slot counter (ISO18092)
 *
 *  \param[in]   slots: number of slots
 * 
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SetSlotCounter(uint8_t slots);

/*! 
 *****************************************************************************
 *  \brief  Write Register
 *
 *  This function is used to write ST25R95 ARC_B register
 *
 *  \param[in]   protocol: protocol index
 *  \param[in]   reg: register (currently only ST25R95_REG_ARC_B register is defined)
 *  \param[in]   value: register value 
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95WriteReg(uint8_t protocol, uint16_t reg, uint8_t value);

/*! 
 *****************************************************************************
 *  \brief  Read Register
 *
 *  This function is used to read ST25R95 ARC_B register
 *
 *  \param[in]   reg: register (currently only ST25R95_REG_ARC_B register is defined)
 *  \param[out]  value: register value
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95ReadReg(uint16_t reg, uint8_t *value);

/*! 
 *****************************************************************************
 *  \brief  Poll field (CE mode)
 *
 *  This function is used to poll field. Only available in CE mode.
 *
 *  \return false   : field OFF
 *  \return true    : field ON
 *
 *****************************************************************************
 */
extern bool st25r95PollField(void);

/*! 
 *****************************************************************************
 *  \brief  Set Anti-Collision filter (CE mode)
 *
 *  This function is used to program the AC filter. Only available in CE mode.
 *
 *  \param[in]   confA: pointer to Passive A configurations
 * 
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SetACFilter(const rfalLmConfPA *confA);

/*! 
 *****************************************************************************
 *  \brief  Listen (CE mode)
 *
 *  This function starts the receive in CE mode.
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95Listen(void);

/*! 
 *****************************************************************************
 *  \brief  Uart Tx Callback (UART mode)
 *
 *  Uart Tx interrupt handler callback.
 *
 *****************************************************************************
 */
extern void st25r95UartTxCpltCallback(void);

/*! 
 *****************************************************************************
 *  \brief  Uart Rx Callback (UART mode)
 *
 *  Uart Rx interrupt handler callback.
 *
 *****************************************************************************
 */
extern void st25r95UartRxCpltCallback(void);

/*! 
 *****************************************************************************
 *  \brief  Uart Error Callback (UART mode)
 *
 *  Uart transceive error interrupt handler callback.
 *
 *****************************************************************************
 */
extern void st25r95UartErrorCallback(void);

/*! 
 *****************************************************************************
 *  \brief End of transmit test function (SPI mode)
 *
 *  This function is used to test whether the current transmit is finished or not.
 *
 *  \return true    : Transmit completed
 *  \return false   : Transmit not completed
 *
 *****************************************************************************
 */
extern bool st25r95SPIIsTransmitCompleted(void);

/*! 
 *****************************************************************************
 *  \brief End of transmit test function (UART mode)
 *
 *  This function is used to test whether the current transmit is finished or not.
 *
 *  \return true    : Transmit completed
 *  \return false   : Transmit not completed
 *
 *****************************************************************************
 */
extern bool st25r95UARTIsTransmitTxCompleted(void);

/*! 
 *****************************************************************************
 *  \brief Listen mode test function (CE mode)
 *
 *  This function is used to test whether the ST25R95 is currently in Listen mode.
 *
 *  \return true    : Listen mode on going
 *  \return false   : Not in Listen mode
 *
 *****************************************************************************
 */
extern bool st25r95SPIIsInListen(void);

/*! 
 *****************************************************************************
 *  \brief Get Listen Mode state (CE mode)
 *
 *  This function returns the Listen mode state.
 *
 * \return rfalLmState Any : LM State
 *
 *****************************************************************************
 */
extern rfalLmState st25r95SPIGetLmState(void);

/*! 
 *****************************************************************************
 *  \brief  Deactivate the Anti-Collision filter (CE mode)
 *
 *  This function is used to deactivate the AC filter. Only available in CE mode.
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPIDeactivateACFilter(void);

/*! 
 *****************************************************************************
 *  \brief  Set Anti-Collision filter state (CE mode)
 *
 *  This function is used to set the AC filter state. Only available in CE mode.
 *
 *  \param[in]   state: AC state
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPISetACState(uint8_t state);

/*! 
 *****************************************************************************
 *  \brief  Rearm Listen mode (CE mode)
 *
 *  This function is used to rearm the listen mode. Only available in CE mode.
 *
 *****************************************************************************
 */
extern void st25r95RearmListen(void);

/*! 
 *****************************************************************************
 *  \brief  Send Idle command (SPI mode)
 *
 *  This function is used to send the Idle command.
 *
 *  \param[in]   dacDataL: lower comparator value
 *  \param[in]   dacDataH: higher comparator value
 *  \param[in]   WUPeriod: time between 2 detections
 *
 *****************************************************************************
 */
extern void st25r95SPIIdle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod);

/*! 
 *****************************************************************************
 *  \brief  Flush the Idle response (SPI mode)
 *
 *  This function is used to flush the Idle response.
 *
 *****************************************************************************
 */
extern void st25r95SPIGetIdleResponse(void);

/*! 
 *****************************************************************************
 *  \brief  Send Idle command (UART mode)
 *
 *  This function is used to send the Idle command.
 *
 *  \param[in]   dacDataL: lower comparator value
 *  \param[in]   dacDataH: higher comparator value
 *  \param[in]   WUPeriod: time between 2 detections
 *
 *****************************************************************************
 */
extern void st25r95UARTIdle(uint8_t dacDataL, uint8_t dacDataH, uint8_t WUPeriod);

/*! 
 *****************************************************************************
 *  \brief  Flush the Idle response (UART mode)
 *
 *  This function is used to flush the Idle response.
 *
 *****************************************************************************
 */
extern void st25r95UARTGetIdleResponse(void);

/*! 
 *****************************************************************************
 *  \brief  Send transmit flag (SPI mode)
 *
 *  This function is used to send the transmit flag during the SendRecv command.
 *
 *  \param[in]   protocol: value of the protocol
 *  \param[in]   transmitFlag: transmission flags
 *
 *****************************************************************************
 */
extern void st25r95SPISendTransmitFlag(uint8_t protocol, uint8_t transmitFlag);

/*! 
 *****************************************************************************
 *  \brief  Send transmit flag (UART mode)
 *
 *  This function is used to send the transmit flag during the SendRecv command.
 *
 *  \param[in]   protocol: value of the protocol
 *  \param[in]   transmitFlag: transmission flags
 *
 *****************************************************************************
 */
extern void st25r95UARTSendTransmitFlag(uint8_t protocol, uint8_t transmitFlag);

/*! 
 *****************************************************************************
 *  \brief  Process end of Rx (SPI mode)
 *
 *  This function is used to conclude the Rx request and returns the status of the Rx.
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95SPICompleteRx(void);

/*! 
 *****************************************************************************
 *  \brief  Process end of Rx (UART mode)
 *
 *  This function is used to conclude the Rx request and returns the status of the Rx.
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95UARTCompleteRx(void);

/*! 
 *****************************************************************************
 *  \brief  return to ready mode (SPI mode)
 *
 *  This function is used to exit from Idle mode and return to Ready mode.
 *
 *****************************************************************************
 */
extern void st25r95SPIKillIdle(void);

/*! 
 *****************************************************************************
 *  \brief  return to ready mode (UART mode)
 *
 *  This function is used to exit from Idle mode and return to Ready mode.
 *
 *****************************************************************************
 */
extern void st25r95UARTKillIdle(void);

/*! 
 *****************************************************************************
 *  \brief Calibrate Tag Detector
 *
 *  This function returns the Tag Detector DAC calibration value.
 *
 * \return uint8_t Any : calibration value
 *
 *****************************************************************************
 */
extern uint8_t st25r95CalibrateTagDetector(void);

/*! 
 *****************************************************************************
 *  \brief SPI transceive function
 *
 *  This function is used for SPI communication.
 *
 *  \param[in]   txData: Tx Data
 *  \param[out]  rxData: Rx Data
 *  \param[in]   length: Tx/Rx Data buffer len
 *
 *****************************************************************************
 */
extern void st25r95SPIRxTx(uint8_t *txData, uint8_t *rxData, uint16_t length);

#endif /* ST25R95_COM_H */

/**
  * @}
  *
  * @}
  *
  * @}
  * 
  * @}
  */
