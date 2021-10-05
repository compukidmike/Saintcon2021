


#ifndef NFC_H_
#define NFC_H_

#include <stdio.h>
#include <atmel_start.h>

#define NFC_IRQ_IN_PIN PIN_PB08
#define NFC_IRQ_OUT_PIN PIN_PB09
#define NFC_CS_PIN PIN_PA14

#define NDEF_MSG_BLK 0x03
#define NDEF_MSG_END 0xFE

#define NDEF_LEN_SPACE

#define MB 0x80
#define ME 0x40
#define CF 0x20
#define SR 0x10
#define IL 0x08
#define TNF_WELL_KNOWN 0x01

#define NDEF_TYPE_LEN 0x01

#define NDEF_TEXT 0x54
#define NDEF_URL 0x55

#define URL_HTTP 0x3
#define URL_HTTPS 0x4
#define URL_PHONE 0x5
#define URL_EMAIL 0x6


bool nfc_init(void);
void nfc_poll(void);
bool nfc_test(void);
uint8_t nfc_read(uint8_t* buffer);
void nfc_reset(void);
void nfc_comm(uint8_t * rx, uint8_t * tx, char * command, uint8_t size, bool read);
void nfc_reader(void);

#endif