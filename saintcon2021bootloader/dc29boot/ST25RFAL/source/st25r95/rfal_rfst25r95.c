
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

/*! \file rfal_rfst25r95.c
 *
 *  \author 
 *
 *  \brief RF Abstraction Layer (RFAL)
 *
 *  RFAL implementation for ST25R95
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "rfal_rf.h"
#include "utils.h"
#include "platform.h"
#include "st25r95.h"
#include "st25r95_com.h"
#include "rfal_nfcf.h"
#include "rfal_nfca.h"
#include "rfal_analogConfig.h"

/*
 ******************************************************************************
 * ENABLE SWITCHS
 ******************************************************************************
 */
 
#ifndef RFAL_FEATURE_LISTEN_MODE
    #define RFAL_FEATURE_LISTEN_MODE    false    /* Listen Mode configuration missing. Disabled by default */
#endif /* RFAL_FEATURE_LISTEN_MODE */

#ifndef RFAL_FEATURE_WAKEUP_MODE
    #define RFAL_FEATURE_WAKEUP_MODE    false    /* Wake-Up mode configuration missing. Disabled by default */
#endif /* RFAL_FEATURE_WAKEUP_MODE */
    
#ifndef RFAL_FEATURE_LOWPOWER_MODE
    #define RFAL_FEATURE_LOWPOWER_MODE  false    /* Low Power mode configuration missing. Disabled by default */
#endif /* RFAL_FEATURE_LOWPOWER_MODE */

/*
******************************************************************************
* GLOBAL TYPES
******************************************************************************
*/

/*! Struct that holds all involved on a Transceive including the context passed by the caller     */
typedef struct{
    rfalTransceiveState     state;       /*!< Current transceive state                            */
    rfalTransceiveState     lastState;   /*!< Last transceive state (debug purposes)              */
    ReturnCode              status;      /*!< Current status/error of the transceive              */
    
    rfalTransceiveContext   ctx;         /*!< The transceive context given by the caller          */
} rfalTxRx;

/*! Struct that holds all context for the Listen Mode                                             */
typedef struct{
    uint8_t*                rxBuf;       /*!< Location to store incoming data in Listen Mode      */
    uint16_t                rxBufLen;    /*!< Length of rxBuf                                     */
    uint16_t*               rxLen;       /*!< Pointer to write the data length placed into rxBuf  */
    bool                    dataFlag;    /*!< Listen Mode current Data Flag                       */
} rfalLm;

/*! Struct that holds all context for the Wake-Up Mode                                             */
typedef struct{
    rfalWumState            state;       /*!< Current Wake-Up Mode state                           */
    rfalWakeUpConfig        cfg;         /*!< Current Wake-Up Mode context                         */
    uint8_t                 CalTagDet;   /*!< Tag Detection calibration value                      */
} rfalWum;

typedef struct{
    uint32_t                GT;          /*!< GT in 1/fc                  */
    uint32_t                FDTListen;   /*!< FDTListen in 1/fc           */
    uint32_t                FDTPoll;     /*!< FDTPoll in 1/fc             */
} rfalTimings;

/*! Struct that holds the software timers                                 */
typedef struct{
    uint32_t                GT;          /*!< RFAL's GT timer             */
    uint32_t                FDTPoll;     /*!< RFAL's FST Poll timer       */
} rfalTimers;

/*! Struct that holds the RFAL's callbacks                                */
typedef struct{
    rfalPreTxRxCallback     preTxRx;     /*!< RFAL's Pre TxRx callback    */
    rfalPostTxRxCallback    postTxRx;    /*!< RFAL's Post TxRx callback   */
    rfalSyncTxRxCallback    syncTxRx;    /*!< RFAL's Sync TxRx callback */
} rfalCallbacks;

/*! Struct that holds NFC-F data - Used only inside rfalFelicaPoll() (static to avoid adding it into stack) */
typedef struct{    
    uint16_t           actLen;
    rfalFeliCaPollRes* pollResList;
    uint8_t            pollResListSize;
    uint8_t            *devicesDetected;
    uint8_t            *collisionsDetected;
    rfalFeliCaPollRes  pollResponses[RFAL_FELICA_POLL_MAX_SLOTS];   /* FeliCa Poll response container for 16 slots */
} rfalNfcfWorkingData;

typedef struct{
    rfalState             state;     /*!< RFAL's current state                            */
    rfalMode              mode;      /*!< RFAL's current mode                             */
    rfalBitRate           txBR;      /*!< RFAL's current Tx Bit Rate                      */
    rfalBitRate           rxBR;      /*!< RFAL's current Rx Bit Rate                      */
    bool                  field;     /*!< Current field state (On / Off)                  */
    
    rfalTimings           timings;   /*!< RFAL's timing setting                           */
    rfalTxRx              TxRx;      /*!< RFAL's transceive management                    */
    rfalLm                Lm;        /*!< RFAL's listen mode management                   */
    rfalWum               wum;       /*!< RFAL's Wake-Up mode management                  */
    
    rfalTimers            tmr;       /*!< RFAL's Software timers                          */
    rfalCallbacks         callbacks; /*!< RFAL's callbacks                                */

    uint8_t               protocol;  /*!< ProtocolSelect protocol                         */
    uint8_t               RxInformationBytes[3]; /*!< ST25R95 additional information bytes*/
#if RFAL_FEATURE_NFCF
    rfalNfcfWorkingData   nfcfData; /*!< RFAL's working data when supporting NFC-F        */
#endif /* RFAL_FEATURE_NFCF */
    bool                  NfcaSplitFrame;
#if RFAL_FEATURE_LISTEN_MODE
    bool                  cardEmulT4AT;
#endif /* RFAL_FEATURE_LISTEN_MODE */
} rfal;

/*! Felica's command set */
typedef enum 
{
    FELICA_CMD_POLLING                  = 0x00, /*!< Felica Poll/REQC command (aka SENSF_REQ) to identify a card    */
    FELICA_CMD_POLLING_RES              = 0x01, /*!< Felica Poll/REQC command (aka SENSF_RES) response              */
    FELICA_CMD_REQUEST_SERVICE          = 0x02, /*!< verify the existence of Area and Service                       */
    FELICA_CMD_REQUEST_RESPONSE         = 0x04, /*!< verify the existence of a card                                 */
    FELICA_CMD_READ_WITHOUT_ENCRYPTION  = 0x06, /*!< read Block Data from a Service that requires no authentication */
    FELICA_CMD_WRITE_WITHOUT_ENCRYPTION = 0x08, /*!< write Block Data to a Service that requires no authentication  */
    FELICA_CMD_REQUEST_SYSTEM_CODE      = 0x0c, /*!< acquire the System Code registered to a card                   */
    FELICA_CMD_AUTHENTICATION1          = 0x10, /*!< authenticate a card                                            */
    FELICA_CMD_AUTHENTICATION2          = 0x12, /*!< allow a card to authenticate a Reader/Writer                   */
    FELICA_CMD_READ                     = 0x14, /*!< read Block Data from a Service that requires authentication    */
    FELICA_CMD_WRITE                    = 0x16, /*!< write Block Data to a Service that requires authentication     */
} t_rfalFeliCaCmd;

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define RFAL_ST25R95_GPT_MAX_1FC         rfalConv8fcTo1fc(0xFFFF)                     /*!< Max GPT steps in 1fc (0xFFFF steps of 8/fc    => 0xFFFF * 590ns  = 38,7ms)      */
#define RFAL_ST25R95_NRT_MAX_1FC         rfalConv4096fcTo1fc(0xFFFF)                  /*!< Max NRT steps in 1fc (0xFFFF steps of 4096/fc => 0xFFFF * 302us  = 19.8s)       */
#define RFAL_ST25R95_NRT_DISABLED        0                                            /*!< NRT Disabled: All 0 No-response timer is not started, wait forever              */
#define RFAL_ST25R95_MRT_MAX_1FC         rfalConv64fcTo1fc(0x00FF)                    /*!< Max MRT steps in 1fc (0x00FF steps of 64/fc   => 0x00FF * 4.72us = 1.2ms)       */
#define RFAL_ST25R95_MRT_MIN_1FC         rfalConv64fcTo1fc(0x0004)                    /*!< Min MRT steps in 1fc (0<=mrt<=4 ; 4 (64/fc)  => 0x0004 * 4.72us = 18.88us)      */
#define RFAL_ST25R95_GT_MAX_1FC          rfalConvMsTo1fc(5000)                        /*!< Max GT value allowed in 1/fc                                                    */            
#define RFAL_ST25R95_GT_MIN_1FC          rfalConvMsTo1fc(RFAL_ST25R95_SW_TMR_MIN_1MS) /*!< Min GT value allowed in 1/fc                                                    */
#define RFAL_ST25R95_SW_TMR_MIN_1MS      1          

#define RFAL_FELICA_POLL_DELAY_TIME     512                                           /*!<  FeliCa Poll Processing time is 2.417 ms ~512*64/fc Digital 1.1 A4              */
#define RFAL_FELICA_POLL_SLOT_TIME      256                                           /*!<  FeliCa Poll Time Slot duration is 1.208 ms ~256*64/fc Digital 1.1 A4           */

#define RFAL_ISO14443A_SDD_RES_LEN      5                                             /*!< SDD_RES | Anticollision (UID CLn) length  -  rfalNfcaSddRes                     */

#define RFAL_ST25R95_ISO14443A_APPENDCRC                                             0x20U /*!< Transmission flags bit 5: Append CRC        */
#define RFAL_ST25R95_ISO14443A_SPLITFRAME                                            0x40U /*!< Transmission flags bit 6: SplitFrame        */
#define RFAL_ST25R95_ISO14443A_TOPAZFORMAT                                           0x80U /*!< Transmission flags bit 7: Topaz send format */

#define RFAL_ST25R95_IDLE_DEFAULT_WUPERIOD                                           0x24U /*!< Fixed WU Period to reach ~300 ms timeout with Max Sleep = 0 */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/
#define rfalTimerStart( timer, time_ms )         do{ platformTimerDestroy( timer ); (timer) = platformTimerCreate((uint16_t)(time_ms)); } while(0) /*!< Configures and starts timer */
#define rfalTimerisExpired( timer )              platformTimerIsExpired( timer )           /*!< Checks if timer has expired                                                         */
#define rfalTimerDestroy( timer )                platformTimerDestroy( timer )             /*!< Destroys timer                                                                      */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static rfal gRFAL;              /*!< RFAL module instance               */

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void rfalTransceiveTx(void);
static void rfalTransceiveRx(void);
static ReturnCode rfalTransceiveRunBlockingTx(void);
static bool rfalChipIsBusy(void);

static ReturnCode rfalRunTransceiveWorker( void );
#if RFAL_FEATURE_LISTEN_MODE
static ReturnCode rfalRunListenModeWorker( void );
#endif /* RFAL_FEATURE_LISTEN_MODE */
#if RFAL_FEATURE_WAKEUP_MODE
static void rfalRunWakeUpModeWorker( void );
#endif /* RFAL_FEATURE_WAKEUP_MODE */

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode rfalInitialize(void)
{
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    /* Initialize chip */
    if (st25r95Initialize() != ERR_NONE)
    {
        return (ERR_SYSTEM);
    }
    
    /* Check expected chip: ST25R95 */
    if(!st25r95CheckChipID())
    {
        return ERR_HW_MISMATCH;
    }
    
    /*******************************************************************************/
    /* Debug purposes */
    /*LogSetLevel(LOG_MODULE_DEFAULT, LOG_LEVEL_INFO); !!!!!!!!!!!!!!! */
    
    /*******************************************************************************/
    gRFAL.state              = RFAL_STATE_INIT;
    gRFAL.mode               = RFAL_MODE_NONE;
    gRFAL.protocol           = ST25R95_PROTOCOL_FIELDOFF;
    gRFAL.field              = false;
        
    /* Disable all timings */
    gRFAL.timings.FDTListen  = RFAL_TIMING_NONE;
    gRFAL.timings.FDTPoll    = RFAL_TIMING_NONE;
    gRFAL.timings.GT         = RFAL_TIMING_NONE;
    
    
    rfalTimerDestroy(gRFAL.tmr.GT);
    rfalTimerDestroy(gRFAL.tmr.FDTPoll);
    gRFAL.tmr.GT             = RFAL_TIMING_NONE;
    gRFAL.tmr.FDTPoll        = RFAL_TIMING_NONE;
    
    gRFAL.callbacks.preTxRx  = NULL;
    gRFAL.callbacks.postTxRx = NULL;
    gRFAL.callbacks.syncTxRx = NULL;
     
    /* Initialize Wake-Up Mode */
    gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;
    #if ST25R95_TAGDETECT_CALIBRATE || !defined(ST25R95_TAGDETECT_DEF_CALIBRATION)
    gRFAL.wum.CalTagDet = st25r95CalibrateTagDetector();
    #else
    gRFAL.wum.CalTagDet = ST25R95_TAGDETECT_DEF_CALIBRATION;
    #endif /* ST25R95_TAGDETECT_CAL */
    if (gRFAL.wum.CalTagDet == 0xFFU)
    {
        return ERR_SYSTEM;
    }
    return ERR_NONE;
}

/*******************************************************************************/
ReturnCode rfalCalibrate(void)
{
    return (ERR_NONE);
}

/*******************************************************************************/
ReturnCode rfalAdjustRegulators(uint16_t* result)
{
    NO_WARNING(result);
    return (ERR_NONE);
}

/*******************************************************************************/
void rfalSetUpperLayerCallback(rfalUpperLayerCallback pFunc)
{
    NO_WARNING(pFunc);
    return;
}

/*******************************************************************************/
void rfalSetPreTxRxCallback(rfalPreTxRxCallback pFunc)
{
    gRFAL.callbacks.preTxRx = pFunc;
}


/*******************************************************************************/
void rfalSetSyncTxRxCallback( rfalSyncTxRxCallback pFunc )
{
    gRFAL.callbacks.syncTxRx = pFunc;
}


/*******************************************************************************/
void rfalSetPostTxRxCallback(rfalPostTxRxCallback pFunc)
{
    gRFAL.callbacks.postTxRx = pFunc;
}

/*******************************************************************************/
ReturnCode rfalDeinitialize(void)
{
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    /* Deinitialize chip */
    st25r95Deinitialize();
 
    gRFAL.state = RFAL_STATE_IDLE;
    return ERR_NONE;
}

/*******************************************************************************/
void rfalSetObsvMode(uint32_t txMode, uint32_t rxMode)
{
    NO_WARNING(txMode);
    NO_WARNING(rxMode);
    return;
}


/*******************************************************************************/
void rfalGetObsvMode(uint8_t* txMode, uint8_t* rxMode)
{
    NO_WARNING(txMode);
    NO_WARNING(rxMode);
    return;
}

/*******************************************************************************/
void rfalDisableObsvMode(void)
{
    return;
}

/*******************************************************************************/
ReturnCode rfalSetMode(rfalMode mode, rfalBitRate txBR, rfalBitRate rxBR)
{
    /* Check if RFAL is not initialized */
    if (gRFAL.state == RFAL_STATE_IDLE)
    {
        return ERR_WRONG_STATE;
    }
    
    /* Check allowed bit rate value */
    if ((txBR == RFAL_BR_KEEP) || (rxBR == RFAL_BR_KEEP))
    {
        return ERR_PARAM;
    }
   
    /*******************************************************************************/
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
   
    switch (mode)
    {
        /*******************************************************************************/
        case RFAL_MODE_POLL_NFCA:        
        case RFAL_MODE_POLL_NFCA_T1T:
            gRFAL.protocol = ST25R95_PROTOCOL_ISO14443A;
            gRFAL.NfcaSplitFrame = false;
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX) );
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX) );
            break;
        case RFAL_MODE_POLL_NFCB:
            gRFAL.protocol = ST25R95_PROTOCOL_ISO14443B;
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX) );
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCB | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX) );
            break;
        case RFAL_MODE_POLL_NFCF:
            gRFAL.protocol = ST25R95_PROTOCOL_ISO18092;
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX) );
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCF | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX) );
            break;        
        case RFAL_MODE_POLL_NFCV:
            gRFAL.protocol = ST25R95_PROTOCOL_ISO15693;
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX) );
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_POLL | RFAL_ANALOG_CONFIG_TECH_NFCV | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX) );
            break;
        case RFAL_MODE_LISTEN_NFCA:
            #if RFAL_FEATURE_LISTEN_MODE
            gRFAL.protocol = ST25R95_PROTOCOL_CE_ISO14443A;
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_LISTEN| RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_TX) );
            rfalSetAnalogConfig( (RFAL_ANALOG_CONFIG_LISTEN| RFAL_ANALOG_CONFIG_TECH_NFCA | RFAL_ANALOG_CONFIG_BITRATE_COMMON | RFAL_ANALOG_CONFIG_RX) );
            break;  
            #else
            return ERR_NOTSUPP;
            /*NOTREACHED*/
            break;
            #endif /* RFAL_FEATURE_LISTEN_MODE */
        /*******************************************************************************/
        case RFAL_MODE_POLL_B_PRIME:
        case RFAL_MODE_POLL_B_CTS:
        case RFAL_MODE_POLL_PICOPASS:
        case RFAL_MODE_POLL_ACTIVE_P2P:
        case RFAL_MODE_LISTEN_ACTIVE_P2P:
        case RFAL_MODE_LISTEN_NFCB:
        case RFAL_MODE_LISTEN_NFCF:
            return ERR_NOTSUPP;
            /*NOTREACHED*/
            break;
            
        /*******************************************************************************/
        default:
            return ERR_NOT_IMPLEMENTED;
    }
    
    /* Set state as STATE_MODE_SET only if not initialized yet (PSL) */
    gRFAL.state = ((gRFAL.state < RFAL_STATE_MODE_SET) ? RFAL_STATE_MODE_SET : gRFAL.state);
    gRFAL.mode  = mode;
    
    /* Apply the given bit rate and mode */
    return (rfalSetBitRate(txBR, rxBR));
}

/*******************************************************************************/
rfalMode rfalGetMode(void)
{
    return gRFAL.mode;
}

/*******************************************************************************/
ReturnCode rfalSetBitRate(rfalBitRate txBR, rfalBitRate rxBR)
{
    ReturnCode retCode = ERR_NONE;

    /* Check if RFAL is not initialized */
    if (gRFAL.state == RFAL_STATE_IDLE)
    {
        return ERR_WRONG_STATE;
    }
   
    /*******************************************************************************/
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
   
    /* Store the new Bit Rates */
    gRFAL.txBR = ((txBR == RFAL_BR_KEEP) ? gRFAL.txBR : txBR);
    gRFAL.rxBR = ((rxBR == RFAL_BR_KEEP) ? gRFAL.rxBR : rxBR);
    
    retCode = st25r95SetBitRate(gRFAL.protocol, txBR, rxBR);
    if ((retCode == ERR_NONE) && (gRFAL.protocol != ST25R95_PROTOCOL_FIELDOFF))
    {
        /* If field on, update bitrate value through ProtocolSelect */
        retCode = st25r95ProtocolSelect(gRFAL.protocol);
    }
      
    return (retCode);
}

/*******************************************************************************/
ReturnCode rfalGetBitRate(rfalBitRate *txBR, rfalBitRate *rxBR)
{
    if ((gRFAL.state == RFAL_STATE_IDLE) || (gRFAL.mode == RFAL_MODE_NONE))
    {
        return ERR_WRONG_STATE;
    }
    
    if (txBR != NULL)
    {
        *txBR = gRFAL.txBR;
    }
    
    if (rxBR != NULL)
    {
        *rxBR = gRFAL.rxBR;
    }
    
    return ERR_NONE;
}

/*******************************************************************************/
void rfalSetErrorHandling(rfalEHandling eHandling)
{
    NO_WARNING(eHandling);
    return;
}

/*******************************************************************************/
rfalEHandling rfalGetErrorHandling(void)
{
    return RFAL_ERRORHANDLING_NONE;
}

/*******************************************************************************/
void rfalSetFDTPoll(uint32_t FDTPoll)
{
    gRFAL.timings.FDTPoll = MIN(FDTPoll, RFAL_ST25R95_GPT_MAX_1FC);
}

/*******************************************************************************/
uint32_t rfalGetFDTPoll(void)
{
    return gRFAL.timings.FDTPoll;
}

/*******************************************************************************/
void rfalSetFDTListen(uint32_t FDTListen)
{
    gRFAL.timings.FDTListen = MIN(FDTListen, RFAL_ST25R95_MRT_MAX_1FC);
}

/*******************************************************************************/
uint32_t rfalGetFDTListen(void)
{
    return gRFAL.timings.FDTListen;
}

/*******************************************************************************/
void rfalSetGT(uint32_t GT)
{
    gRFAL.timings.GT = MIN(GT, RFAL_ST25R95_GT_MAX_1FC);
}

/*******************************************************************************/
uint32_t rfalGetGT(void)
{
    return gRFAL.timings.GT;
}

/*******************************************************************************/
bool rfalIsGTExpired(void)
{
    if (gRFAL.tmr.GT != RFAL_TIMING_NONE)
    {
        if (!rfalTimerisExpired(gRFAL.tmr.GT))
        {
            return false;
        }
    }
    return true;
}

/*******************************************************************************/
ReturnCode rfalFieldOnAndStartGT(void)
{
    ReturnCode ret;
    
    /* Check if RFAL has been initialized  */
    if ((gRFAL.state < RFAL_STATE_INIT))
    {
        return ERR_WRONG_STATE;
    }
    
    /*******************************************************************************/
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    
    ret = ERR_NONE;
    
    /*******************************************************************************/
    /* Turn field On if not already On */
    if (!gRFAL.field)
    {
        ret = st25r95FieldOn(gRFAL.protocol);   
        gRFAL.field = true;
    }
    
    /*******************************************************************************/
    /* Start GT timer in case the GT value is set */
    if ((gRFAL.timings.GT != RFAL_TIMING_NONE))
    {
        /* Ensure that a SW timer doesn't have a lower value then the minimum  */
        rfalTimerStart(gRFAL.tmr.GT, rfalConv1fcToMs(MAX((gRFAL.timings.GT),RFAL_ST25R95_GT_MIN_1FC)));
    }
    
    return ret;
}

/*******************************************************************************/
ReturnCode rfalFieldOff(void)
{
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }

#if RFAL_FEATURE_WAKEUP_MODE
    rfalWakeUpModeStop();
#endif /* RFAL_FEATURE_WAKEUP_MODE */

    gRFAL.field = false;
    gRFAL.protocol = ST25R95_PROTOCOL_FIELDOFF;
    return (st25r95FieldOff());
}



/*******************************************************************************/
ReturnCode rfalStartTransceive( const rfalTransceiveContext *ctx )
{
    /* Ensure that RFAL is already Initialized and the mode has been set */
    if ((gRFAL.state >= RFAL_STATE_MODE_SET))
    {
        /*******************************************************************************/
        /* Ensure that no previous operation is still ongoing */
        if( rfalChipIsBusy() )
        {        
            return ERR_REQUEST;
        }
    
        gRFAL.TxRx.ctx = *ctx;
        
        /*******************************************************************************/
        if (rfalIsModePassiveComm(gRFAL.mode))  /* Passive Comms */
        {
            if ((gRFAL.TxRx.ctx.fwt != RFAL_FWT_NONE) && (gRFAL.TxRx.ctx.fwt != 0))
            {
                st25r95SetFWT(gRFAL.protocol, gRFAL.TxRx.ctx.fwt);
            }
            else
            {
                /* Since ST25R95 does not support, use max FWT available */
                st25r95SetFWT( gRFAL.protocol, ST25R95_FWT_MAX );
            }
        }
        
        gRFAL.state       = RFAL_STATE_TXRX;
        gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_IDLE;
        gRFAL.TxRx.status = ERR_BUSY;        
        gRFAL.Lm.dataFlag = false;
     
#if RFAL_FEATURE_NFCV        
        /*******************************************************************************/
        if ((RFAL_MODE_POLL_NFCV == gRFAL.mode) || (RFAL_MODE_POLL_PICOPASS == gRFAL.mode))
        { 
            /* In NFCV a TxRx with a valid txBuf and txBufSize==0 indicates to send an EOF */
            /* Skip logic below that would go directly into receive                        */
            if ( gRFAL.TxRx.ctx.txBuf != NULL )
            {
                return  ERR_NONE;
            }
        }
#endif /* RFAL_FEATURE_NFCV */


        /*******************************************************************************/
        /* Check if the Transceive start performing Tx or goes directly to Rx          */
        if ((gRFAL.TxRx.ctx.txBuf == NULL) || (gRFAL.TxRx.ctx.txBufLen == 0))
        {
            return ERR_NOT_IMPLEMENTED;
        }
    
        return ERR_NONE;
    }
    
    return ERR_WRONG_STATE;
}


/*******************************************************************************/
bool rfalIsTransceiveInTx( void )
{
    return ( (gRFAL.TxRx.state >= RFAL_TXRX_STATE_TX_IDLE) && (gRFAL.TxRx.state < RFAL_TXRX_STATE_RX_IDLE) );
}


/*******************************************************************************/
bool rfalIsTransceiveInRx( void )
{
    return (gRFAL.TxRx.state >= RFAL_TXRX_STATE_RX_IDLE);
}


/*******************************************************************************/
ReturnCode rfalTransceiveBlockingTx(uint8_t* txBuf, uint16_t txBufLen, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t* actLen, uint32_t flags, uint32_t fwt)
{
    ReturnCode               ret;
    rfalTransceiveContext    ctx;
    
    rfalCreateByteFlagsTxRxContext(ctx, txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt);
    EXIT_ON_ERR(ret, rfalStartTransceive(&ctx));

    
    return rfalTransceiveRunBlockingTx();
}

/*******************************************************************************/
ReturnCode rfalTransceiveBlockingRx(void)
{
    ReturnCode ret;
    
    do{
        rfalWorker();
    }
    while (((ret = rfalGetTransceiveStatus()) == ERR_BUSY) || rfalIsTransceiveInRx());    
        
    return ret;
}


/*******************************************************************************/
ReturnCode rfalTransceiveBlockingTxRx(uint8_t* txBuf, uint16_t txBufLen, uint8_t* rxBuf, uint16_t rxBufLen, uint16_t* actLen, uint32_t flags, uint32_t fwt)
{
    ReturnCode ret;
    
    EXIT_ON_ERR(ret, rfalTransceiveBlockingTx(txBuf, txBufLen, rxBuf, rxBufLen, actLen, flags, fwt));
    ret = rfalTransceiveBlockingRx();
    
    /* Convert received bits to bytes */
    if (actLen != NULL)
    {
        *actLen = rfalConvBitsToBytes(*actLen);
    }
    
    return ret;
}


/*******************************************************************************/
static ReturnCode rfalRunTransceiveWorker(void)
{
    if (gRFAL.state == RFAL_STATE_TXRX)
    {     
        /* Run Tx or Rx state machines */
        if (rfalIsTransceiveInTx())
        {
            rfalTransceiveTx();
            return rfalGetTransceiveStatus();
        }
        else if (rfalIsTransceiveInRx())
        {
            rfalTransceiveRx();
            return rfalGetTransceiveStatus();
        }
    }    
    return ERR_WRONG_STATE;
}

/*******************************************************************************/
rfalTransceiveState rfalGetTransceiveState(void)
{
    return gRFAL.TxRx.state;
}

ReturnCode rfalGetTransceiveStatus(void)
{
    return ((gRFAL.TxRx.state == RFAL_TXRX_STATE_IDLE) ? gRFAL.TxRx.status : ERR_BUSY);
}


/*******************************************************************************/
ReturnCode rfalGetTransceiveRSSI(uint16_t *rssi)
{
    NO_WARNING(rssi);
    
    return ERR_NOTSUPP;
}


/*******************************************************************************/
void rfalWorker(void)
{
    platformProtectWorker();               /* Protect RFAL Worker/Task/Process */
    
    switch (gRFAL.state)
    {
        case RFAL_STATE_TXRX:
            rfalRunTransceiveWorker();
            break;
            
    #if RFAL_FEATURE_LISTEN_MODE
        case RFAL_STATE_LM:
            rfalRunListenModeWorker();
            break;
    #endif /* RFAL_FEATURE_LISTEN_MODE */
        
    #if RFAL_FEATURE_WAKEUP_MODE
        case RFAL_STATE_WUM:
            rfalRunWakeUpModeWorker();
            break;
    #endif /* RFAL_FEATURE_WAKEUP_MODE */

        /* Nothing to be done */
        default:            
            break;
    }
    
    platformUnprotectWorker();             /* Unprotect RFAL Worker/Task/Process */
}

/*******************************************************************************/
static void rfalTransceiveTx(void)
{
    uint8_t transmitFlag = 0;
    
    if (gRFAL.TxRx.state != gRFAL.TxRx.lastState)
    {        
        /* rfalLogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, gRFAL.TxRx.lastState, gRFAL.TxRx.state);*/
        gRFAL.TxRx.lastState = gRFAL.TxRx.state;
    }
    
    switch (gRFAL.TxRx.state)
    {
        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_IDLE:
            /* Nothing to do */
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_GT ;
            /* fall through */

        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_WAIT_GT:
            /* Wait for GT and FDT Poll */

            if (!rfalIsGTExpired() || !rfalTimerisExpired(gRFAL.tmr.FDTPoll))
            {
                break;
            }
            
            rfalTimerDestroy(gRFAL.tmr.FDTPoll);
            rfalTimerDestroy(gRFAL.tmr.GT);
            gRFAL.tmr.GT      = RFAL_TIMING_NONE;
            gRFAL.tmr.FDTPoll = RFAL_TIMING_NONE;
            
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_PREP_TX;
            /* fall through */

        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_PREP_TX:
            /*******************************************************************************/
            /* Execute Pre Transceive Callback                                             */
            /*******************************************************************************/
            if( gRFAL.callbacks.preTxRx != NULL )
            {   
                gRFAL.callbacks.preTxRx();
            }
            /*******************************************************************************/
            /* Prepare Rx                                                                  */
            /*******************************************************************************/
            st25r95PrepareRx(
                gRFAL.protocol, 
                gRFAL.TxRx.ctx.rxBuf, 
                rfalConvBitsToBytes(gRFAL.TxRx.ctx.rxBufLen), 
                gRFAL.TxRx.ctx.rxRcvdLen, 
                gRFAL.TxRx.ctx.flags, 
                gRFAL.RxInformationBytes
                );
            
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_TRANSMIT;
            /* fall through */
            
        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_TRANSMIT:

            /*******************************************************************************/
            /* Execute Sync Transceive Callback                                             */
            /*******************************************************************************/
            if( gRFAL.callbacks.syncTxRx != NULL )
            {
                /* If set, wait for sync callback to signal sync/trigger transmission */
                if( !gRFAL.callbacks.syncTxRx() )
                {
                    break;
                }
            }
            
            /*******************************************************************************/
            /* Send the data                                                               */
            /*******************************************************************************/
            st25r95SendData(gRFAL.TxRx.ctx.txBuf, rfalConvBitsToBytes(gRFAL.TxRx.ctx.txBufLen), gRFAL.protocol, gRFAL.TxRx.ctx.flags);
            
            /* Start FDTPoll SW timer */
            rfalTimerStart( gRFAL.tmr.FDTPoll, (RFAL_ST25R95_SW_TMR_MIN_1MS + rfalConv1fcToMs(gRFAL.timings.FDTPoll)) );
            
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_WAIT_TXE;
            /* fall through */

        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_WAIT_TXE:
            if (!st25r95IsTransmitTxCompleted())
            {
                break;
            }
#if RFAL_FEATURE_NFCA
            transmitFlag = gRFAL.TxRx.ctx.txBufLen % 8;
            if (transmitFlag == 0 )
            {
                transmitFlag = 0x8;
            }
            if (!(gRFAL.TxRx.ctx.flags & RFAL_TXRX_FLAGS_CRC_TX_MANUAL))
            {
                transmitFlag |= RFAL_ST25R95_ISO14443A_APPENDCRC;
            }
            if (gRFAL.NfcaSplitFrame) 
            {
                transmitFlag |= RFAL_ST25R95_ISO14443A_SPLITFRAME;
            }
            if (gRFAL.mode == RFAL_MODE_POLL_NFCA_T1T)
            {
                transmitFlag |= RFAL_ST25R95_ISO14443A_TOPAZFORMAT;
            }
#endif /* RFAL_FEATURE_NFCA */
            st25r95SendTransmitFlag(gRFAL.protocol, transmitFlag);
#if RFAL_FEATURE_LISTEN_MODE
            if (gRFAL.protocol == ST25R95_PROTOCOL_CE_ISO14443A)
            {
                st25r95RearmListen();
            }
#endif /* RFAL_FEATURE_LISTEN_MODE */
            gRFAL.TxRx.state = RFAL_TXRX_STATE_TX_DONE;
            /* fall through */

        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_DONE:
            /* If no rxBuf is provided do not wait/expect Rx */
            if (gRFAL.TxRx.ctx.rxBuf == NULL)
            {                
                /* Clean up Transceive */
                //rfalCleanupTransceive();                   
                gRFAL.TxRx.status = ERR_NONE;
                gRFAL.TxRx.state  = RFAL_TXRX_STATE_IDLE;
                break;
            }
            /* Goto Rx */
            gRFAL.TxRx.state  =  RFAL_TXRX_STATE_RX_IDLE;
            break;

        /*******************************************************************************/
        case RFAL_TXRX_STATE_TX_FAIL:
            /* Error should be assigned by previous state */
            if (gRFAL.TxRx.status == ERR_BUSY)
            {
                gRFAL.TxRx.status = ERR_SYSTEM;
            }
            gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
            break;
        
        /*******************************************************************************/
        default:
            gRFAL.TxRx.status = ERR_SYSTEM;
            gRFAL.TxRx.state  = RFAL_TXRX_STATE_TX_FAIL;
            break;
    }
}

/*******************************************************************************/
static void rfalTransceiveRx(void)
{
    ReturnCode retCode;
    
    if (gRFAL.TxRx.state != gRFAL.TxRx.lastState)
    {        
        /*rfalLogD("%s: lastSt: %d curSt: %d \r\n", __FUNCTION__, gRFAL.TxRx.lastState, gRFAL.TxRx.state);*/
        gRFAL.TxRx.lastState = gRFAL.TxRx.state;
    }
    
    switch (gRFAL.TxRx.state)
    {
        /*******************************************************************************/
        case RFAL_TXRX_STATE_RX_IDLE:
            
            /* Clear rx counters */
            if (gRFAL.TxRx.ctx.rxRcvdLen)  {*gRFAL.TxRx.ctx.rxRcvdLen = 0;}
            
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_WAIT_RXE;
            /* fall through */
           
        /*******************************************************************************/    
        case RFAL_TXRX_STATE_RX_WAIT_RXE:
            if (st25r95PollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ERR_TIMEOUT)
            {
                break;
            }
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_READ_DATA;
            /* fall through */
            
        /*******************************************************************************/    
        case RFAL_TXRX_STATE_RX_READ_DATA:            
            retCode = st25r95CompleteRx();
            /* Re-Start FDTPoll SW timer */
            rfalTimerStart( gRFAL.tmr.FDTPoll, (RFAL_ST25R95_SW_TMR_MIN_1MS + rfalConv1fcToMs(gRFAL.timings.FDTPoll)) );

            if (gRFAL.TxRx.ctx.rxRcvdLen != NULL)
            {
                (*gRFAL.TxRx.ctx.rxRcvdLen) = rfalConvBytesToBits(*gRFAL.TxRx.ctx.rxRcvdLen);
        
                /*******************************************************************************/
                /* In case of Incomplete byte append the residual bits                         */
                /*******************************************************************************/
                if( (retCode >= ERR_INCOMPLETE_BYTE_01) && (retCode <= ERR_INCOMPLETE_BYTE_07) )
                {
                    (*gRFAL.TxRx.ctx.rxRcvdLen) += (retCode - ERR_INCOMPLETE_BYTE);
                    
                    if( (*gRFAL.TxRx.ctx.rxRcvdLen) > 0)
                    {
                       (*gRFAL.TxRx.ctx.rxRcvdLen) -= RFAL_BITS_IN_BYTE;
                    }
                    
                    retCode = ERR_INCOMPLETE_BYTE;
                }
            }
            
            
            /*******************************************************************************/
            /* Execute Post Transceive Callback                                            */
            /*******************************************************************************/
            if( gRFAL.callbacks.postTxRx != NULL )
            {
                gRFAL.callbacks.postTxRx();
            }
            
            if (retCode != ERR_NONE)
            {
                gRFAL.TxRx.status = retCode;
                gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
                break;
            }
            gRFAL.TxRx.state = RFAL_TXRX_STATE_RX_DONE;
            /* fall through */
                                       
        /*******************************************************************************/
        case RFAL_TXRX_STATE_RX_DONE:
            gRFAL.TxRx.status = ERR_NONE;
            gRFAL.TxRx.state  = RFAL_TXRX_STATE_IDLE;
            break;
            
            
        /*******************************************************************************/
        case RFAL_TXRX_STATE_RX_FAIL:
            gRFAL.TxRx.state = RFAL_TXRX_STATE_IDLE;
            break;
                
        /*******************************************************************************/
        default:
            gRFAL.TxRx.status = ERR_SYSTEM;
            gRFAL.TxRx.state  = RFAL_TXRX_STATE_RX_FAIL;
            break;           
    }    
}

/*******************************************************************************/
static ReturnCode rfalTransceiveRunBlockingTx(void)
{
    ReturnCode ret;
        
    do 
    {
        rfalWorker();
    }
    while (((ret = rfalGetTransceiveStatus()) == ERR_BUSY) && rfalIsTransceiveInTx());
    
    if (rfalIsTransceiveInRx())
    {
        return ERR_NONE;
    }
    
    return ret;
}

/*******************************************************************************/
static bool rfalChipIsBusy(void)
{
    /* ST25R95 cannot be interrupted while an operation is ongoing */
    
    /* Check whether a Transceive operation is still running */
    if( ( gRFAL.state == RFAL_STATE_TXRX ) && ( gRFAL.TxRx.state > RFAL_TXRX_STATE_TX_IDLE ) )
    {
        return (true);
    }
    
    return (false);
}

#if RFAL_FEATURE_NFCA
/*******************************************************************************/
ReturnCode rfalISO14443ATransceiveShortFrame(rfal14443AShortFrameCmd txCmd, uint8_t* rxBuf, uint8_t rxBufLen, uint16_t* rxRcvdLen, uint32_t fwt)
{
    ReturnCode ret;
    rfalTransceiveContext ctx;
    uint8_t st95hShortFrameBuffer;
    /* Check if RFAL is properly initialized */
    if ((gRFAL.state < RFAL_STATE_MODE_SET) || (( gRFAL.mode != RFAL_MODE_POLL_NFCA ) && ( gRFAL.mode != RFAL_MODE_POLL_NFCA_T1T )) || !gRFAL.field)
    {
        return ERR_WRONG_STATE;
    }
    
    /* Check for valid parameters */
    if( (rxBuf == NULL) || (rxRcvdLen == NULL) || (fwt == RFAL_FWT_NONE) )
    {
        return ERR_PARAM;
    }
    
    gRFAL.NfcaSplitFrame = false;
    /*******************************************************************************/
    /* Update the short frame buffer with the REQA or WUPA command                 */
    st95hShortFrameBuffer =  txCmd;


    ctx.flags     = (RFAL_TXRX_FLAGS_CRC_TX_MANUAL | RFAL_TXRX_FLAGS_CRC_RX_KEEP);
    ctx.txBuf     = &st95hShortFrameBuffer;
    ctx.txBufLen  = 7;
    ctx.rxBuf     = rxBuf;
    ctx.rxBufLen  = rxBufLen;
    ctx.rxRcvdLen = rxRcvdLen;
    ctx.fwt       = fwt;
    
    rfalStartTransceive(&ctx);
    
    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveRunBlockingTx();
    if (ret == ERR_NONE)
    {
        ret = rfalTransceiveBlockingRx();
    }
    
    /* ST25R95 has no means to disable CRC check, discard CRC errors */
    if (ret == ERR_CRC)
    {
        ret = ERR_NONE;
    }
    
    return ret;
}

/*******************************************************************************/
ReturnCode rfalISO14443ATransceiveAnticollisionFrame( uint8_t *buf, uint8_t *bytesToSend, uint8_t *bitsToSend, uint16_t *rxLength, uint32_t fwt )
{
    ReturnCode            ret;
    rfalTransceiveContext ctx;
    uint8_t               collByte;
    
    /* Check if RFAL is properly initialized */
    if( (gRFAL.state < RFAL_STATE_MODE_SET) || ( gRFAL.mode != RFAL_MODE_POLL_NFCA ) )
    {
        return ERR_WRONG_STATE;
    }
    
    /* Check for valid parameters */
    if( (buf == NULL) || (bytesToSend == NULL) || (bitsToSend == NULL) || (rxLength == NULL) )
    {
        return ERR_PARAM;
    }
    
    gRFAL.NfcaSplitFrame = true; 
    
    /*******************************************************************************/
    /* Prepare for Transceive                                                      */
    ctx.flags     = (RFAL_TXRX_FLAGS_CRC_TX_MANUAL | RFAL_TXRX_FLAGS_CRC_RX_KEEP);
    ctx.txBuf     = buf;
    ctx.txBufLen  = (rfalConvBytesToBits(*bytesToSend) + *bitsToSend  );
    ctx.rxBuf     = (buf + (*bytesToSend));
    ctx.rxBufLen  = rfalConvBytesToBits( RFAL_ISO14443A_SDD_RES_LEN );
    ctx.rxRcvdLen = rxLength;
    ctx.fwt       = fwt;
      
    EXIT_ON_ERR( ret, rfalStartTransceive(&ctx) );
      
    /*******************************************************************************/
    collByte = 0;
    
    /* save the collision byte */
    if ((*bitsToSend) > 0)
    {
        buf[(*bytesToSend)] <<= (RFAL_BITS_IN_BYTE - (*bitsToSend));
        buf[(*bytesToSend)] >>= (RFAL_BITS_IN_BYTE - (*bitsToSend));
        collByte = buf[(*bytesToSend)];
    }
    
    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveRunBlockingTx();
    if( ret == ERR_NONE)
    {
        ret = rfalTransceiveBlockingRx();
        /* ignore CRC error */
        if (ret == ERR_CRC)
        {
            ret = ERR_NONE;
        }
    
        /*******************************************************************************/
        if ((*bitsToSend) > 0)
        {
            buf[(*bytesToSend)] >>= (*bitsToSend);
            buf[(*bytesToSend)] <<= (*bitsToSend);
            buf[(*bytesToSend)] |= collByte;
        }
       
        if( (ERR_RF_COLLISION == ret) )
        {
            (*rxLength) = rfalConvBytesToBits(gRFAL.RxInformationBytes[1]);
            (*bytesToSend) = (gRFAL.RxInformationBytes[1] + (*bytesToSend)) & 0xF;
            (*bitsToSend)  = gRFAL.RxInformationBytes[2] & 0x7;
       }
    }
    gRFAL.NfcaSplitFrame = false;     
    return ret;
}

#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCV

/*******************************************************************************/
ReturnCode rfalISO15693TransceiveAnticollisionFrame(uint8_t *txBuf, uint8_t txBufLen, uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
    ReturnCode            ret;
    rfalTransceiveContext ctx;
    
    /* Check if RFAL is properly initialized */
    if ((gRFAL.state < RFAL_STATE_MODE_SET) || (gRFAL.mode != RFAL_MODE_POLL_NFCV))
    {
        return ERR_WRONG_STATE;
    }
    
    /*******************************************************************************/
    /* Prepare for Transceive  */
    ctx.flags     = (((txBufLen==0)?RFAL_TXRX_FLAGS_CRC_TX_MANUAL:RFAL_TXRX_FLAGS_CRC_TX_AUTO) | RFAL_TXRX_FLAGS_CRC_RX_KEEP | RFAL_TXRX_FLAGS_AGC_OFF | ((txBufLen==0)?RFAL_TXRX_FLAGS_NFCV_FLAG_MANUAL:RFAL_TXRX_FLAGS_NFCV_FLAG_AUTO)); /* Disable Automatic Gain Control (AGC) for better detection of collision */
    ctx.txBuf     = txBuf;
    ctx.txBufLen  = rfalConvBytesToBits(txBufLen);
    ctx.rxBuf     = rxBuf;
    ctx.rxBufLen  = rfalConvBytesToBits(rxBufLen);
    ctx.rxRcvdLen = actLen;
    ctx.fwt       = RFAL_FWT_NONE;
    
    EXIT_ON_ERR( ret, rfalStartTransceive(&ctx) );
    
    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveRunBlockingTx();
    if (ret == ERR_NONE)
    {
        ret = rfalTransceiveBlockingRx();
    }

    return ret;
} 

/*******************************************************************************/
ReturnCode rfalISO15693TransceiveEOFAnticollision(uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen)
{
    uint8_t dummy;

    return rfalISO15693TransceiveAnticollisionFrame(&dummy, 0, rxBuf, rxBufLen, actLen);
}

/*******************************************************************************/
ReturnCode rfalISO15693TransceiveEOF( uint8_t *rxBuf, uint8_t rxBufLen, uint16_t *actLen )
{
    ReturnCode ret;
    uint8_t    dummy;
    
    /* Check if RFAL is properly initialized */
    if( ( gRFAL.state < RFAL_STATE_MODE_SET ) || ( gRFAL.mode != RFAL_MODE_POLL_NFCV ) )
    {
        return ERR_WRONG_STATE;
    }
    
    /*******************************************************************************/
    /* Run Transceive blocking */
    ret = rfalTransceiveBlockingTxRx( &dummy,
                                      0,
                                      rxBuf,
                                      rxBufLen,
                                      actLen,
                                      ( (uint32_t)RFAL_TXRX_FLAGS_CRC_TX_MANUAL | (uint32_t)RFAL_TXRX_FLAGS_CRC_RX_KEEP |(uint32_t)RFAL_TXRX_FLAGS_AGC_ON ),
                                      rfalConv64fcTo1fc(RFAL_FWT_NONE) );
    return ret;
} 
   
#endif  /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_NFCF

/*******************************************************************************/
ReturnCode rfalFeliCaPoll(rfalFeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, rfalFeliCaPollRes* pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected)
{
    ReturnCode ret;

    EXIT_ON_ERR( ret, rfalStartFeliCaPoll( slots, sysCode, reqCode, pollResList, pollResListSize, devicesDetected, collisionsDetected ) );
    rfalRunBlocking( ret, rfalGetFeliCaPollStatus() );
    
    return ret;
}


/*******************************************************************************/
ReturnCode rfalStartFeliCaPoll( rfalFeliCaPollSlots slots, uint16_t sysCode, uint8_t reqCode, rfalFeliCaPollRes* pollResList, uint8_t pollResListSize, uint8_t *devicesDetected, uint8_t *collisionsDetected )
{
    ReturnCode        ret;
    uint8_t           frame[RFAL_FELICA_POLL_REQ_LEN - RFAL_FELICA_LEN_LEN];  // LEN is added automatically
    uint8_t           frameIdx;
    
    /* Check if RFAL is properly initialized */
    if( (gRFAL.state < RFAL_STATE_MODE_SET) || ( gRFAL.mode != RFAL_MODE_POLL_NFCF ) )
    {
        return ERR_WRONG_STATE;
    }
    
    frameIdx = 0;
    
    /*******************************************************************************/
    /* Compute SENSF_REQ frame */
    frame[frameIdx++] =  FELICA_CMD_POLLING;       /* CMD: SENF_REQ                       */
    frame[frameIdx++] = (uint8_t)(sysCode >> 8);   /* System Code (SC)                    */
    frame[frameIdx++] = (uint8_t)(sysCode & 0xFF); /* System Code (SC)                    */
    frame[frameIdx++] = reqCode;                   /* Communication Parameter Request (RC)*/
    frame[frameIdx++] = (uint8_t)slots;            /* TimeSlot (TSN)                      */
    
    st25r95SetSlotCounter((uint8_t)slots);
        
    /*******************************************************************************/
    /* Run transceive blocking, 
     * Calculate Total Response Time in(64/fc): 
     *                       512 PICC process time + (n * 256 Time Slot duration)  */
    EXIT_ON_ERR( ret, rfalTransceiveBlockingTx(
                        frame, 
                        frameIdx, 
                        ((uint8_t*)gRFAL.nfcfData.pollResponses),
                        RFAL_FELICA_POLL_RES_LEN, 
                        &gRFAL.nfcfData.actLen,
                        RFAL_TXRX_FLAGS_CRC_RX_REMV,
                        rfalConv64fcTo1fc(RFAL_FELICA_POLL_DELAY_TIME + (RFAL_FELICA_POLL_SLOT_TIME * ((uint32_t)slots + 1U)))) );
                                    
   /* Store context */
   gRFAL.nfcfData.pollResList        = pollResList;
   gRFAL.nfcfData.pollResListSize    = pollResListSize;
   gRFAL.nfcfData.devicesDetected    = devicesDetected;
   gRFAL.nfcfData.collisionsDetected = collisionsDetected;
   
   return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalGetFeliCaPollStatus( void )
{
    ReturnCode ret;
    uint8_t    devDetected;
    uint8_t    colDetected;
    
    /* Check if RFAL is properly initialized */
    if( (gRFAL.state != RFAL_STATE_TXRX) || ( gRFAL.mode != RFAL_MODE_POLL_NFCF ) )
    {
        return ERR_WRONG_STATE;
    }
    
    devDetected = 0;
    colDetected = 0;
    
    ret = rfalGetTransceiveStatus();
    
    /* Wait until transceive has terminated */
    if( ret == ERR_BUSY )
    {
        return ret;
    }
    
    if( ret != ERR_TIMEOUT )
    {
        /* If the reception was OK, new device found */
        if( ret == ERR_NONE )
        {
            devDetected++;
        }
        /* If the reception was not OK, mark as collision */
        else
        {
            colDetected++;
        }                
    }
    st25r95SetSlotCounter( (uint8_t)RFAL_FELICA_1_SLOT );
    
    
    /*******************************************************************************/
    /* Assign output parameters if requested                                       */
    if( (gRFAL.nfcfData.pollResList != NULL) && (gRFAL.nfcfData.pollResListSize > 0U) && (devDetected > 0U) )
    {
        ST_MEMCPY( gRFAL.nfcfData.pollResList, gRFAL.nfcfData.pollResponses, (RFAL_FELICA_POLL_RES_LEN * (uint32_t)MIN(gRFAL.nfcfData.pollResListSize, devDetected) ) );
    }
    
    if( gRFAL.nfcfData.devicesDetected != NULL )
    {
        *gRFAL.nfcfData.devicesDetected = devDetected;
    }
    
    if( gRFAL.nfcfData.collisionsDetected != NULL )
    {
        *gRFAL.nfcfData.collisionsDetected = colDetected;
    }
    
    return (( (colDetected != 0U) || (devDetected != 0U)) ? ERR_NONE : ret);
}

#endif /* RFAL_FEATURE_NFCF */




/*****************************************************************************
 *  Listen Mode                                                              *  
 *****************************************************************************/

/*******************************************************************************/
bool rfalIsExtFieldOn(void)
{
    /* Ensure that no previous operation is still ongoing */
    if( !rfalChipIsBusy() )
    {        
    #if RFAL_FEATURE_LISTEN_MODE
        return (st25r95PollField());
    #endif /* RFAL_FEATURE_LISTEN_MODE */
    }
    
    return (false);
}

#if RFAL_FEATURE_LISTEN_MODE

/*******************************************************************************/
ReturnCode rfalListenStart(uint32_t lmMask, const rfalLmConfPA *confA, const rfalLmConfPB *confB, const rfalLmConfPF *confF, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen)
{
    NO_WARNING(confB);
    NO_WARNING(confF);
    
    /* Check if RFAL is not initialized */
    if (gRFAL.state == RFAL_STATE_IDLE)
    {
        return ERR_WRONG_STATE;
    }
    
    gRFAL.cardEmulT4AT = false;
    
    /*******************************************************************************/
    /* Check whether a Transceive operation is still ongoing                       *
     * ST25R95 cannot be interrupted while a Transceive is ongoing, reject         */
    if (rfalChipIsBusy())
    {        
        return ERR_REQUEST;
    }
    
    /*******************************************************************************/
    if ((lmMask & RFAL_LM_MASK_ACTIVE_P2P) || (lmMask & RFAL_LM_MASK_NFCB) || (lmMask & RFAL_LM_MASK_NFCF))
    {
        return ERR_NOTSUPP;
    }
    
    if ((lmMask & RFAL_LM_MASK_NFCA))
    {
        if (confA == NULL) 
        {
            return (ERR_PARAM);
        }
        
        rfalSetMode(RFAL_MODE_LISTEN_NFCA, RFAL_BR_106, RFAL_BR_106);
            
        if (st25r95SetACFilter(confA) != ERR_NONE) 
        {
            return (ERR_PARAM);
        }
        
        gRFAL.Lm.rxBuf    = rxBuf;
        gRFAL.Lm.rxBufLen = rfalConvBytesToBits(rxBufLen);
        gRFAL.Lm.rxLen    = rxLen;
        *gRFAL.Lm.rxLen   = 0;
        gRFAL.Lm.dataFlag = false;
        gRFAL.state       = RFAL_STATE_LM;
        
        return ERR_NONE;
    }

    return ERR_NOTSUPP;

}

/*******************************************************************************/
static ReturnCode rfalRunListenModeWorker(void)
{
    ReturnCode retCode = ERR_NONE;
    
    if (!st25r95IsInListen())
    {
        retCode = st25r95Listen();
    }
    
    if (retCode != ERR_NONE)
    {
        return (retCode);
    }
    
    if (st25r95PollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) == ERR_TIMEOUT)
    {
        return (ERR_NONE);
    }
    
    st25r95PrepareRx(
        gRFAL.protocol, 
        gRFAL.Lm.rxBuf, 
        rfalConvBitsToBytes(gRFAL.Lm.rxBufLen), 
        gRFAL.Lm.rxLen, 
        (gRFAL.TxRx.ctx.flags & RFAL_TXRX_FLAGS_CRC_RX_KEEP) != RFAL_TXRX_FLAGS_CRC_RX_KEEP, 
        gRFAL.RxInformationBytes
        );
     retCode = st25r95CompleteRx();
     if (!((retCode == ERR_LINK_LOSS) || ((retCode == ERR_NONE) && (gRFAL.Lm.rxLen == 0))))
     {
        *gRFAL.Lm.rxLen   = rfalConvBytesToBits( *gRFAL.Lm.rxLen );
        gRFAL.Lm.dataFlag = true;
        gRFAL.state       = RFAL_STATE_MODE_SET;
    }
    
    return (retCode);
}

/*******************************************************************************/
ReturnCode rfalListenStop( void )
{   
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    /* Check if RFAL is initialized */
    if( gRFAL.state < RFAL_STATE_INIT )
    {
        return ERR_WRONG_STATE;
    }
    
    st25r95CommandEcho(); /* kill listen command */
    st25r95FieldOff();
    gRFAL.state              = RFAL_STATE_INIT;
    gRFAL.mode               = RFAL_MODE_NONE;
    gRFAL.protocol           = ST25R95_PROTOCOL_FIELDOFF;
    gRFAL.field              = false;
    gRFAL.Lm.dataFlag        = false;
    
    return ERR_NONE;
}


/*******************************************************************************/
ReturnCode rfalListenSleepStart( rfalLmState sleepSt, uint8_t *rxBuf, uint16_t rxBufLen, uint16_t *rxLen )
{
    ReturnCode retCode = ERR_PARAM;
    
    if (sleepSt == RFAL_LM_STATE_SLEEP_A)
    {
        gRFAL.state       = RFAL_STATE_LM;
        gRFAL.Lm.rxBuf    = rxBuf;
        gRFAL.Lm.rxBufLen = rfalConvBytesToBits(rxBufLen);
        gRFAL.Lm.rxLen    = rxLen;
        *gRFAL.Lm.rxLen   = 0;
        gRFAL.Lm.dataFlag = false;    
        rfalListenSetState(sleepSt);
        retCode = ERR_NONE;
    }
    
    return retCode;
}


/*******************************************************************************/
rfalLmState rfalListenGetState(bool *dataFlag, rfalBitRate *lastBR)
{
    rfalLmState state;
    
    /* Allow state retrieval even if gRFAL.state != RFAL_STATE_LM so  *
     * that this Lm state can be used by caller after activation      */

    if( lastBR != NULL )
    {
        *lastBR = gRFAL.txBR;
    }
    
    if( dataFlag != NULL )
    {
        *dataFlag = gRFAL.Lm.dataFlag;
    }
    state = st25r95GetLmState();
    
    if( ((state == RFAL_LM_STATE_ACTIVE_A) || (state == RFAL_LM_STATE_ACTIVE_Ax)) && gRFAL.cardEmulT4AT )
    {
        state = RFAL_LM_STATE_CARDEMU_4A;        
    }
    
    return (state);
}


/*******************************************************************************/
ReturnCode rfalListenSetState( rfalLmState newSt )
{
    ReturnCode retCode = ERR_NONE;
    uint8_t st25r95State;
    bool WasInListen;
    
    
    /* Check if RFAL is initialized */
    if( gRFAL.state < RFAL_STATE_INIT )
    {
        return ERR_WRONG_STATE;
    }
    
    
    WasInListen = st25r95IsInListen();
    st25r95CommandEcho(); /* kill listen command */
    gRFAL.cardEmulT4AT = false;
    
    switch (newSt)
    {
        default:
        case RFAL_LM_STATE_NOT_INIT:
        case RFAL_LM_STATE_POWER_OFF:
        case RFAL_LM_STATE_READY_B:
        case RFAL_LM_STATE_READY_F:
        case RFAL_LM_STATE_CARDEMU_4B:
        case RFAL_LM_STATE_CARDEMU_3:
        case RFAL_LM_STATE_TARGET_A:
        case RFAL_LM_STATE_TARGET_F:
        case RFAL_LM_STATE_SLEEP_B:
        case RFAL_LM_STATE_SLEEP_AF:
            retCode = ERR_PARAM;
            break;
            
        case RFAL_LM_STATE_IDLE:
            st25r95State = ST25R95_ACSTATE_IDLE;
            break;

        case RFAL_LM_STATE_READY_A:
            st25r95State = ST25R95_ACSTATE_READYA;
            break;
            
        case RFAL_LM_STATE_ACTIVE_A:
            st25r95State = ST25R95_ACSTATE_ACTIVE;
            break;

        case RFAL_LM_STATE_SLEEP_A:
            st25r95State = ST25R95_ACSTATE_HALT;
            break;                
            
        case RFAL_LM_STATE_READY_Ax:
            st25r95State = ST25R95_ACSTATE_READYAX;
            break;
            
        case RFAL_LM_STATE_ACTIVE_Ax:
            st25r95State = ST25R95_ACSTATE_ACTIVEX;
            break;
            
        case RFAL_LM_STATE_CARDEMU_4A:
            st25r95State = st25r95GetLmState();
            if( (st25r95State != ST25R95_ACSTATE_ACTIVE) || (st25r95State != ST25R95_ACSTATE_ACTIVEX) )
            {
                st25r95State = ST25R95_ACSTATE_ACTIVE;
            }
            gRFAL.cardEmulT4AT = true;
            break;
    }
    if (retCode == ERR_NONE)
    {
        st25r95SetACState(st25r95State);
    }
    if (WasInListen)
    {
        st25r95Listen();
    }
    
    return (retCode);
}
#endif /* RFAL_FEATURE_LISTEN_MODE */


/*******************************************************************************
 *  Wake-Up Mode                                                               *
 *******************************************************************************/

#if RFAL_FEATURE_WAKEUP_MODE

/*******************************************************************************/
ReturnCode rfalWakeUpModeStart( const rfalWakeUpConfig *config )
{
    /* Check if RFAL is not initialized */
    if (gRFAL.state == RFAL_STATE_IDLE)
    {
        return ERR_WRONG_STATE;
    }
    
    if( config == NULL )
    {
        gRFAL.wum.cfg.period           = RFAL_WUM_PERIOD_300MS;

        gRFAL.wum.cfg.indAmp.enabled   = true;
        gRFAL.wum.cfg.indAmp.delta     = 8U;
        gRFAL.wum.cfg.indAmp.reference = RFAL_WUM_REFERENCE_AUTO;
    }
    else
    {
        gRFAL.wum.cfg = *config;
    }
    
    /* Check for valid configuration */
    if( !gRFAL.wum.cfg.indAmp.enabled )
    {
        return ERR_PARAM;
    }
    
    if (gRFAL.wum.cfg.indAmp.reference == RFAL_WUM_REFERENCE_AUTO)
    {
        gRFAL.wum.cfg.indAmp.reference = gRFAL.wum.CalTagDet;
    }
    if ((gRFAL.wum.cfg.indAmp.delta > gRFAL.wum.cfg.indAmp.reference) || ((((uint32_t)gRFAL.wum.cfg.indAmp.delta) + ((uint32_t)gRFAL.wum.cfg.indAmp.reference)) > 0xFCUL))
    {
        return ERR_PARAM;
    }

    /* Use a fixed period of ~300 ms */
    st25r95Idle(gRFAL.wum.cfg.indAmp.reference - gRFAL.wum.cfg.indAmp.delta, gRFAL.wum.cfg.indAmp.reference + gRFAL.wum.cfg.indAmp.delta, RFAL_ST25R95_IDLE_DEFAULT_WUPERIOD);
    gRFAL.state     = RFAL_STATE_WUM;
    gRFAL.wum.state = RFAL_WUM_STATE_ENABLED;
    return ERR_NONE;
}


/*******************************************************************************/
bool rfalWakeUpModeHasWoke( void )
{   
    return (gRFAL.wum.state >= RFAL_WUM_STATE_ENABLED_WOKE);
}


/*******************************************************************************/
ReturnCode rfalWakeUpModeGetInfo( bool force, rfalWakeUpInfo *info )
{
    NO_WARNING(info);
    NO_WARNING(force);
    
    return ERR_NOTSUPP;
}


/*******************************************************************************/
ReturnCode rfalWakeUpModeStop( void )
{
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    /* Check if RFAL is in Wake-up mode */
    if( gRFAL.state != RFAL_STATE_WUM )
    {
        return ERR_WRONG_STATE;
    }
    
    gRFAL.wum.state = RFAL_WUM_STATE_NOT_INIT;
    st25r95KillIdle();
    st25r95CommandEcho();
    return ERR_NONE;
}


/*******************************************************************************/
static void rfalRunWakeUpModeWorker( void )
{   
    if( gRFAL.state != RFAL_STATE_WUM )
    {
        return;
    }
    
    switch( gRFAL.wum.state )
    {
        case RFAL_WUM_STATE_ENABLED:
        case RFAL_WUM_STATE_ENABLED_WOKE:
            if (st25r95PollRead(ST25R95_CONTROL_POLL_NO_TIMEOUT) != ERR_TIMEOUT)
            {
                st25r95GetIdleResponse();
                gRFAL.wum.state = RFAL_WUM_STATE_ENABLED_WOKE;
            }
            
        default:
            break;
    }
}

#endif /* RFAL_FEATURE_WAKEUP_MODE */


/*******************************************************************************
 *  Low-Power Mode                                                               *
 *******************************************************************************/

#if RFAL_FEATURE_LOWPOWER_MODE

/*******************************************************************************/
ReturnCode rfalLowPowerModeStart( void )
{
    /* Check if RFAL is not initialized */
    if( gRFAL.state < RFAL_STATE_INIT )
    {
        return ERR_WRONG_STATE;
    }
    
    return ERR_NOTSUPP;
}


/*******************************************************************************/
ReturnCode rfalLowPowerModeStop( void )
{
    return ERR_NOTSUPP;
}

#endif /* RFAL_FEATURE_LOWPOWER_MODE */



/*******************************************************************************
 *  RF Chip                                                                    *
 *******************************************************************************/

/*******************************************************************************/
ReturnCode rfalChipWriteReg(uint16_t reg, const uint8_t* values, uint8_t len)
{
    ReturnCode retCode;
    
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    
    if (len != 1U)
    {
        retCode = ERR_PARAM;
    }
    else
    {
        retCode = st25r95WriteReg(gRFAL.protocol, reg, values[0]);
    } 
    return (retCode);
}

/*******************************************************************************/
ReturnCode rfalChipReadReg( uint16_t reg, uint8_t* values, uint8_t len )
{
    ReturnCode retCode;
    
    /* Ensure that no previous operation is still ongoing */
    if( rfalChipIsBusy() )
    {        
        return ERR_REQUEST;
    }
    
    
    if (len != 1U)
    {
        retCode = ERR_PARAM;
    }
    else
    {
        retCode = st25r95ReadReg(reg, values);
    } 
    return (retCode);
}

/*******************************************************************************/
ReturnCode rfalChipExecCmd( uint16_t cmd )
{
    NO_WARNING(cmd);
        
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipWriteTestReg( uint16_t reg, uint8_t value )
{
    NO_WARNING(reg);
    NO_WARNING(value);
    
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipReadTestReg( uint16_t reg, uint8_t* value )
{
    NO_WARNING(reg);
    NO_WARNING(value);
        
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipChangeRegBits( uint16_t reg, uint8_t valueMask, uint8_t value )
{
    ReturnCode retCode;
    uint8_t tmp;
    
    retCode = st25r95ReadReg(reg, &tmp);
    
    if (retCode == ERR_NONE)
    {
        /* mask out the bits we don't want to change */
        tmp &= (uint8_t)(~((uint32_t)valueMask));
        /* set the new value */
        tmp |= (value & valueMask);
        retCode = st25r95WriteReg(gRFAL.protocol, reg, tmp);
    }
        
    return retCode;
}

/*******************************************************************************/
ReturnCode rfalChipChangeTestRegBits( uint16_t reg, uint8_t valueMask, uint8_t value )
{
    NO_WARNING(reg);
    NO_WARNING(valueMask);
    NO_WARNING(value);
    
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipSetRFO( uint8_t rfo )
{
    NO_WARNING(rfo);
    
    return ERR_NOTSUPP;
}


/*******************************************************************************/
ReturnCode rfalChipGetRFO( uint8_t* result )
{
    NO_WARNING(result);

    return ERR_NOTSUPP;
}


/*******************************************************************************/
ReturnCode rfalChipMeasureAmplitude( uint8_t* result )
{
    NO_WARNING(result);
    
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipMeasurePhase( uint8_t* result )
{
    NO_WARNING(result);
    
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipMeasureCapacitance( uint8_t* result )
{
    NO_WARNING(result);
    
    return ERR_NOTSUPP;
}

/*******************************************************************************/
ReturnCode rfalChipMeasurePowerSupply( uint8_t param, uint8_t* result )
{
    NO_WARNING(param);
    NO_WARNING(result);
    
    return ERR_NOTSUPP;
}
