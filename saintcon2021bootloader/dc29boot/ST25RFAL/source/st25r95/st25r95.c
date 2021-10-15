
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

/*! \file st25r95.c
 *
 *  \author 
 *
 *  \brief ST25R95 high level interface
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "platform.h"
#include "st25r95.h"
#include "st25r95_com.h"
#include "utils.h"

/*
 ******************************************************************************
 * ENABLE SWITCH
 ******************************************************************************
 */
 
#ifndef ST25R95
#error "RFAL: Missing ST25R device selection. Please globally define ST25R95."
#endif /* ST25R95 */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */
 
#if ST25R95_INTERFACE_UART
static uint8_t st25r95UartnIRQINPulse[1] = {0x00};
#endif /* ST25R95_INTERFACE_UART */

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
ReturnCode st25r95Initialize(void)
{
     uint32_t attempt = 5;
     ReturnCode retCode = ERR_NONE;
        
    /* First perform the startup sequence */
    st25r95_nIRQ_IN_Pulse();
    /* Reset ST25R95 */
    st25r95ResetChip();
    /* If no answer from ECHO command, reset and retry again up to max attempt */ 
    while ((st25r95CommandEcho() != ERR_NONE) && (attempt != 0))
    {
        attempt--;
        st25r95ResetChip();
    }
    if (attempt == 0) 
    {
        platformErrorHandle();
        retCode = ERR_SYSTEM;
    }
    
    return (retCode);
}


/*******************************************************************************/
void st25r95Deinitialize(void)
{
    /* Reset ST25R95 */
    st25r95ResetChip();    
}


/*******************************************************************************/
void st25r95SPI_nIRQ_IN_Pulse(void)
{
    platformGpioSet(ST25R95_N_IRQ_IN_PORT, ST25R95_N_IRQ_IN_PIN);
    platformDelay(1); /* wait t0 */
    platformGpioClear(ST25R95_N_IRQ_IN_PORT, ST25R95_N_IRQ_IN_PIN);
    platformDelay(1); /* wait t1 */
    platformGpioSet(ST25R95_N_IRQ_IN_PORT, ST25R95_N_IRQ_IN_PIN); 
    platformDelay(11); /* wait t3: seems more than 10ms needed */
}

/*******************************************************************************/
#if ST25R95_INTERFACE_UART
void st25r95UART_nIRQ_IN_Pulse(void)
{
    /* kill Idle mode if any */
    platformGpioClear(ST25R95_N_SS_PORT, ST25R95_N_SS_PIN);
    platformDelay(5);
    platformGpioSet(ST25R95_N_SS_PORT, ST25R95_N_SS_PIN);
    
    /* Start up sequence */
    platformDelay(1); /* wait t0 */
    /* Send null char to have nIRQ_IN/UART_TX low for more than 10µs (t1) */
    platformUartTx(st25r95UartnIRQINPulse, 1);
    platformDelay(11); /* wait t3: seems more than 10ms needed */
}
#endif /* ST25R95_INTERFACE_UART */

/*******************************************************************************/
#if !(ST25R95_INTERFACE_UART) /* ST25R95_INTERFACE_SPI */
void st25r95SPIResetChip(void)
{
    platformSpiDeselect();
    platformDelay(1);
    platformSpiSelect();
    /* Send Reset Control byte over SPI */
    st25r95SPISendReceiveByte(ST25R95_CONTROL_RESET);
    platformDelay(1); 
    platformSpiDeselect();
    platformDelay(3);
    st25r95_nIRQ_IN_Pulse();
}
#endif /* ST25R95_INTERFACE_SPI */


/*******************************************************************************/
ReturnCode st25r95FieldOn(uint32_t protocol)
{   
    if (protocol == ST25R95_PROTOCOL_FIELDOFF)
    {
        protocol = ST25R95_PROTOCOL_ISO15693;
    }
    return (st25r95ProtocolSelect(protocol));
}

ReturnCode st25r95FieldOff(void)
{   
    return (st25r95ProtocolSelect(ST25R95_PROTOCOL_FIELDOFF));
}
