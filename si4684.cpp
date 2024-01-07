//
// Extra Si468x functions not provided by dabshield library
//

#include <Arduino.h>
#include <Waveshare_LCD1602_RGB.h>
#include <SPI.h>
#include <DABShield.h>

#include "dab1284.h"
#include "types.h"
#include "si4684.h"
#include "fmtx.h"
#include "pins.h"

#define DAB_GET_ENSEMBLE_INFO  0xb4
#define DAB_GET_COMPONENT_INFO 0xbb
#define DAB_GET_SERVICE_INFO   0xc0

extern unsigned char spiBuf[];
extern unsigned char command_error;

extern Waveshare_LCD1602_RGB lcd;
extern DAB Dab;
extern PROMTYPE promData;
extern int serviceIndex;


static void si468x_responseN(int len)
{
    int i;

    for(i = 0; i < len + 1; i++)
    {
        spiBuf[i] = 0;
    }

    DABSpiMsg(spiBuf, len + 1);
}

static void si468x_response(void)
{
    si468x_responseN(4);
}

static void si468x_cts(void)
{
    uint32_t timeout;
    command_error = 0;
    timeout = 1000;
    do
    {
        delay(4);
        si468x_response();
        timeout--;
        if(timeout == 0)
        {
            command_error = 0x80;
            Serial.println("Command timedout");
            break;
        }
    }
    while((spiBuf[1] & 0x80) == 0x00);

    if((spiBuf[1] & 0x40) == 0x40)
    {
        si468x_responseN(5);
        command_error = 0x80 | spiBuf[5];
    }
}

void dumpSpiBuf(int c)
{
    unsigned char *bPtr;
    char txt[10];

    bPtr = spiBuf;
    while(c)
    {
        sprintf(txt, "%02x ", *bPtr);
        Serial.print(txt);

        c--;
        bPtr++;
    }
    Serial.println("");
}

void dabGetEnsembleInfo()
{
    Serial.println("dabGetEnsembleInfo()");
    
    spiBuf[0] = DAB_GET_ENSEMBLE_INFO;
    spiBuf[1] = 0x00;

    dumpSpiBuf(2);
    DABSpiMsg(spiBuf, 2);

    si468x_cts();
    dumpSpiBuf(4);
    
    si468x_responseN(26);
    dumpSpiBuf(26);
}

void dabGetServiceInfo(unsigned long int serviceId)
{
    spiBuf[0] = DAB_GET_SERVICE_INFO;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    spiBuf[4] = serviceId & 0xFF;
    spiBuf[5] = (serviceId >> 8) & 0xFF;
    spiBuf[6] = (serviceId >> 16) & 0xFF;
    spiBuf[7] = (serviceId >> 24) & 0xFF;

    //dumpSpiBuf(8);
    DABSpiMsg(spiBuf, 8);

    si468x_cts();
    //dumpSpiBuf(4);
    
    si468x_responseN(27);
    //dumpSpiBuf(27);
}

void dabGetShortLabel(unsigned long int serviceId, char *shortLabel, unsigned char *pty)
{
    int c;
    unsigned short int labelMask;

    dabGetServiceInfo(serviceId);

    *pty = (spiBuf[5] >> 1) & 0x1f;

    labelMask = spiBuf[25] + (spiBuf[26] << 8);

//                      12345678
    strcpy(shortLabel, "        ");

    c = 0;
    while(c < 16)
    {
        if((labelMask & 0x8000) == 0x8000)
        {
            *shortLabel = spiBuf[c + 9];
            shortLabel++;
        }

        labelMask = labelMask << 1;
        c++;
    }

    shortLabel[8] = '\0';
}

void DABSpiMsg(unsigned char *data, uint32_t len)
{
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite (__DABCS_PIN, LOW);
    SPI.transfer(data, len);
    digitalWrite (__DABCS_PIN, HIGH);
    SPI.endTransaction();
}

void startReceiver()
{
    if(promData.band == BAND_FM)
    {
        Dab.begin(1, __DABINTERRUPT_PIN, __DABRESET_PIN, __DABPWREN_PIN);
    }
    else
    {
        Dab.begin(0, __DABINTERRUPT_PIN, __DABRESET_PIN, __DABPWREN_PIN);
    }
}

boolean setFrequency()
{
    lcd.clear();
    lcd.send_string("Tuning...");
        
    Dab.tune(promData.fIndex);

    return Dab.servicevalid();
}

void startDabService()
{
    char txtBuff[64];
    char shortLabel[32];
    unsigned char pty;
    DABService *srv;
    
    Dab.set_service(serviceIndex);
    Dab.status();

    srv = &(Dab.service[serviceIndex]);

    dabGetShortLabel(srv -> ServiceID, shortLabel, &pty);
    txSetRDSstationName(shortLabel);
    txSetRDSpiCode((unsigned short int)((srv -> ServiceID) & 0xffff));
    txSetRDSptype(pty);
}

boolean setService()
{
    serviceIndex = 0;
    while(serviceIndex < Dab.numberofservices && Dab.service[serviceIndex].ServiceID != promData.serviceId)
    {
        serviceIndex++;
    }

    if(serviceIndex == Dab.numberofservices)
    {
        lcd.setCursor(0, 1);
        lcd.send_string("Station gone!");
        serviceIndex = VALUE_INVALID;
    }
    else
    {
        startDabService();
    }
}

void processServiceData(void)
{
//    Serial.println(Dab.ServiceData);
}

void rxSetup()
{
    pinMode(__DABCS_PIN, OUTPUT);
    digitalWrite(__DABCS_PIN, HIGH);
    SPI.begin();
    Dab.setCallback(processServiceData);
    startReceiver();

    if(Dab.error != 0)
    {
        Serial.println("Couldn't find receiver");
        hwError("No receiver!");
    }

    Serial.print("Found DAB receiver Si");
    Serial.println(Dab.PartNo);
 
    Serial.print("Using DABShield library version ");
    Serial.print(Dab.LibMajor);
    Serial.print(".");
    Serial.println(Dab.LibMinor);

    Dab.vol(promData.volume);

}

void rxLoop()
{
    Dab.task();
    Dab.status();
}
