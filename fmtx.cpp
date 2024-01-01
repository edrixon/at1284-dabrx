#include <Arduino.h>
#include <Adafruit_Si4713.h>

#include "dab1284.h"
#include "types.h"
#include "fmtx.h"
#include "pins.h"
#include "si4713.h"

Adafruit_Si4713 tx;

extern PROMTYPE promData;

void txSetRDSstationName(char *ps)
{
    char msg[64];

    sprintf(msg, "RDS PS - %s\n", ps);
    Serial.print(msg);
    
    tx.setRDSstation(ps);
}

void txSetRDSpiCode(unsigned short int pi)
{
    char msg[64];

    sprintf(msg, "RDS PI - 0x%04x\n", pi);
    Serial.print(msg);
    
    tx.setProperty(SI4713_PROP_TX_RDS_PI, pi);
}

void txSetFreq(unsigned int txFreq)
{
    char msg[64];
    
    sprintf(msg, "TX frequency %d.%02d MHz\n", txFreq / 100, txFreq % 100);
    Serial.print(msg);

    tx.tuneFM(txFreq);
}

void txOn(boolean turnOn)
{
    if(turnOn == true)
    {
        Serial.println("Transmitter enabled");
        tx.setTXpower(115);  // dBuV, 88-115 max
    }
    else
    {
        Serial.println("Transmitter disabled");
        tx.setTXpower(0);   
    }
}

void txSetup()
{    
    char fStr[64];

    tx = Adafruit_Si4713(__FMTXRST_PIN);
    if(!tx.begin())
    {
        Serial.println("Couldn't find transmitter!!");
        hwError("No transmitter!");
    }

    Serial.print("Found FM transmitter Si47");
    Serial.println(tx.getRev());

    txSetFreq(promData.txFreq);

    txOn(promData.txState);

    // begin the RDS transmission
    tx.beginRDS();
    txSetRDSstationName("None");
    tx.setRDSbuffer("Ed Rixon, 2023");

    // 50us pre-emphasis
    tx.setProperty(SI4713_PROP_TX_PREEMPHASIS, PEMPH_50US);

    tx.setProperty(SI4713_PROP_TX_LINE_LEVEL_INPUT_LEVEL, INPUT_LEVEL);
    
    // audio deviation
    tx.setProperty(SI4713_PROP_TX_AUDIO_DEVIATION, AUDIO_DEV);

    // RDS deviation
    tx.setProperty(SI4713_PROP_TX_RDS_DEVIATION, RDS_DEV);

    // Pilot deviation
    tx.setProperty(SI4713_PROP_TX_PILOT_DEVIATION, PILOT_DEV);

    // RDS data block split
    // Send PS 87.5% of the time - default is 50%
    tx.setProperty(SI4713_PROP_TX_RDS_PS_MIX, 0x05);

    // RDS PI code
    txSetRDSpiCode(RDS_PI);

#ifdef __OVERMOD_LED
    // Over modulation LED
    pinMode(__LED_PIN, OUTPUT);
    digitalWrite(__LED_PIN, LOW);
#endif

    //txGetProperty(SI4713_PROP_TX_AUDIO_DEVIATION);

}

void txLoop()
{
    tx.readASQ();

#ifdef __OVERMOD_LED
    // overmod
    if((tx.currASQ & 0x04) == 0x04)
    {
        digitalWrite(__LED_PIN, HIGH);
    }
    else
    {
        digitalWrite(__LED_PIN, LOW);
    }
#endif
}
