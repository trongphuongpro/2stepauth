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

	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2))
	{
	}
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// initialize RST pin for MFRC522 reader
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

	// initialize UART0
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioConfig(0, 115200, SysCtlClockGet());

	// initialize UART1
	GPIOPinConfigure(GPIO_PC4_U1RX);
	GPIOPinConfigure(GPIO_PC5_U1TX);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    UARTFIFODisable(UART1_BASE);

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

	// clear FIFO of SSI2
	uint32_t tmp;
	while (SSIDataGetNonBlocking(SSI2_BASE, &tmp));

	UARTprintf("--------------\n");

	tiva_mfrc522_init(	/* SPI module = */ SSI2_BASE,
						/* SS port = */ GPIO_PORTB_BASE,
						/* SS pin = */ GPIO_PIN_5,
						/* RST port = */ GPIO_PORTB_BASE, 
						/* RST pin = */ GPIO_PIN_0);

	
    Message_t data[8];
    Message_t msg;
    MessageBox_p inbox = uart_messagebox_create(UART1_BASE, data, 8);

	//  Enable Global Interrupts
    IntMasterEnable();

    const uint8_t preamble[4] = {0xAA, 0xBB, 0xCC, 0xDD};
	UID_t uid;
	uint8_t status;

	while (1) {
		UARTprintf("--------------\n");

		if (mfrc522_available()) {
			status = mfrc522_getID(&uid);

			UARTprintf("Card type: %d\nID: ", uid.SAK);
			for (uint8_t i = 0; i < uid.size; i++) {
				UARTprintf("%02X ", uid.UID[i]);
			}
			UARTprintf("\n");

			UARTprintf("Select: %d\n", status);
			
			status = mfrc522_sendHaltA();
			UARTprintf("Halt: %d\n", status);
			
			message_send(preamble, 1, 2, uid.UID, uid.size);
		}

		if (messagebox_isAvailable(inbox)) {
			messagebox_pop(inbox, &msg);

            UARTprintf("\n[Address] %d", msg.address);
            UARTprintf("\n[Size] %d", msg.payloadSize);
            UARTprintf("\n[Payload] ");
            for (int i = 0; i < msg.payloadSize; i++) {
                UARTprintf("%c", msg.payload[i]);
            }

            UARTprintf("\n-----------------------------");
		}
		SysCtlDelay(500*SysCtlClockGet()/3000);
	}
}
