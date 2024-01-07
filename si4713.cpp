//
// Extra functionality for SI4713 transmitter not already provided in the Adafruit library
//

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_Si4713.h>

#include "dab1284.h"
#include "types.h" 

unsigned int txGetProperty(unsigned int property)
{
    unsigned int rtn;
    char txtBuff[64];
    
    Wire.beginTransmission(SI4710_ADDR1);
    Wire.write(SI4710_CMD_GET_PROPERTY);
    Wire.write(0);
    Wire.write(property >> 8);
    Wire.write(property & 0x00ff);
    Wire.endTransmission();

    Wire.requestFrom(SI4710_ADDR1, 4);

    // Read status byte
    rtn = Wire.read();

    // Don't care
    rtn = Wire.read();

    // Read property value
    rtn = (Wire.read() << 8) | Wire.read();

    return rtn;
}
