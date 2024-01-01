//
// Extra Si468x functions not provided by dabshield library
//

#include <Arduino.h>

#include "dab1284.h"
#include "si4684.h"

#define DAB_GET_ENSEMBLE_INFO  0xb4
#define DAB_GET_COMPONENT_INFO 0xbb
#define DAB_GET_SERVICE_INFO   0xc0

extern unsigned char spiBuf[];
extern unsigned char command_error;

static void si468x_responseN(int len)
{
  int i;

  for (i = 0; i < len + 1; i++)
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
  while ((spiBuf[1] & 0x80) == 0x00);

  if ((spiBuf[1] & 0x40) == 0x40)
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

void dabGetShortLabel(unsigned long int serviceId, char *shortLabel)
{
    int c;
    unsigned short int labelMask;

    dabGetServiceInfo(serviceId);

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
