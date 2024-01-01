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

    sprintf(txtBuff, "txGetProperty(0x%04x)\n", property);
    Serial.print(txtBuff);
    
    Wire.beginTransmission(SI4710_ADDR1);
    Wire.write(SI4710_CMD_GET_PROPERTY);
    Wire.write(0);
    Wire.write(property >> 8);
    Wire.write(property & 0x00ff);
    Wire.endTransmission();

    Wire.requestFrom(SI4710_ADDR1, 4);

    rtn = Wire.read();
    sprintf(txtBuff, "  Status - %02x\n", rtn);
    Serial.print(txtBuff);
    
    rtn = Wire.read();
    rtn = (Wire.read() << 8) | Wire.read();

    sprintf(txtBuff, "  Value - 0x%04x\n", rtn);
    Serial.print(txtBuff);

    return rtn;
}
