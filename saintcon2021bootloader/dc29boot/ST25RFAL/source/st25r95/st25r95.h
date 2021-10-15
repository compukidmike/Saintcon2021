
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

/*! \file st25r95.h
 *
 *  \author 
 *
 *  \brief ST25R95 declaration file
 *
 * API:
 * - Initialize ST25R95 driver: #st25r95Initialize
 * - Deinitialize ST25R95 driver: #st25r95Deinitialize
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
 * @addtogroup ST25R95_Driver
 * @brief RFAL ST25R95 Driver
 * @{
 * 
 */

#ifndef ST25R95_H
#define ST25R95_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"
#include "st_errno.h"

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#if !(ST25R95_INTERFACE_UART) /* ST25R95_INTERFACE_SPI */ 
#define st25r95_nIRQ_IN_Pulse()         st25r95SPI_nIRQ_IN_Pulse()  /*!< UART/SPI wrapper for st25r95_nIRQ_IN_Pulse */
#define st25r95ResetChip()              st25r95SPIResetChip()       /*!< UART/SPI wrapper for st25r95ResetChip      */
#else /* !ST25R95_INTERFACE_SPI */
#define st25r95_nIRQ_IN_Pulse()         st25r95UART_nIRQ_IN_Pulse() /*!< UART/SPI wrapper for st25r95_nIRQ_IN_Pulse */
#define st25r95ResetChip()                                          /*!< UART/SPI wrapper for st25r95ResetChip      */
#endif /* ST25R95_INTERFACE_SPI */

/*
******************************************************************************
* GLOBAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*! 
 *****************************************************************************
 *  \brief  Initialise ST25R95 driver
 *
 *  This function initialises the ST25R95 driver.
 *
 *  \return ERR_NONE    : Operation successful
 *  \return Other       : Init failed
 *
 *****************************************************************************
 */
extern ReturnCode st25r95Initialize(void);

/*! 
 *****************************************************************************
 *  \brief  Deinitialize ST25R95 driver
 *
 *  Calling this function deinitializes the ST25R95 driver.
 *
 *****************************************************************************
 */
extern void st25r95Deinitialize(void);

/*! 
 *****************************************************************************
 *  \brief  Generates an IRQ_IN pulse (SPI intefrace)
 *
 *  Generates an IRQ_IN pulse (see CR95HF DS §3.2).
 *
 *  
 *****************************************************************************
 */
extern void st25r95SPI_nIRQ_IN_Pulse(void);

/*! 
 *****************************************************************************
 *  \brief  Generates an IRQ_IN pulse (UART interface)
 *
 *  Generates an IRQ_IN pulse (see CR95HF DS §3.2).
 *
 *  
 *****************************************************************************
 */
extern void st25r95UART_nIRQ_IN_Pulse(void);

/*! 
 *****************************************************************************
 *  \brief  Check Identity
 *
 *  Checks if the chip ID is as expected.
 *  
 *  \return  true when IC type is as expected
 *  
 *****************************************************************************
 */
extern bool st25r95CheckChipID(void);

/*! 
 *****************************************************************************
 *  \brief  Turns field on
 *
 *  This function turns field On
 *
 *  \param[in]   protocol: ST25R95_PROTOCOL_xxx protocol value.
 * 
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95FieldOn(uint32_t protocol);

/*! 
 *****************************************************************************
 *  \brief  Turns field off
 *
 *  This function turns field Off
 *
 *  \return ERR_NONE    : Operation successful
 *
 *****************************************************************************
 */
extern ReturnCode st25r95FieldOff(void);

#endif /* ST25R95_H */

/**
  * @}
  *
  * @}
  *
  * @}
  * 
  * @}
  */
