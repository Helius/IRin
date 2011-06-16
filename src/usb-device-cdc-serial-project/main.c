/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//-----------------------------------------------------------------------------
/// \dir "USB CDC serial converter"
///
/// !!!Purpose
///
/// The USB CDC Serial Project will help you to get familiar with the
/// USB Device Port(UDP) and USART interface on AT91SAM microcontrollers. Also
/// it can help you to be familiar with the USB Framework that is used for
/// rapid development of USB-compliant class drivers such as USB Communication
/// Device class (CDC).
///
/// You can find following information depends on your needs:
/// - Sample usage of USB CDC driver and USART driver.
/// - USB CDC driver development based on the AT91 USB Framework.
/// - USB enumerate sequence, the standard and class-specific descriptors and
///   requests handling.
/// - The initialize sequence and usage of UDP interface.
/// - The initialize sequence and usage of USART interface with PDC.
///
/// !See
/// - usart: USART interface driver
/// - tc: TIMER/COUNTER interface driver
/// - usb: USB Framework, USB CDC driver and UDP interface driver
///    - "AT91 USB device framework"
///       - "USBD API"
///    - "cdc-serial"
///       - "USB CDC Serial Device"
///       - "USB CDC Serial Host Driver"
///
/// !!!Requirements
///
/// This package can be used with all Atmel evaluation kits that have both
/// UDP and USART interface.
///
/// The current supported board list:
/// - at91sam7s-ek (exclude at91sam7s32)
/// - at91sam7x-ek
/// - at91sam7xc-ek
/// - at91sam7a3-ek
/// - at91sam7se-ek
/// - at91sam9260-ek
/// - at91sam9263-ek
///
/// !!!Description
///
/// When an EK running this program connected to a host (PC for example), with
/// USB cable, the EK appears as a Seriao COM port for the host, after driver
/// installation with the offered 6119.inf. Then the host can send or receive
/// data through the port with host software. The data stream from the host is
/// then sent to the EK, and forward to USART port of AT91SAM chips. The USART
/// port of the EK is monitored by the timer and the incoming data will be sent
/// to the host.
///
/// !!!Usage
///
/// -# Build the program and download it inside the evaluation board. Please
///    refer to the
///    <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6224.pdf">
///    SAM-BA User Guide</a>, the
///    <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6310.pdf">
///    GNU-Based Software Development</a> application note or to the
///    <a href="ftp://ftp.iar.se/WWWfiles/arm/Guides/EWARM_UserGuide.ENU.pdf">
///    IAR EWARM User Guide</a>, depending on your chosen solution.
/// -# On the computer, open and configure a terminal application
///    (e.g. HyperTerminal on Microsoft Windows) with these settings:
///   - 115200 bauds
///   - 8 bits of data
///   - No parity
///   - 1 stop bit
///   - No flow control
/// -# Start the application.
/// -# In the terminal window, the following text should appear:
///     \code
///     -- USB Device CDC Serial Project xxx --
///     -- AT91xxxxxx-xx
///     -- Compiled: xxx xx xxxx xx:xx:xx --
///     \endcode
/// -# When connecting USB cable to windows, the LED blinks, and the host
///    reports a new USB %device attachment (if it's the first time you connect
///    an %audio speaker demo board to your host). You can use the inf file
///    at91lib\\usb\\device\\cdc-serial\\drv\\6119.inf to install the serial
///    port. Then new "AT91 USB to Serial Converter (COMx)" appears in the
///    hardware %device list.
/// -# You can run hyperterminal to send data to the port. And it can be seen
///    at the other hyperterminal connected to the USART port of the EK.
///
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// This file contains all the specific code for the
/// usb-device-cdc-serial-project
///
/// !Contents
///
/// The code can be roughly broken down as follows:
///    - Configuration functions
///       - VBus_Configure
///       - PIO & Timer configurations in start of main
///    - Interrupt handlers
///       - ISR_Vbus
///       - ISR_Timer0
///       - ISR_Usart0
///    - Callback functions
///       - UsbDataReceived
///    - The main function, which implements the program behavior
///
/// Please refer to the list of functions in the #Overview# tab of this unit
/// for more detailed information.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <string.h>
#include <board.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <aic/aic.h>
#include <tc/tc.h>
#include <usart/usart.h>
#include <utility/trace.h>
#include <utility/led.h>
#include <usb/device/cdc-serial/CDCDSerialDriver.h>
#include <usb/device/cdc-serial/CDCDSerialDriverDescriptors.h>
#include <pmc/pmc.h>
#include "ir.h"
#include "microrl.h"

#define true 1
#define false 0

//  ir object data and pointer on it
ir_t ir;
ir_t * pir = &ir;
// microrl object and pointer on it
microrl_t rl;
microrl_t * prl = &rl;

//------------------------------------------------------------------------------
//      Definitions
//------------------------------------------------------------------------------
#ifndef AT91C_ID_TC0
    #define AT91C_ID_TC0 AT91C_ID_TC
#endif

/// Size in bytes of the buffer used for reading data from the USB & USART
#define DATABUFFERSIZE \
    BOARD_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_DATAIN)

/// Use for power management
#define STATE_IDLE    0
/// The USB device is in suspend state
#define STATE_SUSPEND 4
/// The USB device is in resume state
#define STATE_RESUME  5

//------------------------------------------------------------------------------
//      Internal variables
//------------------------------------------------------------------------------
/// State of USB, for suspend and resume
unsigned char USBState = STATE_IDLE;

/// Number of byte USB recieve
static int usb_recieve;
/// List of pins that must be configured for use by the application.
//static const Pin pins[] = {PIN_USART0_TXD, PIN_USART0_RXD};

/// Double-buffer for storing incoming USART data.
//static unsigned char usartBuffers[2][DATABUFFERSIZE];

/// Current USART buffer index.
//static unsigned char usartCurrentBuffer = 0;

/// Buffer for storing incoming USB data.
static unsigned char usbBuffer[DATABUFFERSIZE];


/// Pin definititon
static const Pin pinIR     = PIN_IR_REC;
static const Pin pinToggle = PIN_LED_DS4;
static const Pin ledRed    = PIN_LED_RED;
static const Pin ledGrn    = PIN_LED_GRN;
static const Pin dbg_pin   = PIN_DBG;

//*****************************************************************************
// send string to host
void cdc_write (char * data)
{
	// Send current buffer through the USB
	while (CDCDSerialDriver_Write(data, strlen(data), 0, 0) != USBD_STATUS_SUCCESS);
}

//*****************************************************************************
// ir signal line change interrupt
static void ISR_IR(const Pin *pPin)
{
	
  // call ir handler and pass level on IR pin (inverse, because IR reciever invert pulse)
	ir_line_handler(pir, !PIO_Get(&pinIR));
}

//------------------------------------------------------------------------------
/// Configures the IR pin to trigger an interrupt when the level on that pin
/// changes.
//------------------------------------------------------------------------------
static void IR_Configure( void )
{
    TRACE_DEBUG(" ");

    // Configure PIO
    PIO_Configure (&pinIR, 1);
    PIO_ConfigureIt (&pinIR, ISR_IR);
    PIO_EnableIt (&pinIR);
}

#define VBUS_CONFIGURE()    USBD_Connect()

#if defined (CP15_PRESENT)
//------------------------------------------------------------------------------
/// Put the CPU in 32kHz, disable PLL, main oscillator
/// Put voltage regulator in standby mode
//------------------------------------------------------------------------------
void LowPowerMode(void)
{
    PMC_CPUInIdleMode();
}
//------------------------------------------------------------------------------
/// Put voltage regulator in normal mode
/// Return the CPU to normal speed 48MHz, enable PLL, main oscillator
//------------------------------------------------------------------------------
void NormalPowerMode(void)
{
}

#elif defined (at91sam7x) || defined (at91sam7xc)
//------------------------------------------------------------------------------
/// Put the CPU in 32kHz, disable PLL, main oscillator
/// Put voltage regulator in standby mode
//------------------------------------------------------------------------------
void LowPowerMode(void)
{
    // MCK=48MHz to MCK=32kHz
    // MCK = SLCK/2 : change source first from 48 000 000 to 18. / 2 = 9M
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_PRES_CLK_2;
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
    // MCK=SLCK : then change prescaler
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK;
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
    // disable PLL
    AT91C_BASE_PMC->PMC_PLLR = 0;
    // Disable Main Oscillator
    AT91C_BASE_PMC->PMC_MOR = 0;

    // Voltage regulator in standby mode : Enable VREG Low Power Mode
    AT91C_BASE_VREG->VREG_MR |= AT91C_VREG_PSTDBY;

    PMC_DisableProcessorClock();
}

//------------------------------------------------------------------------------
/// Put voltage regulator in normal mode
/// Return the CPU to normal speed 48MHz, enable PLL, main oscillator
//------------------------------------------------------------------------------
void NormalPowerMode(void)
{
    // Voltage regulator in normal mode : Disable VREG Low Power Mode
    AT91C_BASE_VREG->VREG_MR &= ~AT91C_VREG_PSTDBY;

    // MCK=32kHz to MCK=48MHz
    // enable Main Oscillator
    AT91C_BASE_PMC->PMC_MOR = (( (AT91C_CKGR_OSCOUNT & (0x06 <<8)) | AT91C_CKGR_MOSCEN ));
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS ) );

    // enable PLL@96MHz
    AT91C_BASE_PMC->PMC_PLLR = ((AT91C_CKGR_DIV & 0x0E) |
         (AT91C_CKGR_PLLCOUNT & (28<<8)) |
         (AT91C_CKGR_MUL & (0x48<<16)));
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK ) );
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
    AT91C_BASE_CKGR->CKGR_PLLR |= AT91C_CKGR_USBDIV_1 ;
    // MCK=SLCK/2 : change prescaler first
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_PRES_CLK_2;
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
    // MCK=PLLCK/2 : then change source
    AT91C_BASE_PMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK  ;
    while( !( AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY ) );
}

#endif

//------------------------------------------------------------------------------
//         Internal functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// Handles interrupts coming from Timer #1 each 100us.
//------------------------------------------------------------------------------
static void ISR_Timer1()
{
    unsigned int status = AT91C_BASE_TC1->TC_SR;
    if ((status & AT91C_TC_CPCS) != 0) {
//			if (PIO_Get (&pinToggle))
//				PIO_Clear (&pinToggle);
//			else
//				PIO_Set (&pinToggle);
			// call 100us periodicaly
			ir_time_handler (pir);
			AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
    }
}


//------------------------------------------------------------------------------
//         Callbacks re-implementation
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Invoked when the USB device leaves the Suspended state. By default,
/// configures the LEDs.
//------------------------------------------------------------------------------
void USBDCallbacks_Resumed(void)
{
    // Initialize LEDs
    LED_Configure(USBD_LEDPOWER);
    LED_Set(USBD_LEDPOWER);
    LED_Configure(USBD_LEDUSB);
    LED_Clear(USBD_LEDUSB);
    USBState = STATE_RESUME;
}

//------------------------------------------------------------------------------
/// Invoked when the USB device gets suspended. By default, turns off all LEDs.
//------------------------------------------------------------------------------
void USBDCallbacks_Suspended(void)
{
    // Turn off LEDs
    LED_Clear(USBD_LEDPOWER);
    LED_Clear(USBD_LEDUSB);
    USBState = STATE_SUSPEND;
}


//------------------------------------------------------------------------------
/// Callback invoked when data has been received on the USB.
//------------------------------------------------------------------------------
static void UsbDataReceived(unsigned int unused,
                            unsigned char status,
                            unsigned int received,
                            unsigned int remaining)
{
    // Check that data has been received successfully
    if (status == USBD_STATUS_SUCCESS) {

        // Send data through USART
//        while (!USART_WriteBuffer(AT91C_BASE_US0, usbBuffer, received));
//        AT91C_BASE_US0->US_IER = AT91C_US_TXBUFE;
			usb_recieve = received;

        // Check if bytes have been discarded
        if ((received == DATABUFFERSIZE) && (remaining > 0)) {

            TRACE_WARNING(
                      "UsbDataReceived: %u bytes discarded\n\r",
                      remaining);
        }
    }
    else {

        TRACE_WARNING( "UsbDataReceived: Transfer error\n\r");
    }
}

//------------------------------------------------------------------------------
//          Main
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Initializes drivers and start the USB <-> Serial bridge.
//------------------------------------------------------------------------------
int main()
{
	// Enable User Reset and set its minimal assertion to 960 us
	//		while(!(AT91C_BASE_RSTC->RSTC_RSR & AT91C_RSTC_NRSTL)) {}; // this is no need
	AT91C_BASE_RSTC->RSTC_RMR = AT91C_RSTC_URSTEN | (0x4<<8) | (unsigned int)(0xA5 << 24);

	TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
	printf ("\n\r---------------------------------------\n\r");
	printf ("-- USB Device CDC Serial Project %s --\n\r", SOFTPACK_VERSION);
	printf ("-- %s\n\r", BOARD_NAME);
	printf ("-- Compiled: %s %s --\n\r\n\r", __DATE__, __TIME__);

	// init of ir
	ir_init (pir);
	microrl_init (prl, cdc_write);

	// If they are present, configure IR & Wake-up pins
	PIO_InitializeInterrupts(0);
	IR_Configure ();

	LED_Configure(USBD_LEDUSB);
	LED_Set(USBD_LEDUSB);

	//helius: set pull up manually
	TRACE_DEBUG ("Disconnect from USB");
	static const Pin pinPullUp = PIN_USB_PULLUP;
	PIO_Configure(&pinPullUp, 1); 
	PIO_Clear (&pinPullUp);
	for (int i = 0; i < 500; i++)
		printf ("   ");
	PIO_Set (&pinPullUp);

	//helius: configere single pin
	PIO_Configure(&pinToggle, 1); 
	PIO_Configure(&ledRed, 1); 
	PIO_Configure(&ledGrn, 1); 
	PIO_Configure(&dbg_pin, 1); 

	//red on
	PIO_Set (&ledRed);
	PIO_Clear (&ledGrn);


	// Configure timer 1 for IR module (100us interrupt)
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC1->TC_IDR = 0xFFFFFFFF;
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV3_CLOCK
													 | AT91C_TC_CPCSTOP
													 | AT91C_TC_CPCDIS
													 | AT91C_TC_WAVESEL_UP_AUTO
													 | AT91C_TC_WAVE;
	AT91C_BASE_TC1->TC_RC = 145; // 100us
	AT91C_BASE_TC1->TC_IER = AT91C_TC_CPCS;
	AIC_ConfigureIT(AT91C_ID_TC1, 0, ISR_Timer1);
	AIC_EnableIT(AT91C_ID_TC1);
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	// BOT driver initialization
	CDCDSerialDriver_Initialize();

	// connect if needed
	VBUS_CONFIGURE();
	usb_recieve = 0;

	// Driver loop
	while (1) {

		// Device is not configured
		if (USBD_GetState() < USBD_STATE_CONFIGURED) {

				// Connect pull-up, wait for configuration
				USBD_Connect();
				while (USBD_GetState() < USBD_STATE_CONFIGURED);

				// Start receiving data on the USB
				CDCDSerialDriver_Read(usbBuffer,
															DATABUFFERSIZE,
															(TransferCallback) UsbDataReceived,
															0);
		} else {
			PIO_Clear (&ledRed);					
		}

		if( USBState == STATE_SUSPEND ) {
				TRACE_DEBUG("suspend  !\n\r");
				USBState = STATE_IDLE;
		}
		if( USBState == STATE_RESUME ) {
				// Return in normal MODE
				TRACE_DEBUG("resume !\n\r");
				USBState = STATE_IDLE;
		}
		// check new IR code
		int key_code = ir_code (pir);
		if (key_code) {
			char key_str [32] = {0};
			sprintf (key_str, "NEC %X\n\r", key_code);
			cdc_write (key_str);
		}

		if (USBD_GetState() == USBD_STATE_CONFIGURED) {
			if (usb_recieve) {
				TRACE_DEBUG ("USB Reciev %d", usb_recieve);
				for (int i = 0; i < usb_recieve; i++)
					microrl_insert_char (prl, usbBuffer[i]);
				
				usb_recieve = 0;
				CDCDSerialDriver_Read(usbBuffer,
															DATABUFFERSIZE,
															(TransferCallback) UsbDataReceived,
															0);
			}
		}
	}
}
