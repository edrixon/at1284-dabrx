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
    lcd.setCursor(0, 0);
    lcd.send_string("Mode: ");
    lcd.send_string(bandNames[tmpBand]);
}

void showVolume()
{
    char volStr[16];

    Serial.print("Volume = ");
    Serial.println(tmpVolume);
      
    clearLine(0);
    sprintf(volStr, "Volume: %d", tmpVolume);
    lcd.send_string(volStr);
}

void showService()
{
    char srvTxt[17];

    if(serviceIndex == VALUE_INVALID)
    {
          Serial.print("No service - serviceIndex = -1");
          strcpy(srvTxt, "No signal!");
    }
    else
    {
        Dab.status();
        
        Serial.print(serviceIndex);
        Serial.print(" - ");
        Serial.print(promData.serviceId, HEX);
        Serial.print(" - ");
        Serial.println(Dab.service[serviceIndex].Label);

        Serial.print("Service type - ");
        Serial.print(Dab.type);
        Serial.print(" - ");
        Serial.println(serviceTypes[Dab.type]);

        sprintf(srvTxt, "%16s", Dab.service[serviceIndex].Label);
    }
    
    clearLine(1);
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

    sprintf(freqTxt, "%d.%03d MHz", (unsigned int)(kHz / 1000), (unsigned int)(kHz % 1000));
    clearLine(line);
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
    sprintf(freqTxt, "%d%c %d.%03d MHz", ((f / 4) + 5), ((f % 4) + 65), (unsigned int)(kHz / 1000), (unsigned int)(kHz % 1000));
    clearLine(line);
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
