
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

/*! \file st25r95_com.c
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
#include "utils.h"

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
 
static uint8_t st25r95CommandIDN[] = {ST25R95_COMMAND_IDN, 0x00};

static uint8_t ProtocolSelectCommandFieldOff[]     = {0x02, 0x02, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO15693[]     = {0x02, 0x02, 0x01, 0x0D};
static uint8_t ProtocolSelectCommandISO14443A[]    = {0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO14443B[]    = {0x02, 0x05, 0x03, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ProtocolSelectCommandISO18092[]     = {0x02, 0x05, 0x04, 0x51, 0x1F, 0x06, 0x00, 0x00}; /* WA: keep len=5 & do not use DD */
static uint8_t ProtocolSelectCommandCEISO14443A[]  = {0x02, 0x02, 0x12, 0x0A};

static uint8_t *ProtocolSelectCommands[6] =
{
    ProtocolSelectCommandFieldOff,
    ProtocolSelectCommandISO15693,
    ProtocolSelectCommandISO14443A,
    ProtocolSelectCommandISO14443B,
    ProtocolSelectCommandISO18092,
    ProtocolSelectCommandCEISO14443A,
};

static uint8_t WrRegAnalogRegConfigISO15693[]   = {0x09, 0x04, 0x68, 0x01, 0x01, 0x53};
static uint8_t WrRegAnalogRegConfigISO14443A[]  = {0x09, 0x04, 0x68, 0x01, 0x01, 0xD3};
static uint8_t WrRegAnalogRegConfigISO14443B[]  = {0x09, 0x04, 0x68, 0x01, 0x01, 0x30};
static uint8_t WrRegAnalogRegConfigISO18092[]   = {0x09, 0x04, 0x68, 0x01, 0x01, 0x50};
static uint8_t WrRegAnalogRegConfigCEISO1443A[] = {0x09, 0x04, 0x68, 0x01, 0x04, 0x27};
static uint8_t *WrRegAnalogRegConfigs[6] =
{
    NULL,
    WrRegAnalogRegConfigISO15693,
    WrRegAnalogRegConfigISO14443A,
    WrRegAnalogRegConfigISO14443B,
    WrRegAnalogRegConfigISO18092,
    WrRegAnalogRegConfigCEISO1443A
};

static uint8_t WrRegEnableAutoDetectFilter[] = {0x09, 0x04, 0x0A, 0x01, 0x02, 0xA1};
static uint8_t WrRegTimerWindowValue[]       = {0x09, 0x04, 0x3A, 0x00, 0x58, 0x04};

static uint8_t Calibrate[] = {ST25R95_COMMAND_IDLE, 0x0E, 0x03, 0xA1, 0x00, 0xB8, 0x01, 0x18, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x3F, 0x01};
static uint8_t WrRegAnalogRegConfigIndex[]  = {0x09, 0x03, 0x68, 0x00, 0x01};
static uint8_t RdRegAnalogRegConfig[]       = {0x08, 0x03, 0x69, 0x01, 0x00};

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
bool st25r95CheckChipID(void)
{
    bool retCode = false;
    uint8_t respBuffer[ST25R95_IDN_RESPONSE_BUFLEN];
    
    if (st25r95SendCommandTypeAndLen(st25r95CommandIDN, respBuffer, ST25R95_IDN_RESPONSE_BUFLEN) == ERR_NONE)
    {
        if (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0)
        {
            retCode = (strcmp((const char *)&respBuffer[ST25R95_CMD_DATA_OFFSET], "NFC FS2JAST4") == 0);
        }
    }
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95ProtocolSelect(uint8_t protocol)
{
    ReturnCode retCode;
    uint8_t respBuffer[MAX(ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN, ST25R95_WRREG_RESPONSE_BUFLEN)];
    
    retCode = st25r95SendCommandTypeAndLen(ProtocolSelectCommands[protocol], respBuffer, ST25R95_PROTOCOLSELECT_RESPONSE_BUFLEN);
    if ((retCode == ERR_NONE) && (respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE))
    {
        retCode = ERR_PARAM;
    }

    /* Adjust ARC_B or ACC_A register */
    if ((protocol != ST25R95_PROTOCOL_FIELDOFF))
    {
        st25r95SendCommandTypeAndLen(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
    }

    if (protocol == ST25R95_PROTOCOL_ISO18092)
    {
        st25r95SendCommandTypeAndLen(WrRegEnableAutoDetectFilter, respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
    }
    if (protocol == ST25R95_PROTOCOL_ISO14443A)
    {
        st25r95SendCommandTypeAndLen(WrRegTimerWindowValue, respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
    }
    #if RFAL_FEATURE_LISTEN_MODE
    if (protocol == ST25R95_PROTOCOL_CE_ISO14443A) 
    {
        st25r95SPIRxCtx.inListen = false;
    }
    #endif /* RFAL_FEATURE_LISTEN_MODE */
    return (retCode);
}

/*******************************************************************************/
ReturnCode st25r95SetBitRate(uint8_t protocol, rfalBitRate txBR, rfalBitRate rxBR)
{
    uint8_t *conf;
    if ((protocol == ST25R95_PROTOCOL_FIELDOFF) || (protocol > ST25R95_PROTOCOL_MAX))
    {
        return ERR_PARAM;
    }
    conf = &ProtocolSelectCommands[protocol][ST25R95_PROTOCOLSELECT_BR_OFFSET];
    *conf &= 0x0F;
    
    switch (protocol)
    {
        case (ST25R95_PROTOCOL_ISO15693):
            switch (rxBR) 
            {
                case (RFAL_BR_26p48):
                    break;
                case (RFAL_BR_52p97):
                    *conf |= 0x10;
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }                    
            break;
        case (ST25R95_PROTOCOL_ISO14443A):
            switch (txBR) 
            {
                case (RFAL_BR_106):
                    break;
                case (RFAL_BR_212):
                    *conf |= 0x40;
                    break;
                case (RFAL_BR_424):
                    *conf |= 0x80;
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            switch (rxBR) 
            {
                case (RFAL_BR_106):
                    break;
                case (RFAL_BR_212):
                    *conf |= 0x10;
                    break;
                case (RFAL_BR_424):
                    *conf |= 0x20;
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            break;
        case (ST25R95_PROTOCOL_ISO14443B):
            switch (txBR) 
            {
                case (RFAL_BR_106):
                    break;
                case (RFAL_BR_212):
                    *conf |= 0x40;
                    break;
                case (RFAL_BR_424):
                    *conf |= 0x80;
                    break;
                case (RFAL_BR_848):
                    *conf |= 0xC0;
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            switch (rxBR) 
            {
                case (RFAL_BR_106):
                    break;
                case (RFAL_BR_212):
                    *conf |= 0x10;
                    break;
                case (RFAL_BR_424):
                    *conf |= 0x20;
                    break;
                case (RFAL_BR_848):
                    *conf |= 0x30;
                    break;                    
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            break;
        case (ST25R95_PROTOCOL_ISO18092):
            switch (txBR) 
            {
                case (RFAL_BR_212):
                    *conf |= 0x40;
                    break;
                case (RFAL_BR_424):
                    *conf |= 0x80;
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            switch (rxBR) 
            {
                case (RFAL_BR_212):
                    *conf |= 0x10;
                    break;
                case (RFAL_BR_424):
                    *conf |= 0x20;
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            break;
        case (ST25R95_PROTOCOL_CE_ISO14443A):
            switch (txBR) 
            {
                case (RFAL_BR_106):
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            switch (rxBR) 
            {
                case (RFAL_BR_106):
                    break;
                default:
                    return (ERR_NOT_IMPLEMENTED);
            }
            break;
        default:
            return (ERR_NOT_IMPLEMENTED);
    }
    return (ERR_NONE);
}

/*******************************************************************************/
ReturnCode st25r95SetFWT(uint8_t protocol, uint32_t fwt)
{
    uint8_t PP;
    uint32_t MM;
    uint32_t DD;
    uint32_t FWT;

#if ST25R95_DEBUG
    platformLog("[%10d] Set FWT=%d (protocol=%d)\r\n", platformGetSysTick(), fwt, protocol);
#endif /* ST25R95_DEBUG */   
    
    FWT = MIN( fwt, ST25R95_FWT_MAX );   /* Limit the FWT to the max supported */
    fwt = FWT;
    PP=0;

    if (protocol == ST25R95_PROTOCOL_ISO18092)
    {
        /* Workaround for ST25R95_PROTOCOL_ISO18092:
         * DD parameters seems to overwritten by MM by the ROM code.
         * So this parameter should not be used (i.e ProtocolSelect Len should be 5)
         */
        DD = 0; /* Should not be used in protocolSelect */
        while (FWT > ((128U + 1U) * (128U) * 32U))
        {
            PP++;
            FWT /= 2U;
        }
        MM = FWT / (128U * 32U);
    }
    else
    {
        while (FWT > ((128 + 1) * (255) * 32))
        {
            PP++;
            FWT /= 2;
        }
        do {
            if (FWT >  ((64 + 1) * (255) * 32))
            {
                MM = 128UL;
                break;
            }
            if (FWT >  ((32 + 1) * (255) * 32))
            {
                MM = 64UL;
                break;
            }
            if (FWT >  ((16 + 1) * (255) * 32))
            {
                MM = 32UL;
                break;
            }
            if (FWT >  ((8 + 1) * (255) * 32))
            {
                MM = 16UL;
                break;
            }
            if (FWT >  ((4 + 1) * (255) * 32))
            {
                MM = 8UL;
                break;
            }
            if (FWT >  ((2 + 1) * (255) * 32))
            {
                MM = 4UL;
                break;
            }
            if (FWT >  ((1 + 1) * (255) * 32))
            {
                MM = 2UL;
                break;
            }
            if (FWT >  ((0 + 1) * (255) * 32))
            {
                MM = 1UL;
                break;
            }
            MM = 0UL;
        } while (0);
        
        DD = (((FWT + 31UL) / 32UL) + MM) / (MM + 1UL);
        DD = (DD > 128) ? DD - 128UL : 0;
    }
    
    switch (protocol)
    {
         case ST25R95_PROTOCOL_ISO14443A:
            if (
                (ProtocolSelectCommandISO14443A[4] != PP) ||
                (ProtocolSelectCommandISO14443A[5] != MM) ||
                (ProtocolSelectCommandISO14443A[6] != DD))
            {                
                ProtocolSelectCommandISO14443A[4] = PP;
                ProtocolSelectCommandISO14443A[5] = (uint8_t)MM;
                ProtocolSelectCommandISO14443A[6] = (uint8_t)DD;
                
                return (st25r95ProtocolSelect(protocol));
            }
            break;

        case ST25R95_PROTOCOL_ISO14443B:
            if (
                (ProtocolSelectCommandISO14443B[4] != PP) ||
                (ProtocolSelectCommandISO14443B[5] != MM) ||
                (ProtocolSelectCommandISO14443B[6] != DD))
            {                
                ProtocolSelectCommandISO14443B[4] = PP;
                ProtocolSelectCommandISO14443B[5] = (uint8_t)MM;
                ProtocolSelectCommandISO14443B[6] = (uint8_t)DD;
                
                return (st25r95ProtocolSelect(protocol));
            }
            break;
            
        case ST25R95_PROTOCOL_ISO18092:
            if (
                (ProtocolSelectCommandISO18092[5] != PP) ||
                (ProtocolSelectCommandISO18092[6] != MM) ||
                (ProtocolSelectCommandISO18092[7] != DD))
            {                
                ProtocolSelectCommandISO18092[5] = PP;
                ProtocolSelectCommandISO18092[6] = (uint8_t)MM;
                ProtocolSelectCommandISO18092[7] = (uint8_t)DD;
                
                return (st25r95ProtocolSelect(protocol));
            }
            break;      
        
        default:
            break;
    }
    return (ERR_NONE);
}

/*******************************************************************************/
ReturnCode st25r95SetSlotCounter(uint8_t slots)
{
    if ((ProtocolSelectCommandISO18092[4] & 0xF) != slots)
    {
        ProtocolSelectCommandISO18092[4] &= 0xF0;
        ProtocolSelectCommandISO18092[4] |= (slots & 0xF);
        return (st25r95ProtocolSelect(ST25R95_PROTOCOL_ISO18092));
    }
    return (ERR_NONE);
}

/*******************************************************************************/
ReturnCode st25r95WriteReg(uint8_t protocol, uint16_t reg, uint8_t value)
{
    ReturnCode retCode;
    uint8_t respBuffer[ST25R95_WRREG_RESPONSE_BUFLEN];
    
    switch (reg)
    {
        case (ST25R95_REG_ARC_B):
            if ((protocol == ST25R95_PROTOCOL_ISO15693)  ||
                (protocol == ST25R95_PROTOCOL_ISO14443A) ||
                (protocol == ST25R95_PROTOCOL_ISO14443B) ||
                (protocol == ST25R95_PROTOCOL_ISO18092))
            {
                WrRegAnalogRegConfigs[protocol][5] = value;
                st25r95SendCommandTypeAndLen(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
                retCode = (respBuffer[ST25R95_CMD_RESULT_OFFSET] == 0) ? ERR_NONE : ERR_PARAM;
            }
            else
            {
                retCode = ERR_WRONG_STATE;
            }
            break;
            
        case (ST25R95_REG_ACC_A):
            if (protocol == ST25R95_PROTOCOL_CE_ISO14443A)
            {
                WrRegAnalogRegConfigs[protocol][5] = value;
                st25r95SendCommandTypeAndLen(WrRegAnalogRegConfigs[protocol], respBuffer, ST25R95_WRREG_RESPONSE_BUFLEN);
                retCode = (respBuffer[ST25R95_CMD_RESULT_OFFSET] == 0) ? ERR_NONE : ERR_PARAM;
            }
            else
            {
                retCode = ERR_WRONG_STATE;
            }
            break;
        default:
            retCode = ERR_PARAM;
            break;
    }
#if ST25R95_DEBUG    
    platformLog("%s: retCode: %2.2x\r\n", __FUNCTION__, retCode);
#endif /* ST25R95_DEBUG */ 
    return (retCode);
}

/*******************************************************************************/
uint8_t st25r95CalibrateTagDetector(void)
{
    const uint8_t steps[6] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x4U};
    uint8_t       respBuffer[ST25R95_IDLE_RESPONSE_BUFLEN];
    uint8_t       i;
    
    /* 8 steps dichotomy implementation as per AN3433 */
    
    /* Check that wake up detection is tag detect (0x02) when DacDataH is Min Dac value 0x00 */ 
    Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] = 0x00U; 
    st25r95SendCommandTypeAndLen(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
    if ((respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) || (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0x01) || (respBuffer[ST25R95_CMD_DATA_OFFSET] != ST25R95_IDLE_WKUP_TAGDETECT))
    {
        return (0xFFU);
    }
    /* Check that wake up detection is timeout (0x01) when DacDataH is Max Dac value 0xFC */ 
    Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] = ST25R95_DACDATA_MAX;
    st25r95SendCommandTypeAndLen(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);
    if ((respBuffer[ST25R95_CMD_RESULT_OFFSET] != ST25R95_ERRCODE_NONE) || (respBuffer[ST25R95_CMD_LENGTH_OFFSET] != 0x01) || (respBuffer[ST25R95_CMD_DATA_OFFSET] != ST25R95_IDLE_WKUP_TIMEOUT))
    {
        return (0xFFU);
    }
    
    for (i = 0; i < 6; i++)
    {
        switch (respBuffer[ST25R95_CMD_DATA_OFFSET])
        {
            case ST25R95_IDLE_WKUP_TIMEOUT:
                Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] -= steps[i];
                break;
            case ST25R95_IDLE_WKUP_TAGDETECT:
                Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] += steps[i];
                break;
            default:
                return ERR_SYSTEM;
                /*NOTREACHED*/
                break;
        }
        respBuffer[ST25R95_CMD_DATA_OFFSET] = 0x00U;
        st25r95SendCommandTypeAndLen(Calibrate, respBuffer, ST25R95_IDLE_RESPONSE_BUFLEN);  
    }
    if (respBuffer[2U] == ST25R95_IDLE_WKUP_TIMEOUT)
    {
        Calibrate[ST25R95_IDLE_DACDATAH_OFFSET] -= 0x04U;
    }
    return (Calibrate[ST25R95_IDLE_DACDATAH_OFFSET]);
}

/*******************************************************************************/
ReturnCode st25r95ReadReg(uint16_t reg, uint8_t *value)
{
    ReturnCode retCode = ERR_NONE;
    uint8_t respBuffer[ST25R95_RDREG_RESPONSE_BUFLEN];
    
     switch (reg)
    {
        case (ST25R95_REG_ARC_B):
            WrRegAnalogRegConfigIndex[4U] = 0x01U;
            break;
            
        case (ST25R95_REG_ACC_A):
            WrRegAnalogRegConfigIndex[4U] = 0x04U;
            break;
        
        default:
            retCode = ERR_PARAM;
            break;
    }
    if (retCode == ERR_NONE)
    {
        st25r95SendCommandTypeAndLen(WrRegAnalogRegConfigIndex, respBuffer, ST25R95_RDREG_RESPONSE_BUFLEN);
        if (respBuffer[ST25R95_CMD_RESULT_OFFSET] == ST25R95_ERRCODE_NONE) 
        {
            st25r95SendCommandTypeAndLen(RdRegAnalogRegConfig, respBuffer, ST25R95_RDREG_RESPONSE_BUFLEN);
            if (respBuffer[ST25R95_CMD_RESULT_OFFSET] == ST25R95_ERRCODE_NONE) 
            {
                *value = respBuffer[2];
            }
            else
            {
                retCode = ERR_PARAM;
            }
        }
        else
        {
            retCode = ERR_PARAM;    
        }
    }

#if ST25R95_DEBUG
    platformLog("%s: retCode: %2.2x\r\n", __FUNCTION__, retCode);
#endif /* ST25R95_DEBUG */   
    return (retCode);
    
}
