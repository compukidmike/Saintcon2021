#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define VENDOR_NAME "MK Factor"
#define PRODUCT_NAME "DEF CON 29 Badge"
#define VOLUME_LABEL "DC29Badge"

#define BOARD_ID "SAME53J18A-dc29-v0"

#define INDEX_URL "https://www.defcon.org/"

#define HOLD_PIN PIN_PA27
#define HOLD_PIN_PULLUP
#define HOLD_STATE 0

//#define USB_VID 0x2341
//#define USB_PID 0x024D

#define LED_PIN PIN_PA22
#define LED_PIN_PULLUP
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

#define BOOT_USART_MODULE                 SERCOM4
#define BOOT_USART_MASK                   APBDMASK
#define BOOT_USART_BUS_CLOCK_INDEX        MCLK_APBDMASK_SERCOM4
#define BOOT_USART_PAD_SETTINGS           UART_RX_PAD1_TX_PAD0
#define BOOT_USART_PAD3                   PINMUX_UNUSED
#define BOOT_USART_PAD2                   PINMUX_UNUSED
#define BOOT_USART_PAD1                   PINMUX_PB09D_SERCOM4_PAD1
#define BOOT_USART_PAD0                   PINMUX_PB08D_SERCOM4_PAD0
#define BOOT_GCLK_ID_CORE                 SERCOM4_GCLK_ID_CORE
#define BOOT_GCLK_ID_SLOW                 SERCOM4_GCLK_ID_SLOW

#endif