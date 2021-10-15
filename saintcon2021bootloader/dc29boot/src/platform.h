/*
 * platform.h
 *
 * This file is required by the ST25R95 RFAL libraries
 */ 


#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "main.h"

#define ST25R95

#define ST25R95_N_IRQ_IN_PORT 1
#define ST25R95_N_IRQ_IN_PIN PIN_PB08
#define ST25R95_N_IRQ_OUT_PORT 1
#define ST25R95_N_IRQ_OUT_PIN PIN_PB09

#define RFAL_FEATURE_NFCA                       true                    /*!< Enable/Disable RFAL support for NFC-A (ISO14443A)                         */
#define RFAL_FEATURE_NFCB                       true                    /*!< Enable/Disable RFAL support for NFC-B (ISO14443B)                         */
#define RFAL_FEATURE_NFCF                       true                    /*!< Enable/Disable RFAL support for NFC-F (FeliCa)                            */
#define RFAL_FEATURE_NFCV                       true                    /*!< Enable/Disable RFAL support for NFC-V (ISO15693)                          */
#define RFAL_FEATURE_T1T                        false                    /*!< Enable/Disable RFAL support for T1T (Topaz)                               */
#define RFAL_FEATURE_ST25TB                     false                    /*!< Enable/Disable RFAL support for ST25TB                                    */
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG      false                    /*!< Enable/Disable Analog Configs to be dynamically updated (RAM)             */
#define RFAL_FEATURE_DYNAMIC_POWER              false                   /*!< Enable/Disable RFAL dynamic power support                                 */
#define RFAL_FEATURE_ISO_DEP                    true                    /*!< Enable/Disable RFAL support for ISO-DEP (ISO14443-4)                      */
#define RFAL_FEATURE_NFC_DEP                    true                    /*!< Enable/Disable RFAL support for NFC-DEP (NFCIP1/P2P)                      */

#define RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN     256                     /*!< ISO-DEP I-Block max length. Please use values as defined by rfalIsoDepFSx */
#define RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN       1024                    /*!< ISO-DEP APDU max length. Please use multiples of I-Block max length       */

#define RFAL_FEATURE_NFC_RF_BUF_LEN				258U
#define RFAL_FEATURE_NFC_DEP_BLOCK_MAX_LEN		254
#define RFAL_FEATURE_NFC_DEP_PDU_MAX_LEN		254

#define RFAL_FEATURE_ISO_DEP_POLL				true

//#define ST_MEMCPY memcpy


void platformGpioSet(uint32_t inport, uint32_t inpin);
void platformGpioClear(uint32_t inport, uint32_t inpin);
void platformDelay(uint32_t ms);
void platformSpiSelect(void);
void platformSpiDeselect(void);
void platformErrorHandle(void);
void platformSpiTxRx(uint8_t *txBuf, uint8_t *rxBuf, const uint16_t len);
bool platformGpioIsHigh(uint32_t inport, uint32_t inpin);
bool platformGpioIsLow(uint32_t inport, uint32_t inpin);
uint32_t platformTimerCreate(uint32_t timeout);
bool platformTimerIsExpired(uint32_t timer);
void platformTimerDestroy(uint32_t timer);
void platformProtectWorker(void);
void platformUnprotectWorker(void);
void platformLog(char *data);

#endif /* PLATFORM_H_ */