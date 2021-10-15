/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2011-2014, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following condition is met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#define CPU_FREQUENCY 48000000

#define FLASH_WAIT_STATES 1

#define SAMD51
#undef SAMD21
#include "same53j18a.h"
#include "atmel_start.h"
#include <stdint.h>

struct io_descriptor *io;

int32_t spi_m_sync_io_readwrite(struct io_descriptor *io, uint8_t *rxbuf, uint8_t *txbuf, const uint16_t length);

/* Non-Volatile Memory Controller (NVMC) Parameters */
#define SAMX5X_PAGE_SIZE			512
#define SAMX5X_BLOCK_SIZE			(SAMX5X_PAGE_SIZE * 16)

/* -------------------------------------------------------------------------- */
/* Non-Volatile Memory Controller (NVMC) Registers */
/* -------------------------------------------------------------------------- */

#define SAMX5X_NVMC				0x41004000
#define SAMX5X_NVMC_CTRLA			(SAMX5X_NVMC + 0x0)
#define SAMX5X_NVMC_CTRLB			(SAMX5X_NVMC + 0x04)
#define SAMX5X_NVMC_PARAM			(SAMX5X_NVMC + 0x08)
#define SAMX5X_NVMC_INTFLAG			(SAMX5X_NVMC + 0x10)
#define SAMX5X_NVMC_STATUS			(SAMX5X_NVMC + 0x12)
#define SAMX5X_NVMC_ADDRESS			(SAMX5X_NVMC + 0x14)
#define SAMX5X_NVMC_RUNLOCK			(SAMX5X_NVMC + 0x18)

/* Control B Register (CTRLB) */
#define SAMX5X_CTRLB_CMD_KEY			0xA500
#define SAMX5X_CTRLB_CMD_ERASEPAGE		0x0000
#define SAMX5X_CTRLB_CMD_ERASEBLOCK		0x0001
#define SAMX5X_CTRLB_CMD_WRITEPAGE		0x0003
#define SAMX5X_CTRLB_CMD_WRITEQUADWORD		0x0004
#define SAMX5X_CTRLB_CMD_LOCK			0x0011
#define SAMX5X_CTRLB_CMD_UNLOCK		0x0012
#define SAMX5X_CTRLB_CMD_PAGEBUFFERCLEAR	0x0015
#define SAMX5X_CTRLB_CMD_SSB			0x0016

/* Interrupt Flag Register (INTFLAG) */
#define SAMX5X_INTFLAG_DONE			(1 << 0)
#define SAMX5X_INTFLAG_ADDRE			(1 << 1)
#define SAMX5X_INTFLAG_PROGE			(1 << 2)
#define SAMX5X_INTFLAG_LOCKE			(1 << 3)
#define SAMX5X_INTFLAG_ECCSE			(1 << 4)
#define SAMX5X_INTFLAG_ECCDE			(1 << 5)
#define SAMX5X_INTFLAG_NVME			(1 << 6)
#define SAMX5X_INTFLAG_SUSP			(1 << 7)
#define SAMX5X_INTFLAG_SEESFULL			(1 << 8)
#define SAMX5X_INTFLAG_SEESOVF			(1 << 9)

/* Status Register (STATUS) */
#define SAMX5X_STATUS_READY			(1 << 0)

/* Non-Volatile Memory Calibration and Auxiliary Registers */
#define SAMX5X_NVM_USER_PAGE			0x00804000
#define SAMX5X_NVM_CALIBRATION			0x00800000
#define SAMX5X_NVM_SERIAL(n)			(0x0080600C + \
(n == 0 ? 0x1F0 : n * 4))

#define SAMX5X_USER_PAGE_OFFSET_LOCK		0x08
#define SAMX5X_USER_PAGE_OFFSET_BOOTPROT	0x03
#define SAMX5X_USER_PAGE_MASK_BOOTPROT		0x3C
#define SAMX5X_USER_PAGE_SHIFT_BOOTPROT	2

/* -------------------------------------------------------------------------- */
/* Device Service Unit (DSU) Registers */
/* -------------------------------------------------------------------------- */

#define SAMX5X_DSU				0x41002000
#define SAMX5X_DSU_EXT_ACCESS			(SAMX5X_DSU + 0x100)
#define SAMX5X_DSU_CTRLSTAT			(SAMX5X_DSU_EXT_ACCESS + 0x00)
#define SAMX5X_DSU_ADDRESS			(SAMX5X_DSU_EXT_ACCESS + 0x04)
#define SAMX5X_DSU_LENGTH			(SAMX5X_DSU_EXT_ACCESS + 0x08)
#define SAMX5X_DSU_DATA				(SAMX5X_DSU_EXT_ACCESS + 0x0C)
#define SAMX5X_DSU_DID				(SAMX5X_DSU_EXT_ACCESS + 0x18)
#define SAMX5X_DSU_PID(n)			(SAMX5X_DSU + 0x1FE0 + \
(0x4 * (n % 4)) - \
(0x10 * (n / 4)))
#define SAMX5X_DSU_CID(n)			(SAMX5X_DSU + 0x1FF0 + \
(0x4 * (n % 4)))

/* Control and Status Register (CTRLSTAT) */
#define SAMX5X_CTRL_CHIP_ERASE			(1 << 4)
#define SAMX5X_CTRL_MBIST			(1 << 3)
#define SAMX5X_CTRL_CRC				(1 << 2)
#define SAMX5X_STATUSA_PERR			(1 << 12)
#define SAMX5X_STATUSA_FAIL			(1 << 11)
#define SAMX5X_STATUSA_BERR			(1 << 10)
#define SAMX5X_STATUSA_CRSTEXT			(1 << 9)
#define SAMX5X_STATUSA_DONE			(1 << 8)
#define SAMX5X_STATUSB_PROT			(1 << 16)
/*
#ifndef BOOT_USART_MODULE
#define BOOT_USART_MODULE SERCOM3
#define BOOT_USART_PAD_SETTINGS UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3 PINMUX_UNUSED
#define BOOT_USART_PAD2 PINMUX_UNUSED
#define BOOT_USART_PAD1 PINMUX_PA23C_SERCOM3_PAD1
#define BOOT_USART_PAD0 PINMUX_PA22C_SERCOM3_PAD0
#endif*/


#define NFC_IRQ_IN_PIN PIN_PB08
#define NFC_IRQ_OUT_PIN PIN_PB09
#define NFC_CS_PIN PIN_PA14

#include <stdint.h>

#endif // _MAIN_H_
