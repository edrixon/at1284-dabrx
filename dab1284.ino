
#include <EnableInterrupt.h>
#include <SPI.h>
#include <DABShield.h>

#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"

#include "types.h"
#include "pins.h"
#include "show.h"
#include "dab1284.h"
#include "prom.h"

void selServiceEnter();
void selServiceExit();
void selServiceAction();
void selFreqEnter();
void selFreqExit();
void selFreqAction();
void selMenuEnter();
void selMenuExit();
void selMenuAction();
void selVolumeEnter();
void selVolumeExit();
void selVolumeAction();
void showStatusEnter();
void showStatusExit();
void showStatusAction();
void findEnsemblesEnter();
void findEnsemblesExit();
void findEnsemblesAction();
void selEnsembleEnter();
void selEnsembleExit();
void selEnsembleAction();
void showTimeEnter();
void showTimeExit();
void showTimeAction();
void clearPromEnter();
void clearPromExit();
void clearPromAction();
void setBandEnter();
void setBandExit();
void setBandAction();

STATEDEF stateDefs[] =
{
    { selServiceEnter, selServiceExit, selServiceAction, 0, 0 },
    { selMenuEnter, selMenuExit, selMenuAction, 10, STATE_SELSERVICE },
    { selFreqEnter, selFreqExit, selFreqAction, 10, STATE_SELSERVICE },
    { selVolumeEnter, selVolumeExit, selVolumeAction, 10, STATE_SELSERVICE },
    { showStatusEnter, showStatusExit, showStatusAction, 10, STATE_SELSERVICE },
    { findEnsemblesEnter, findEnsemblesExit, findEnsemblesAction, 120, STATE_SELSERVICE },
    { selEnsembleEnter, selEnsembleExit, selEnsembleAction, 10, STATE_SELSERVICE },
    { showTimeEnter, showTimeExit, showTimeAction, 10, STATE_SELSERVICE },
    { clearPromEnter, clearPromExit, clearPromAction, 10, STATE_SELSERVICE },
    { setBandEnter, setBandExit, setBandAction, 10, STATE_SELSERVICE }
};

char *menuItems[] =
{
//   1234567890123456
    "[Set frequency ]",
    "[Set volume    ]",
    "[Show status   ]",
    "[Find ensembles]",
    "[Set ensemble  ]",
    "[Show DAB time ]",
    "[Clear settings]",
    "[Set mode      ]",
    "[Exit menu     ]"
};

int numMenuItems = 9;
STATEID menuStates[] = 
{
    STATE_SELFREQ,
    STATE_SELVOLUME,
    STATE_SHOWSTATUS,
    STATE_FINDENSEMBLES,
    STATE_SELENSEMBLE,
    STATE_SHOWTIME, 
    STATE_CLEARPROM,
    STATE_SETBAND,
    STATE_SELSERVICE
};

char *serviceTypes[] =
{
    "none",
    "audio",
    "data"
};

unsigned char lockChrBmp[] = { 0x0e, 0x11, 0x11, 0x1f, 0x1b, 0x1b, 0x1f, 0x00 };  // A padlock
unsigned char rssiChrBmp[] = { 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f };  // An empty square
unsigned char snrChrBmp[] =  { 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00 };  // A solid block

int numBands = 2;
char *bandNames[] = { "DAB", "FM " };

char ensembles[DAB_FREQS][17];
int ensemblesFound;

Waveshare_LCD1602_RGB lcd(16, 2);
DAB Dab;
PROMTYPE promData;
STATEINFO curState;
int serviceIndex;
unsigned long int writePromMs;
unsigned long int writePromDelayMs;
unsigned long int showTimeMs;
int menuStateId;
int tmpVolume;
unsigned char tmpFreqIndex;
BAND tmpBand;
unsigned long int pressMs;

// Variables set in interrupt handlers 
volatile boolean encUp;
volatile boolean encClk;
volatile boolean encButton;

// Interrupt when the encoder push button is pressed
void encButtonInterruptHandler()
{
    encButton = true;
    pressMs = millis();
}

// Interrupt when the encoder is turned
void encInterruptHandler()
{
    if(promData.locked == false)
    {
        encClk = true;
        if(digitalRead(__ENCDT_PIN) == HIGH)
        {
            encUp = true;
        }
        else
        {
            encUp = false;
        }
    }
}

void resetStateTimeout()
{
    curState.lastMs = millis();
}

boolean buttonPressed()
{
    boolean rtn;
  
    rtn = false;
    if(encButton == true)
    {
        encButton = false;
        rtn = true;
    }
  
    return rtn;
}

boolean longPress()
{
    boolean rtn;

    rtn = false;

    if(pressMs != 0)
    {
        if(digitalRead(__ENCBUTTON_PIN) == LOW)
        {
            if(millis() - pressMs > LONGPRESS_MS)
            {
                encButton = false;
                rtn = true;
            }
        }
    }

    return rtn;
}

void clearLine(int ln)
{
    lcd.setCursor(0, ln);

//                   1234567890123456
    lcd.send_string("                ");
    lcd.setCursor(0, ln);
}

void hwError(char *str)
{
    lcd.setRGB(255, 0, 0);
    lcd.setCursor(0, 0);
    lcd.send_string(str);
    
    while(1);
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

void processServiceData(void)
{
//    Serial.println(Dab.ServiceData);
}

void selMenuEnter()
{
    Serial.println("Menu");

    lcd.send_string("Select option:-");
    
    menuStateId = 0;
    showMenuItem();
}

void selMenuExit()
{
}

void selMenuAction()
{
    if(encClk == true)
    {
        encClk = false;
        resetStateTimeout();
        if(encUp == true)
        {
            menuStateId++;
            if(menuStateId == numMenuItems)
            {
                menuStateId = 0;
            }
        }
        else
        {
            if(menuStateId == 0)
            {
                menuStateId = numMenuItems - 1;
            }
            else
            {
                menuStateId--;
            }
        }
        
        showMenuItem();
          
    }

    if(buttonPressed() == true)
    {
        changeState(menuStates[menuStateId]);
    }
  
}

void selVolumeEnter()
{  
    Serial.println("Set volume");
    tmpVolume = promData.volume;
    showVolume();
}

void selVolumeExit()
{
    if(tmpVolume != promData.volume)
    {
        promData.volume = tmpVolume;
        savePromData();
    }
    else
    {
        Serial.println("No change");
    }
}

void selVolumeAction()
{
    if(encClk == true)
    {
        encClk = false;
        resetStateTimeout();
        if(encUp == true)
        {
            if(tmpVolume < MAX_VOLUME)
            {
                tmpVolume++;
            }
        }
        else
        {
            if(tmpVolume > MIN_VOLUME)
            {
                tmpVolume--;
            }
        }

        showVolume();
        Dab.vol(tmpVolume);

    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }  
}

void selFmFrequencyEnter()
{
    showFmFrequency(1);
}

void selFmFrequencyExit()
{
    savePromData();
}

void selFmFrequencyAction()
{ 
    if(encClk == true)
    {
        encClk = false;
        resetStateTimeout();

        if(encUp == true)
        {
            promData.fmFreq = promData.fmFreq + FM_STEP;
            if(promData.fmFreq > FM_MAX)
            {
                promData.fmFreq = FM_MIN; 
            }
        }
        else
        {
            if(promData.fmFreq > FM_MIN)
            {
                promData.fmFreq = promData.fmFreq - FM_STEP;
            }
            else
            {
                promData.fmFreq = FM_MAX;
            }
        }

        showFmFrequency(1);
        Dab.tune(promData.fmFreq);
    }
}

void selServiceEnter()
{
    Serial.println("Select service");

    if(promData.band == BAND_FM)
    {
        showFmFrequency(1);
    }
    else
    {
        showFrequency(promData.fIndex, 0);
        showService();
    }
}

void selServiceExit()
{
}

void selServiceAction()
{
    if(promData.band == BAND_FM)
    {
        selFmFrequencyAction();
    }
    else
    {
        if(encClk == true && Dab.numberofservices > 0)
        {
            encClk = false;
            if(encUp == true)
            {
                serviceIndex++;
                if(serviceIndex == Dab.numberofservices)
                {
                    serviceIndex = 0;
                }
            }
            else
            {
                if(serviceIndex == 0)
                {
                    serviceIndex = Dab.numberofservices - 1;
                }
                else
                {
                    serviceIndex--;
                }
            }

            promData.serviceId = Dab.service[serviceIndex].ServiceID;
            Dab.set_service(serviceIndex);

            showService();

            savePromData();
        }
    }

    if(digitalRead(__ENCBUTTON_PIN) == LOW)
    {
        if(longPress() == true)
        {
            pressMs = 0;
            if(promData.locked == true)
            {
                Serial.println("UNLOCKED!");
                promData.locked = false;
            }
            else
            {
                Serial.println("LOCKED!");
                promData.locked = true;
            }
            
            showLocked();
        }
    }
    else
    {
        if(promData.locked == false)
        {
            if(buttonPressed() == true)
            {
                changeState(STATE_SELMENU);
            }
        }
    }
}

void selFreqEnter()
{
    Serial.println("Select frequency");

    lcd.setCursor(0, 0);
    lcd.send_string("Tune:-");

    if(promData.band == BAND_FM)
    {
        selFmFrequencyEnter();
    }
    else
    {
        tmpFreqIndex = promData.fIndex;
        showFrequency(tmpFreqIndex, 1);
    }
}

void selFreqExit()
{
    if(promData.band == BAND_FM)
    {
        selFmFrequencyExit();
    }
    else
    {
        if(promData.fIndex != tmpFreqIndex)
        {
            promData.fIndex = tmpFreqIndex;

            setFrequency();

            if(Dab.servicevalid() == true)
            {
                serviceIndex = 0;
                do
                {
                    Dab.set_service(serviceIndex);
                    Dab.status();
                    if(Dab.type == SERVICE_DATA)
                    {
                        serviceIndex++;
                    }
                }
                while(Dab.type == SERVICE_DATA);
            }
            else
            {
                serviceIndex = VALUE_INVALID;
            }

            savePromData();
        }
    }
}

void selFreqAction()
{
    if(promData.band == BAND_FM)
    {
        selFmFrequencyAction();
    }
    else
    {
        if(encClk == true)
        {
            encClk = false;
            resetStateTimeout();

            if(encUp == true)
            {
                tmpFreqIndex++;
                if(tmpFreqIndex == DAB_FREQS)
                {
                    tmpFreqIndex = 0; 
                }
            }
            else
            {
                if(tmpFreqIndex > 0)
                {
                    tmpFreqIndex--;
                }
                else
                {
                    tmpFreqIndex = DAB_FREQS - 1;
                }
            }

            showFrequency(tmpFreqIndex, 1);
        }
    }
        
    if(buttonPressed() == true)
    {
        changeState(STATE_SHOWSTATUS);
    }
}

void showStatusEnter()
{
    Serial.println("Show status");
    showEnsemble(); 
}

void showStatusExit()
{  
}

void showStatusAction()
{
    char sigInfoTxt[17];
    int rssi;
    int snr;
    int c;

    Dab.status();

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
    
    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }    
}

void findEnsemblesEnter()
{
    Serial.println("Find ensembles");
    Serial.print("Checking ");
    Serial.print(DAB_FREQS);
    Serial.println(" channels");
  
    tmpFreqIndex = 0;
    ensemblesFound = 0;
}

void findEnsemblesExit()
{
    Serial.print("Found ");
    Serial.print(ensemblesFound);
    Serial.println(" ensembles");

    setFrequency();

    if(Dab.servicevalid() == true)
    {
        serviceIndex = 0;
        do
        {
            Dab.set_service(serviceIndex);
            Dab.status();
            if(Dab.type == SERVICE_DATA)
            {
                serviceIndex++;
            }
        }
        while(Dab.type == SERVICE_DATA);
    }
    else
    {
        serviceIndex = VALUE_INVALID;
    }
}

void findEnsemblesAction()
{ 
    Serial.print("Frequency index - ");
    Serial.println(tmpFreqIndex);

    Serial.print("   ");
    showFrequency(tmpFreqIndex, 0);
    Dab.tune(tmpFreqIndex);

    Serial.print("   ");
    if(Dab.servicevalid() == true)
    {
        ensemblesFound++;
        
        Serial.println(Dab.Ensemble); 
        strcpy(ensembles[tmpFreqIndex], Dab.Ensemble);
    }
    else
    {
        Serial.println("No DAB service");
    }
    
    tmpFreqIndex++;
    if(tmpFreqIndex == DAB_FREQS)
    {
        changeState(STATE_SELSERVICE);
    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }
 }

void selEnsembleEnter()
{
    Serial.println("Select ensemble");
  
    if(ensemblesFound == 0)
    {
        lcd.send_string("List empty");
    }
    else
    {
        lcd.send_string("Ensemble:-");
        
        tmpFreqIndex = 0;
        while(ensembles[tmpFreqIndex][0] == '\0')
        {
            tmpFreqIndex++;
        }
        lcd.setCursor(0, 1);
        lcd.send_string(ensembles[tmpFreqIndex]);
    }
}

void selEnsembleExit()
{
    if(ensemblesFound > 0)
    {
        serviceIndex = 0;
        promData.fIndex = tmpFreqIndex;

        setFrequency();

        if(Dab.servicevalid() == true)
        {
            do
            {
                Dab.set_service(serviceIndex);
                Dab.status();
                if(Dab.type == SERVICE_DATA)
                {
                    serviceIndex++;
                }
            }
            while(Dab.type == SERVICE_DATA);
        }
    }
}

void selEnsembleAction()
{
    if(ensemblesFound > 0)
    {
        if(encClk == true)
        {
            encClk = false;
            
            Serial.print("From ");
            Serial.println(tmpFreqIndex);
            
            if(encUp == true)
            {
                Serial.print("Up to - ");
                do
                {
                    tmpFreqIndex++;
                    if(tmpFreqIndex == DAB_FREQS)
                    {
                        tmpFreqIndex = 0;
                    }
                }
                while(ensembles[tmpFreqIndex][0] == '\0');
            }
            else
            {   
                Serial.print("Down to - ");
                do
                {
                    if(tmpFreqIndex == 0)    
                    {
                        tmpFreqIndex = DAB_FREQS - 1;
                    }
                    else
                    {
                        tmpFreqIndex--;
                    }
                }
                while(ensembles[tmpFreqIndex][0] == '\0');
            }

            Serial.println(tmpFreqIndex);

            lcd.setCursor(0, 1);
            lcd.send_string(ensembles[tmpFreqIndex]);
        } 

        if(buttonPressed() == true)
        {
            changeState(STATE_SHOWSTATUS);
        }
    }
    else
    {
        if(buttonPressed() == true)
        {
            if(promData.locked == false)
            {
                changeState(STATE_SELSERVICE);
            }
        }
    }
}

void clearPromEnter()
{
    lcd.send_string("Loaded defaults"); 
}

void clearPromExit()
{
}

void clearPromAction()
{
    clearProm();
    
    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }  
}

void enterState(STATEID stateId)
{ 
    Serial.print("Enter state - ");
    Serial.println(stateId);

    curState.stateDef = &stateDefs[stateId];
    
    Serial.print("Timeout = ");
    Serial.print(curState.stateDef -> timeoutSecs);
    Serial.println(" seconds");    
     
    curState.timeoutMs = (curState.stateDef -> timeoutSecs * 1000);
    resetStateTimeout();

    delay(300);
    encClk = false;
    encButton = false;  

    lcd.clear();
    
    curState.stateDef -> enterFn();
}

void showTimeEnter()
{
    Serial.println("Show time");
    showTimeMs = 0; 
}

void showTimeExit()
{
}

void showTimeAction()
{
    char timestring[16];
    DABTime dabTime;

    if(millis() - showTimeMs > 500)
    {
        showTimeMs = millis();
        Dab.time(&dabTime);
    
        sprintf(timestring,"%02d/%02d/%02d ", dabTime.Days, dabTime.Months, dabTime.Year);
        lcd.setCursor(0, 0);
        lcd.send_string(timestring);
    
        sprintf(timestring,"%02d:%02d", dabTime.Hours, dabTime.Minutes);
        lcd.setCursor(0, 1);
        lcd.send_string(timestring);
    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }    
}

void setBandEnter()
{
    Serial.print("Set band");
    tmpBand = promData.band;
    showBand();
}

void setBandExit()
{
    if(tmpBand != promData.band)
    {
        promData.band = tmpBand;
        savePromData();
        startReceiver();
        if(promData.band == BAND_FM)
        {        
            Dab.tune(promData.fmFreq);
        }
        else
        {
            setFrequency();
            Dab.set_service(serviceIndex);
        }
    }
}

void setBandAction()
{
    if(encClk == true)
    {
        encClk = false;
        resetStateTimeout();
        if(encUp == true)
        {
            tmpBand = tmpBand + 1;
            if(tmpBand == numBands)
            {
                tmpBand = 0;
            }
        }
        else
        {
            if(tmpBand == 0)
            {
                tmpBand = numBands - 1;
            }
            else
            {
                tmpBand = tmpBand - 1;
            }
        }
        
        showBand();
          
    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }
}

void changeState(STATEID newState)
{
    curState.stateDef -> exitFn();

    enterState(newState);
}


void setup()
{
    Serial.begin(9600);
    Serial.print("\n\n\n\n\n\n");
    Serial.println("ATmega1284P DAB receiver\n\n");
    
    lcd.init();
    lcd.setColorWhite();
    lcd.send_string("Starting...");
    lcd.customSymbol(LOCK_CHR, lockChrBmp);
    lcd.customSymbol(RSSI_CHR, rssiChrBmp);
    lcd.customSymbol(SNR_CHR, snrChrBmp);

    readProm();

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

    Serial.print("Found Si");
    Serial.println(Dab.PartNo);

    Serial.print("Using DABShield library version ");
    Serial.print(Dab.LibMajor);
    Serial.print(".");
    Serial.println(Dab.LibMinor);

    lcd.clear();

    Dab.vol(promData.volume);

    if(promData.band == BAND_FM)
    {
        Dab.tune(promData.fmFreq);  
    }
    else
    {
        if(setFrequency() == true)
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
                Dab.set_service(serviceIndex);
            }
        }
        else
        {
            serviceIndex = VALUE_INVALID;
            lcd.setCursor(0, 1);
            lcd.send_string("No DAB service!");
        }
    }    

    writePromDelayMs = WRITE_SECS * 1000;
    writePromMs = 0;

    pinMode(__ENCBUTTON_PIN, INPUT);
    enableInterrupt(__ENCBUTTON_PIN, encButtonInterruptHandler, FALLING);

    pinMode(__ENCDT_PIN, INPUT);
    pinMode(__ENCCLK_PIN, INPUT);
    enableInterrupt(__ENCCLK_PIN, encInterruptHandler, FALLING);

    enterState(STATE_SELSERVICE);
}

void loop()
{
    unsigned long int nowMs;
    
    Dab.task();

    curState.stateDef -> actionFn();

    delay(TICKTIME);

    nowMs = millis();

    // If this state can timeout
    if(curState.timeoutMs > 0)
    {
        // See if it's timedout
        if(nowMs - curState.lastMs > curState.timeoutMs)
        {
            Serial.println("State timeout");
            changeState(curState.stateDef -> timeoutState);
        }
    } 

    // If there's a pending PROM write
    if(writePromMs > 0)
    {
        // See if it's time to write... 
        if(nowMs - writePromMs > writePromDelayMs)
        {
            writeProm();
        }
    }
}
