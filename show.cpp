#include <Arduino.h>
#include <DABShield.h>
#include "Waveshare_LCD1602_RGB.h"

#include "dab1284.h"
#include "types.h"

extern int tmpVolume;
extern DAB Dab;
extern Waveshare_LCD1602_RGB lcd;
extern char *menuItems[];
extern char *serviceTypes[];
extern int menuStateId;
extern PROMTYPE promData;
extern int serviceIndex;
extern char *bandNames[];
extern BAND tmpBand;
extern boolean locked;
extern boolean tmpTxState;
extern int tmpTxFreq;

void showSignal()
{
    char sigInfoTxt[17];
    int rssi;
    int snr;
    int c;

    snr = map(Dab.snr, 0, 20, 0, 16);
    for(c = 0; c < snr; c++)
    {
          sigInfoTxt[c] = SNR_CHR;
    }

    while(c < 16)
    {
        sigInfoTxt[c] = ' ';
        c++;
    }

    rssi = map(Dab.signalstrength, 0, 63, 0, 16);
    sigInfoTxt[rssi] = RSSI_CHR;

    sigInfoTxt[16] = '\0';

    lcd.setCursor(0, 0);
    lcd.send_string(sigInfoTxt);
}    

void showLocked()
{
    lcd.setCursor(15, 0);
    if(promData.locked == true)
    {
        lcd.write_char(LOCK_CHR);
    }
    else
    {
        lcd.write_char(' ');
    }
}

void showBand()
{
    lcd.setCursor(0, 1);
    lcd.send_string(bandNames[tmpBand]);
}

void showVolume()
{
    char volStr[16];

    Serial.print("Volume = ");
    Serial.println(tmpVolume);

    clearLine(1);
    sprintf(volStr, "%d", tmpVolume);
    lcd.send_string(volStr);
}

void showService()
{
    char srvTxt[64];

    if(serviceIndex == VALUE_INVALID)
    {
          Serial.print("No service - serviceIndex = -1");
          strcpy(srvTxt, "No signal!");
    }
    else
    {
        Dab.status();

        sprintf(srvTxt, "Service type - %d - %s\n", Dab.type, serviceTypes[Dab.type]);
        Serial.print(srvTxt);
        
        sprintf(srvTxt, "%d - 0x%04lx - 0x%04lx - ", serviceIndex, promData.serviceId, Dab.service[serviceIndex].CompID);
        Serial.print(srvTxt);

        sprintf(srvTxt, "%16s", Dab.service[serviceIndex].Label);
        Serial.println(srvTxt);
    }
    
    lcd.setCursor(0, 1);
    lcd.send_string(srvTxt);
}       

void showMenuItem()
{
    Serial.print("Menu item ");
    Serial.print(menuStateId);
    Serial.print(" - ");
    Serial.println(menuItems[menuStateId]);
    
    lcd.setCursor(0, 1);
    lcd.send_string(menuItems[menuStateId]);
}

void showFmFrequency(int line)
{
    char freqTxt[17];
    unsigned long int kHz;

    kHz = (unsigned long int)promData.fmFreq * 10;

    sprintf(freqTxt, "FM %d.%03d MHz ", (unsigned int)(kHz / 1000), (unsigned int)(kHz % 1000));
    lcd.setCursor(0, 1);
    lcd.send_string(freqTxt);
    Serial.println(freqTxt);

    if(line == 0)
    {
        showLocked();
    }
}

void showFrequency(unsigned char f, int line)
{
    unsigned long int kHz;
    char freqTxt[17];

    kHz = Dab.freq_khz(f);
    sprintf(freqTxt, "%d%c %d.%03d MHz ", ((f / 4) + 5), ((f % 4) + 65), (unsigned int)(kHz / 1000), (unsigned int)(kHz % 1000));
    lcd.setCursor(0, line);
    lcd.send_string(freqTxt);

    if(line == 0)
    {
        showLocked();
    }
    
    Serial.println(freqTxt);
}

void showEnsemble()
{
    char ensembleTxt[17];

    if(Dab.servicevalid() == true)
    {
        sprintf(ensembleTxt, "%16s", Dab.Ensemble);
    }
    else
    {
//                           1234567890123456
        strcpy(ensembleTxt, "No DAB service! ");
    }

    lcd.setCursor(0, 1);
    lcd.send_string(ensembleTxt);
}

void showTxState()
{
    lcd.setCursor(0, 1);
    if(tmpTxState == true)
    {
        Serial.println("Transmitter ON");
        lcd.send_string("ON ");
    }
    else
    {
        Serial.println("Transmitter OFF");
        lcd.send_string("OFF");      
    }
}

void showTxFreq()
{
    char freqTxt[17];
    unsigned long int
    
    kHz = (unsigned long int)tmpTxFreq * 10;

    sprintf(freqTxt, "%d.%03d MHz ", (unsigned int)(kHz / 1000), (unsigned int)(kHz % 1000));
    lcd.setCursor(0, 1);
    lcd.send_string(freqTxt);
    Serial.println(freqTxt);
}
