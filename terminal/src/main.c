#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include "mfrc522.h"
#include "mfrc522_status.h"
#include "message.h"
#include "messagebox.h"
#include "lcd1602.h"
#include "keypad.h"
#include "utils.h"


uint16_t password_prompt();


int main() {
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

	// Disable Global Interrupts
    IntMasterDisable();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);


	// initialize RST pin for MFRC522 reader
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
	///////////////////////////////////////////////////////////////////////////

	// initialize UART0
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioConfig(0, 115200, SysCtlClockGet());
	///////////////////////////////////////////////////////////////////////////

	// initialize UART1
	GPIOPinConfigure(GPIO_PC4_U1RX);
	GPIOPinConfigure(GPIO_PC5_U1TX);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    UARTFIFODisable(UART1_BASE); // VERY IMPORTANT, DONT USE FIFO

	// initialize SSI2
	GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	GPIOPinConfigure(GPIO_PB6_SSI2RX);
	GPIOPinConfigure(GPIO_PB7_SSI2TX);
	GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_4);

	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);

	SSIDisable(SSI2_BASE);

	SSIConfigSetExpClk(SSI2_BASE, 
						SysCtlClockGet(), 
						SSI_FRF_MOTO_MODE_0,
						SSI_MODE_MASTER,
						1000000,
						8);
	SSIEnable(SSI2_BASE);
	///////////////////////////////////////////////////////////////////////////

	// clear FIFO of SSI2
	uint32_t tmp;
	while (SSIDataGetNonBlocking(SSI2_BASE, &tmp));

    //  Enable Global Interrupts
    IntMasterEnable();

	///////////////////////////////////////////////////////////////////////////

	// config for LCD1602
	PortPin_t en = {.base=GPIO_PORTA_BASE, .pin=GPIO_PIN_5};
    PortPin_t rs = {.base=GPIO_PORTA_BASE, .pin=GPIO_PIN_6};
    PortPin_t d4 = {.base=GPIO_PORTA_BASE, .pin=GPIO_PIN_7};
    PortPin_t d5 = {.base=GPIO_PORTE_BASE, .pin=GPIO_PIN_1};
    PortPin_t d6 = {.base=GPIO_PORTE_BASE, .pin=GPIO_PIN_2};
    PortPin_t d7 = {.base=GPIO_PORTE_BASE, .pin=GPIO_PIN_3};

    lcd1602_init(en, rs, d4, d5, d6, d7);
    ///////////////////////////////////////////////////////////////////////////

    // config for keypad 4x3
    PortPin_t c1 = {.base=GPIO_PORTB_BASE, .pin=GPIO_PIN_1};
    PortPin_t c2 = {.base=GPIO_PORTE_BASE, .pin=GPIO_PIN_4};
    PortPin_t c3 = {.base=GPIO_PORTE_BASE, .pin=GPIO_PIN_5};

    PortPin_t r1 = {.base=GPIO_PORTD_BASE, .pin=GPIO_PIN_6};
    PortPin_t r2 = {.base=GPIO_PORTD_BASE, .pin=GPIO_PIN_1};
    PortPin_t r3 = {.base=GPIO_PORTD_BASE, .pin=GPIO_PIN_2};
    PortPin_t r4 = {.base=GPIO_PORTD_BASE, .pin=GPIO_PIN_3};

    keypad_init(/* rows = */ 4, /* cols = */ 3);
    keypad_setRows(r1, r2, r3, r4);
    keypad_setColumns(c1, c2, c3);
    ///////////////////////////////////////////////////////////////////////////

	tiva_mfrc522_init(	/* SPI module = */ SSI2_BASE,
						/* SS port = */ GPIO_PORTB_BASE,
						/* SS pin = */ GPIO_PIN_5,
						/* RST port = */ GPIO_PORTB_BASE, 
						/* RST pin = */ GPIO_PIN_0);

	///////////////////////////////////////////////////////////////////////////
    Message_t data[8];
    Message_t msg;
    MessageBox_p inbox = uart_messagebox_create(UART1_BASE, data, 8);
    ///////////////////////////////////////////////////////////////////////////

    delay_init();

    const uint8_t preamble[4] = {0xAA, 0xBB, 0xCC, 0xDD};
	UID_t uid;


	while (1) {
		if (mfrc522_available()) {
			mfrc522_getID(&uid);

			UARTprintf("ID: %d bytes\n", uid.size);
			for (uint8_t i = 0; i < uid.size; i++) {
				UARTprintf("%02X ", uid.UID[i]);
			}
			UARTprintf("\n");
			
			mfrc522_sendHaltA();
			
			message_send(preamble, 0, 1, uid.UID, uid.size);
		}

		if (messagebox_isAvailable(inbox)) {
			messagebox_pop(inbox, &msg);

            UARTprintf("[From] %d\n", msg.address);
            UARTprintf("[Size] %d\n", msg.payloadSize);
            UARTprintf("[Payload] ");
            for (int i = 0; i < msg.payloadSize; i++) {
                UARTprintf("%02X ", msg.payload[i]);
            }

            UARTprintf("\n-----------------------------\n");

            uint16_t onetime_pwd = ((uint16_t*)msg.payload)[0];

            // if card id is invalid, 
            if (onetime_pwd == 0) {
                lcd1602_clear();
                lcd1602_print("Invalid card", 1, 2, 16, 50);
            }
            // if card id is valid
            else {
                if (password_prompt() == onetime_pwd) {
                    lcd1602_clear();
                    lcd1602_print("Access accepted", 1, 2, 16, 50);
                }
                else {
                    lcd1602_clear();
                    lcd1602_print("Access denied", 1, 2, 16, 50);
                }
            }
            delay_ms(1000);
            lcd1602_clear();
		}
		delay_ms(50);
	}
}


uint16_t password_prompt() {
    uint8_t key;
    int8_t counter = 0;
    uint16_t input = 0;
    uint8_t pos_row = 2;
    uint8_t pos_col = 7;
    uint8_t max_digit = 4;
    uint8_t last_key[4] = {0};

    lcd1602_print("Enter password", /* row */ 1, /* col */ 2, 16, 50);
    lcd1602_print("____", pos_row, pos_col, 16, 50);
    lcd1602_setCursor(pos_row, pos_col);

    while (1) {
        key = keypad_read();

        if (!key) {
            continue;
        }

        // if "enter" is pressed
        if (key == '#') {
            if (counter == max_digit) {
                return input;
            }
            continue;
        }

        // if "delete"/"cancel" is pressed
        if (key == '*') {
            if (counter > 0) {
                // update counter
                counter--;

                lcd1602_setCursor(pos_row, pos_col+counter);
                lcd1602_putChar('_');

                // update input
                input = (input - last_key[counter]) / 10;

                // save the last pressed key
                last_key[counter] = 0;
            }
            else {
                return 0;
            }
        }
        // if "number" is pressed
        else {
            if (counter < max_digit) {
                lcd1602_setCursor(pos_row, pos_col+counter);
                lcd1602_putChar(key);

                // save the last pressed key
                last_key[counter] = key - '0';

                // update input
                input = input * 10 + last_key[counter];

                // update counter
                counter++;
            }
        }
    }
}