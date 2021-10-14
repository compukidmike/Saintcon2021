
/*--NFC write NDEF to ntag--------------------------
 *
 *	uint8_t buffer[50] = {0};
 *	ndef_mime_card("text/plain", "This is a text record", 21, buffer);
 *	if( nfc_ndef_tag_writer(buffer) ){
 *		...
 */

/*--NFC read NDEF to buffer-------------------------
 *
 *	uint8_t buffer[200] = {0};
 *	if( nfc_reader(buffer) ){
 *		...
 */

/*--NFC tag emulation-------------------------------
 *
 *	/////////////// Well Know load into TAG_BUFF ///////////////
 *
 *		uint8_t ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','.','o','r','g'};
 *		ndef_well_known(ndef_data, sizeof(ndef_data));
 *
 *	/////////////// Mime load into TAG_BUFF ///////////////
 *
 *		ndef_mime_card("text/plain", "This is a text record", 21, NULL);
 *
 *	/////////////// VCARD load into TAG_BUFF ///////////////
 *
 *		ndef_vcard( "BEGIN:VCARD....END:VCARD" );
 *
 *	/////////////// Start tag emulation ///////////////
 *
 *		start_nfc_tag_emulation(true);
 *
 *	///////////////
 *
 *		NFC_BADGE_WRITE is set to NWRITE_END after a NDEF record is written (end to beginning)
 *		NFC_BADGE_READ	is set true after a read event
 *		TAG_BUFF		this is the memory buffer for tag emulation data
 *
 */


#ifndef NFC_H_
#define NFC_H_

#include <stdio.h>
#include <atmel_start.h>

#define RESTART_NO_FIELD_CMD 10

#define NFC_IRQ_IN_PIN PIN_PB08
#define NFC_IRQ_OUT_PIN PIN_PB09
#define NFC_CS_PIN PIN_PA14

#define NDEF_MSG_BLK 0x03
#define NDEF_MSG_PROP 0xFD
#define NDEF_MSG_END 0xFE

#define MB 0x80
#define ME 0x40
#define CF 0x20
#define SR 0x10
#define IL 0x08
#define TNF_WELL_KNOWN 0x01
#define TNF_MIME 0x02

#define NDEF_TYPE_VCARD "text/vcard"
#define NDEF_TYPE_P2P "application/encrypted"

#define NDEF_TYPE_LEN 0x01

#define NDEF_TEXT 'T'
#define NDEF_URL 'U'

#define URL_HTTP 0x3
#define URL_HTTPS 0x4
#define URL_PHONE 0x5
#define URL_EMAIL 0x6

#define NFC_NAK 0x00
#define NFC_ACK 0xAA

#define TAG_BUFF_LEN 304 // Must be divisible by 16

#define UID "SAINTCN" // UID for tag emulation. Must be exactly 7 characters long


extern volatile char TAG_BUFF[];
extern volatile bool NFC_BADGE_READ;
extern volatile uint8_t NFC_BADGE_WRITE[2];
enum NFC_BADGE_WRITE_STATES {NWRITE_IDLE, NWRITE_ACTIVE, NWRITE_END};
	
#pragma pack(1)
typedef struct ndef_header {
	uint16_t unk;
	uint8_t flags;
	uint8_t type_len;
	uint8_t payload_len;
	char payload_type;
	uint8_t payload[];
} ndef_header;


void nfc_init(void);	// Setups the ST25R95
bool nfc_test(void);	// Returns true if the ST25R95 responds correctly


bool nfc_ndef_tag_writer(char * ndef_buff);						// Send the ndef record in the provided buffer to a tag. true on success
bool nfc_reader(char * output_buffer);							// Attempt a read and save the NDEF record to the provided buffer. true on success
void start_nfc_tag_emulation(bool setup_irq, ext_irq_cb_t cb);	// Setup and nfc tag emulation. setup_irq will attach nfc_tag_emulation_irq and will set the cb to be called when end of transactions are called.


void ndef_well_known(char* tag_data, uint8_t size);												// Populate TAG_BUFF with a NDEF well known record
void ndef_vcard(char * vcard_data);																// Populate TAG_BUFF with a NDEF vcard record
void ndef_mime_card(char * mime_type, char * mime_data, uint8_t data_len, char * save_buff);	// Populate TAG_BUFF with a NDEF mime record.  Set save_buff to NULL to use TAG_BUFF;

static void nfc_tag_emulation_irq(void);// Interrupt function for tag emulation

void nfc_reset(void);
void init_tag(void);					// Init TAG_BUFF with ntag215 values
bool nfc_select_card(char * buffer);	// Type 2 reader handshake

void nfc_poll(void);														// Wait for NFC_IRQ_OUT_PIN to go low
void nfc_read(uint8_t* buffer);												// Read from the ST25R95
void nfc_comm(uint8_t * rx, char * command, bool read);						// Send command to ST25R95
void nfc_raw_comm(uint8_t * rx, char * command, uint8_t size, bool read);	// Send raw data to ST25R95



#endif
