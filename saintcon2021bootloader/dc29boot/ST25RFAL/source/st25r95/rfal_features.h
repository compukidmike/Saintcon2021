
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

/*! \file
 *
 *  \author  
 *
 *  \brief RFAL Features/Capabilities Definition for ST25R95
 */


#ifndef RFAL_FEATURES_H
#define RFAL_FEATURES_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

#define RFAL_SUPPORT_MODE_POLL_NFCA                true          /*!< RFAL Poll NFCA mode support switch    */
#define RFAL_SUPPORT_MODE_POLL_NFCB                true          /*!< RFAL Poll NFCB mode support switch    */    
#define RFAL_SUPPORT_MODE_POLL_NFCF                true          /*!< RFAL Poll NFCF mode support switch    */
#define RFAL_SUPPORT_MODE_POLL_NFCV                true          /*!< RFAL Poll NFCV mode support switch    */
#define RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P          false         /*!< RFAL Poll AP2P mode support switch    */
#define RFAL_SUPPORT_MODE_LISTEN_NFCA              false         /*!< RFAL Listen NFCA mode support switch  */
#define RFAL_SUPPORT_MODE_LISTEN_NFCB              false         /*!< RFAL Listen NFCB mode support switch  */
#define RFAL_SUPPORT_MODE_LISTEN_NFCF              false         /*!< RFAL Listen NFCF mode support switch  */
#define RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P        false         /*!< RFAL Listen AP2P mode support switch  */

/*******************************************************************************/
/*! RFAL supported Card Emulation (CE)        */
#define RFAL_SUPPORT_CE                            ( RFAL_SUPPORT_MODE_LISTEN_NFCA || RFAL_SUPPORT_MODE_LISTEN_NFCB || RFAL_SUPPORT_MODE_LISTEN_NFCF )

/*! RFAL supported Reader/Writer (RW)         */
#define RFAL_SUPPORT_RW                            ( RFAL_SUPPORT_MODE_POLL_NFCA || RFAL_SUPPORT_MODE_POLL_NFCB || RFAL_SUPPORT_MODE_POLL_NFCF || RFAL_SUPPORT_MODE_POLL_NFCV )

/*! RFAL support for Active P2P (AP2P)        */
#define RFAL_SUPPORT_AP2P                          ( RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P || RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P )


/*******************************************************************************/
#define RFAL_SUPPORT_BR_RW_106                      true         /*!< RFAL RW  106 Bit Rate support switch   */
#define RFAL_SUPPORT_BR_RW_212                      true         /*!< RFAL RW  212 Bit Rate support switch   */
#define RFAL_SUPPORT_BR_RW_424                      true         /*!< RFAL RW  424 Bit Rate support switch   */
#define RFAL_SUPPORT_BR_RW_848                      true         /*!< RFAL RW  848 Bit Rate support switch   */
#define RFAL_SUPPORT_BR_RW_1695                     false        /*!< RFAL RW 1695 Bit Rate support switch   */
#define RFAL_SUPPORT_BR_RW_3390                     false        /*!< RFAL RW 3390 Bit Rate support switch   */
#define RFAL_SUPPORT_BR_RW_6780                     false        /*!< RFAL RW 6780 Bit Rate support switch   */


/*******************************************************************************/
#define RFAL_SUPPORT_BR_AP2P_106                    false        /*!< RFAL AP2P  106 Bit Rate support switch */
#define RFAL_SUPPORT_BR_AP2P_212                    false        /*!< RFAL AP2P  212 Bit Rate support switch */
#define RFAL_SUPPORT_BR_AP2P_424                    false        /*!< RFAL AP2P  424 Bit Rate support switch */
#define RFAL_SUPPORT_BR_AP2P_848                    false        /*!< RFAL AP2P  848 Bit Rate support switch */
#define RFAL_SUPPORT_BR_AP2P_1695                   false        /*!< RFAL AP2P 1695 Bit Rate support switch */
#define RFAL_SUPPORT_BR_AP2P_3390                   false        /*!< RFAL AP2P 3390 Bit Rate support switch */
#define RFAL_SUPPORT_BR_AP2P_6780                   false        /*!< RFAL AP2P 6780 Bit Rate support switch */


/*******************************************************************************/
#define RFAL_SUPPORT_BR_CE_A_106                    true         /*!< RFAL CE A 106 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_A_212                    false        /*!< RFAL CE A 212 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_A_424                    false        /*!< RFAL CE A 424 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_A_848                    false        /*!< RFAL CE A 848 Bit Rate support switch  */


/*******************************************************************************/
#define RFAL_SUPPORT_BR_CE_B_106                    false        /*!< RFAL CE B 106 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_B_212                    false        /*!< RFAL CE B 212 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_B_424                    false        /*!< RFAL CE B 424 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_B_848                    false        /*!< RFAL CE B 848 Bit Rate support switch  */


/*******************************************************************************/
#define RFAL_SUPPORT_BR_CE_F_212                    false        /*!< RFAL CE F 212 Bit Rate support switch  */
#define RFAL_SUPPORT_BR_CE_F_424                    false        /*!< RFAL CE F 424 Bit Rate support switch  */


/*
******************************************************************************
* DEVICE SPECIFIC FEATURE DEFINITIONS
******************************************************************************
*/

/*! RFAL Wake-Up Period/Timer */
typedef enum 
{
    RFAL_WUM_PERIOD_300MS     = 0x12,     /*!< Wake-Up timer 300ms                                        */
} rfalWumPeriod;


/*! RFAL Wake-Up Mode configuration */
typedef struct 
{
    rfalWumPeriod        period;          /*!< Wake-Up Timer period;how often measurement(s) is performed */
    
    struct{                               
        bool             enabled;         /*!< Inductive Amplitude measurement enabled                    */
        uint8_t          delta;           /*!< Delta between the reference and measurement to wake-up     */
        uint16_t         reference;       /*!< Reference to be used;RFAL_WUM_REFERENCE_AUTO sets it auto  */
    }indAmp;                              /*!< Inductive Amplitude Configuration                          */
} rfalWakeUpConfig;

/*! RFAL Wake-Up Mode information */
typedef struct {
    uint8_t dummy;                        /*!< dummy member to avoid empty structs                         */
}rfalWakeUpInfo;

#endif /* RFAL_FEATURES_H */
