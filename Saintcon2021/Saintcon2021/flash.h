/*
 * flash.h
 *
 * Created: 8/18/2021 9:30:11 PM
 *  Author: Professor Plum
 */ 
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef FLASH_H_
#define FLASH_H_

#define FLASH_ID			0x9F
#define FLASH_WRITE_EN		0x06
#define FLASH_WRITE_DE		0x04
#define FLASH_STATUS		0x05
#define FLASH_ERROR			0x07
#define FLASH_QPI_EN		0x38
#define FLASH_QPI_DE		0xF5
#define FLASH_READ			0xEB
#define FLASH_WRITE_PAGE	0x02
#define FLASH_ERASE_ALL		0x60
#define FLASH_ERASE_BLOCK	0xDC
#define FLASH_ERASE_HBLOCK	0x53
#define FLASH_ERASE_SEC		0x21
#define FLASH_SLEEP			0xDH
#define FLASH_WAKE			0xAB
#define FLASH_WRITE_REG		0x01

void flash_init();
void flash_read_id(uint8_t *id);
void flash_enable_write(bool enable);
void flash_erase_all();
void flash_erase_sector(uint32_t addr);
void flash_erase_halfblock(uint32_t addr);
void flash_read(uint32_t addr, void *out, size_t len);
void flash_write(uint32_t addr, void *buf, size_t len);

// Sample vcard
/*
BEGIN:VCARD
VERSION:2.1
N:Wiley;Bob;;;
FN:Bob Wiley
EMAIL;WORK:bob@home.com
TITLE:Your Title
END:VCARD
*/

void flash_save_vcard(char* vcard);
void flash_read_vcard(char* vcard);


#define machine_bkg_img	0
#define machine_img		0
#define parts_img		0
#define build_img		0

#endif /* FLASH_H_ */