#ifdef __HARDWARE_V11

//
//  FOR PCB VERSION 1.1 -
//     - with Adafruit FM TX connector
//     - all pins connected on shield connectors
//     - general purpose LED on board
//     - 3V3 power supply for shields
//     - SPI/ICSP connector in standard position for connection to shields
//     - extra ground/5V/3V3 pads
//

// Build using Bobuino pinout (numbers on shield like an Arduino Uno)

#define __DABCS_PIN        8            // PD5 - DAB chip select
#define __DABINTERRUPT_PIN 2            // PD2 - 'data ready' from DAB RX
#define __DABPWREN_PIN     6            // PB2 - Power enable for DAB RX
#define __DABRESET_PIN     7            // PB3 - Reset DAB RX

#define __ENCCLK_PIN       19           // PA2 - Encoder "clock"
#define __ENCDT_PIN        20           // PA1 - Encoder "data"
#define __ENCBUTTON_PIN    21           // PA0 - Encoder push button

#define __LED_PIN          29           // PC7 - yellow LED on PCB

#define __FMTXRST_PIN      30           // PD4 - FM transmitter reset

#else

// Standard pinout - original PCB version

#define __DABCS_PIN        3            // PB3 - DAB chip select
#define __DABINTERRUPT_PIN 2            // PB2 - 'data ready' from DAB RX
#define __DABPWREN_PIN     1            // PB1 - Power enable for DAB RX
#define __DABRESET_PIN     0            // PB0 - Reset DAB RX

#define __ENCCLK_PIN       26           // PA2 - Encoder "clock"
#define __ENCDT_PIN        25           // PA1 - Encoder "data"
#define __ENCBUTTON_PIN    24           // PA0 - Encoder push button

#endif
