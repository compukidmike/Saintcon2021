/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#include "atmel_start.h"
#include "usb_start.h"

#if CONF_USBD_HS_SP
static uint8_t single_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    CDCD_ACM_HS_DESCES_LS_FS};
static uint8_t single_desc_bytes_hs[] = {
    /* Device descriptors and Configuration descriptors list. */
    CDCD_ACM_HS_DESCES_HS};
#define CDCD_ECHO_BUF_SIZ CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ_HS
#else
static uint8_t single_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    CDCD_ACM_DESCES_LS_FS};
#define CDCD_ECHO_BUF_SIZ CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ
#endif

static struct usbd_descriptors single_desc[]
    = {{single_desc_bytes, single_desc_bytes + sizeof(single_desc_bytes)}
#if CONF_USBD_HS_SP
       ,
       {single_desc_bytes_hs, single_desc_bytes_hs + sizeof(single_desc_bytes_hs)}
#endif
};

void initNewGame(void);
void handleInput(char *c, uint8_t len);

/** Ctrl endpoint buffer */
static uint8_t ctrl_buffer[64];
volatile bool cdcTransferRead = false;
volatile uint32_t cdcTransferReadLen;
volatile bool cdcTransferWrite = false;
volatile bool cdcConnected = false;

static bool cdcWriteDone(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	cdcTransferWrite = false;
	return false;
}

static bool cdcReadDone(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	cdcTransferReadLen = count;
	return false;
}

static uint8_t inBuf[8];
static uint8_t cdcRead(char* buf)
{
	if (cdcTransferReadLen) {
		memcpy(buf, inBuf, cdcTransferReadLen);
		uint8_t ret = cdcTransferReadLen;
		cdcTransferReadLen = 0;
		cdcdf_acm_read(inBuf, 8); 
		return ret;
	}
	return 0;
}

static uint8_t outBuf[80];
static uint32_t outLen = 0;

int32_t cdcWrite(const char* const buf, const uint16_t length)
{
	const char* end = buf + length;
	for (const char* p = buf; p < end && cdcConnected; ++p) {
		outBuf[outLen++] = *p;

		if (*p == '\n' || outLen==sizeof(outBuf)) {
			cdcTransferWrite = true;
			cdcdf_acm_write(outBuf, outLen);
			while(cdcTransferWrite && cdcConnected);
			outLen = 0;
		}
	}
	return length;
}

/**
 * \brief Callback invoked when Line State Change
 */
static bool usb_device_cb_state_c(usb_cdc_control_signal_t state)
{
	if (state.rs232.DTR) {
		/* Callbacks must be registered after endpoint allocation */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdcReadDone);
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdcWriteDone);
		cdcConnected = true;
		cdcTransferRead = true;
		cdcTransferReadLen = 0;
		cdcdf_acm_read(inBuf, 8);
	}
	else {
		cdcConnected = false;
	}

	/* No error. */
	return false;
}

/**
 * \brief CDC ACM Init
 */
void cdc_device_acm_init(void)
{
	/* usb stack init */
	usbdc_init(ctrl_buffer);

	/* usbdc_register_funcion inside */
	cdcdf_acm_init();

	usbdc_start(single_desc);
	usbdc_attach();
}

bool newConnection = true;
void cdcd_loop(void) {
	if (cdcdf_acm_is_enabled()) {
		if (newConnection) {
			initNewGame();
			cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)usb_device_cb_state_c);
			newConnection = false;
		}
		if (cdcConnected) {
			char input[8];
			uint8_t len = cdcRead(input);
			if (len) {
				handleInput(input, len);
			}
		}
	}
	else
		newConnection = true;
}

void usb_init(void)
{

	cdc_device_acm_init();
}
