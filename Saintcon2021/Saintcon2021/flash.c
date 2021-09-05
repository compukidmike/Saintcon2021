/*
 * flash.c
 *
 * Created: 8/18/2021 10:03:04 PM
 *  Author: Professor Plum
 */ 


#include "flash.h"
#include <driver_init.h>
#include "ILI9331.h"
#include "FrameBuffer.h"

const uint32_t FLASH_VCARD = 0x7ff000;

void flash_init() {
	qspi_sync_enable(&QUAD_SPI_0);
	
	flash_enable_write(true);
	struct _qspi_command cmd = {
		.inst_frame.bits.inst_en      = 1,
		.inst_frame.bits.tfr_type     = QSPI_WRITE_ACCESS,
		.instruction                  = FLASH_QPI_EN,
	};

	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	
}

void flash_read_id(uint8_t *id) {
	struct _qspi_command cmd    = {
		.inst_frame.bits.inst_en  = 1,
		.inst_frame.bits.data_en  = 1,
		.inst_frame.bits.tfr_type = QSPI_READ_ACCESS,
		.inst_frame.bits.width    = QSPI_INST4_ADDR4_DATA4,
		.instruction              = FLASH_ID,
		.buf_len                  = 3,
		.rx_buf                   = id,
	};

	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
}

bool flash_is_busy() {
	uint8_t status = 0xFF;
	struct _qspi_command cmd    = {
		.inst_frame.bits.inst_en  = 1,
		.inst_frame.bits.data_en  = 1,
		.inst_frame.bits.tfr_type = QSPI_READ_ACCESS,
		.inst_frame.bits.width    = QSPI_INST4_ADDR4_DATA4,
		.instruction              = FLASH_STATUS,
		.buf_len                  = 1,
		.rx_buf                   = &status,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	return status & 1;
}

void flash_enable_write(bool enable) {
	struct _qspi_command cmd    = {
		.inst_frame.bits.inst_en  = 1,
		.inst_frame.bits.tfr_type = QSPI_WRITE_ACCESS,
		.inst_frame.bits.width    = QSPI_INST4_ADDR4_DATA4,
		.instruction              = enable?FLASH_WRITE_EN:FLASH_WRITE_DE,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
}

void flash_erase_all() {
	flash_enable_write(true);
	
	while(flash_is_busy());
	
	struct _qspi_command cmd    = {
		.inst_frame.bits.inst_en  = 1,
		.inst_frame.bits.tfr_type = QSPI_WRITEMEM_ACCESS,
		.inst_frame.bits.width    = QSPI_INST4_ADDR4_DATA4,
		.instruction              = FLASH_ERASE_ALL,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	
	LCD_FillRect(0,0,240,240,0);
	uint32_t c=0;
	while(flash_is_busy()) {
		LCD_DrawPixel(30+c%180, 30+c/180, RGB(0,0,255));
		c++;
		delay_ms(2);
	}
}

void flash_erase_halfblock(uint32_t addr) {
	
	flash_enable_write(true);
	
	while(flash_is_busy());
	
	struct _qspi_command cmd    = {
		.inst_frame.bits.inst_en  = 1,
		.inst_frame.bits.addr_en  = 1,
		.inst_frame.bits.tfr_type = QSPI_WRITEMEM_ACCESS,
		.inst_frame.bits.width    = QSPI_INST4_ADDR4_DATA4,
		.instruction              = FLASH_ERASE_HBLOCK,
		.address                  = addr,
		.inst_frame.bits.addr_len = 1,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	
	while(flash_is_busy());
}

void flash_erase_sector(uint32_t addr) {
	
	flash_enable_write(true);
	
	while(flash_is_busy());
	
	struct _qspi_command cmd    = {
		.inst_frame.bits.inst_en  = 1,
		.inst_frame.bits.addr_en  = 1,
		.inst_frame.bits.tfr_type = QSPI_WRITEMEM_ACCESS,
		.inst_frame.bits.width    = QSPI_INST4_ADDR4_DATA4,
		.instruction              = FLASH_ERASE_SEC,
		.address                  = addr,
		.inst_frame.bits.addr_len = 1,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	
	while(flash_is_busy());
}

void flash_read(uint32_t addr, void *out, size_t len) {
	
	struct _qspi_command cmd = {
		.inst_frame.bits.inst_en      = 1,
		.inst_frame.bits.data_en      = 1,
		.inst_frame.bits.addr_en      = 1,
		.inst_frame.bits.dummy_cycles = 10,
		.inst_frame.bits.tfr_type     = QSPI_READMEM_ACCESS,
		.inst_frame.bits.width		  = QSPI_INST4_ADDR4_DATA4,
		.instruction                  = FLASH_READ,
		.address                      = addr,
		.buf_len                      = len,
		.rx_buf                       = out,
		.inst_frame.bits.addr_len     = 1,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
}

void flash_write(uint32_t addr, void *buf, size_t len) {
	flash_enable_write(true);
	
	while(flash_is_busy());
	struct _qspi_command cmd = {
		.inst_frame.bits.inst_en      = 1,
		.inst_frame.bits.data_en      = 1,
		.inst_frame.bits.addr_en      = 1,
		.inst_frame.bits.tfr_type     = QSPI_WRITEMEM_ACCESS,
		.inst_frame.bits.width		  = QSPI_INST4_ADDR4_DATA4,
		.instruction                  = FLASH_WRITE_PAGE,
		.address                      = addr,
		.buf_len                      = len,
		.tx_buf                       = buf,
		.inst_frame.bits.addr_len     = 1,
	};
	qspi_sync_serial_run_command(&QUAD_SPI_0, &cmd);
	
	while(flash_is_busy());
}

void flash_save_vcard(char* vcard) {
	flash_erase_sector(FLASH_VCARD);
	flash_write(FLASH_VCARD, vcard, 256);
	flash_write(FLASH_VCARD, vcard+256, 256);
}

void flash_read_vcard(char* vcard) {
	flash_read(FLASH_VCARD, vcard, 512);
}