/*
 * main.h
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#define NFC_IRQ_IN_PIN PIN_PB08
#define NFC_IRQ_OUT_PIN PIN_PB09
#define NFC_CS_PIN PIN_PB14

#include <atmel_start.h>
#include <stdint.h>

struct io_descriptor *io;

void touch_status_display(void);

int32_t spi_m_sync_io_readwrite(struct io_descriptor *io, uint8_t *rxbuf, uint8_t *txbuf, const uint16_t length);

void NFC_init(void);
void exampleRfalPollerRun( void );
void exampleNFCARun( void );


#endif /* MAIN_H_ */