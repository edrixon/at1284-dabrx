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

void txSetRDSptype(unsigned char pty)
{
    unsigned long int txProp;
    unsigned long int ptyMask;

    ptyMask = (unsigned int)pty;
    ptyMask = ptyMask << 5;

    // Mask out PTY from property value - 1111 1100 0001 1111
    txProp = txGetProperty(SI4713_PROP_TX_RDS_PS_MISC) & 0xfc1f;
    txProp = txProp | ptyMask;

    tx.setProperty(SI4713_PROP_TX_RDS_PS_MISC, txProp);
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
    unsigned char rev;
        
    tx = Adafruit_Si4713(__FMTXRST_PIN);
    tx.begin();

    rev = tx.getRev();
    if(rev != 13)
    {
        Serial.println("Couldn't find transmitter!!");
        hwError("No transmitter!");
    }

    Serial.print("Found FM transmitter Si47");
    Serial.println(rev);

    txSetFreq(promData.txFreq);

    txOn(promData.txState);

    // begin the RDS transmission
    tx.beginRDS();

    //                   12345678
    txSetRDSstationName("NoneNone");
    tx.setRDSbuffer("Ed Rixon, 2023");

    // 50us pre-emphasis
    tx.setProperty(SI4713_PROP_TX_PREEMPHASIS, PEMPH_50US);

    tx.setProperty(SI4713_PROP_TX_LINE_LEVEL_INPUT_LEVEL, INPUT_LEVEL);

    // Deviation values required by Si4713 ar 10's of Hz
    // Values in header are in Hz, so divide them by 10
    
    // audio deviation
    tx.setProperty(SI4713_PROP_TX_AUDIO_DEVIATION, AUDIO_DEV / 10);

    // RDS deviation
    tx.setProperty(SI4713_PROP_TX_RDS_DEVIATION, RDS_DEV / 10);

    // Pilot deviation
    tx.setProperty(SI4713_PROP_TX_PILOT_DEVIATION, PILOT_DEV / 10);

    // RDS data block split
    // Send PS 87.5% of the time - default is 50%
    tx.setProperty(SI4713_PROP_TX_RDS_PS_MIX, 0x05);

    // Static PTY, not compressed, not artificial head, stereo      = 0001
    // FIFO and buffer use PTY and TP when written, no TP, PTY [4:3]= 0000
    // PTY[2:0], no TA                                              = 0000
    // Music, [2:0] = 0                                             = 1000
    tx.setProperty(SI4713_PROP_TX_RDS_PS_MISC, 0x1008);

    // RDS PI code
    txSetRDSpiCode(RDS_PI);

#ifdef __OVERMOD_LED
    // Over modulation LED
    pinMode(__LED_PIN, OUTPUT);
    digitalWrite(__LED_PIN, LOW);
#endif

    txSetFreq(promData.txFreq);

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
