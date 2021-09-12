


#ifndef NFC_H_
#define NFC_H_

#include <stdio.h>
#include <atmel_start.h>

#define NFC_IRQ_IN_PIN PIN_PB08
#define NFC_IRQ_OUT_PIN PIN_PB09
#define NFC_CS_PIN PIN_PA14

bool nfc_init(void);
void poll_nfc(void);
bool nfc_test(void);
uint8_t nfc_read(uint8_t* );
void nfc_reset(void);

#endif