
#include <EnableInterrupt.h>
#include <SPI.h>
#include <DABShield.h>

#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"

#include "dab1284.h"
#include "types.h"
#include "pins.h"
#include "show.h"
#include "prom.h"
#include "fmtx.h"
#include "si4684.h"
#include "si4713.h"
#include "state.h"

void selColourEnter();
void selColourExit();
void selColourAction();
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
void setFmTxEnter();
void setFmTxExit();
void setFmTxAction();
void setFmTxFreqEnter();
void setFmTxFreqExit();
void setFmTxFreqAction();

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
    { setBandEnter, setBandExit, setBandAction, 10, STATE_SELSERVICE },
    { selColourEnter, selColourExit, selColourAction, 10, STATE_SELSERVICE },
    { selColourEnter, selColourExit, selColourAction, 10, STATE_SELSERVICE },
    { selColourEnter, selColourExit, selColourAction, 10, STATE_SELSERVICE },
    { setFmTxEnter, setFmTxExit, setFmTxAction, 10, STATE_SELSERVICE },
    { setFmTxFreqEnter, setFmTxFreqExit, setFmTxFreqAction, 10, STATE_SELSERVICE },
};

char *menuItems[] =
{
//   1234567890123456
    "[Set frequency ]",
    "[Set volume    ]",
    "[Set mode      ]",
    "[Set ensemble  ]",
    "[Set red       ]",
    "[Set green     ]",
    "[Set blue      ]",
    "[TX frequency  ]",
    "[FM transmitter]",
    "[Find ensembles]",
    "[Clear settings]",
    "[Show DAB time ]",
    "[Show status   ]",
    "[Exit menu     ]"
};

int numMenuItems = 14;
STATEID menuStates[] = 
{
    STATE_SELFREQ,
    STATE_SELVOLUME,
    STATE_SETBAND,
    STATE_SELENSEMBLE,
    STATE_SELRED,
    STATE_SELGREEN,
    STATE_SELBLUE,
    STATE_SETFMTXFREQ,
    STATE_SETFMTX,
    STATE_FINDENSEMBLES,
    STATE_CLEARPROM,
    STATE_SHOWTIME, 
    STATE_SHOWSTATUS,
    STATE_SELSERVICE
};

char *serviceTypes[] =
{
    "none",
    "audio",
    "data"
};

unsigned char lockChrBmp[] =  { 0x0e, 0x11, 0x11, 0x1f, 0x1b, 0x1b, 0x1f, 0x00 };  // A padlock
unsigned char rssiChrBmp[] =  { 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f };  // An empty square
unsigned char snrChrBmp[] =   { 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00 };  // A solid block
unsigned char writeChrBmp[] = { 0x1f, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x00 };

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
boolean lowSignal;
unsigned char *colourAddr;
int fmMax;
int fmMin;
int fmStep;
boolean topLineCleared;
boolean ledState;
int tmpTxFreq;
int tmpTxState;

// Variables set in interrupt handlers 
volatile boolean encUp;
volatile boolean encClk;
volatile boolean encButton;

void doLed()
{
    if(ledState == true)
    {
        ledState = false;
        digitalWrite(__LED_PIN, LED_OFF);
    }
    else
    {
        ledState = true;
        digitalWrite(__LED_PIN, LED_ON);
    }
}

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

boolean encoderTurned()
{
    boolean rtn;

    rtn = false;
    if(encClk == true)
    {
        resetStateTimeout();
        resetPromWriteTimer();
        encClk = false;
        rtn = true;
    }

    return rtn;
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

void setLcdColour()
{
    lcd.setRGB(promData.redLevel, promData.greenLevel, promData.blueLevel);
}

void selColourEnter()
{
    char clrMsg[20];

    switch(menuStates[menuStateId])
    {
        case STATE_SELRED:
            colourAddr = &promData.redLevel;
            lcd.send_string("RED level:-");
            break;

        case STATE_SELGREEN:
            colourAddr = &promData.greenLevel;
            lcd.send_string("GREEN level:-");
            break;

        default:
            colourAddr = &promData.blueLevel;
            lcd.send_string("BLUE level:-");
    }

    lcd.setCursor(0, 1);
    sprintf(clrMsg, "%d", *colourAddr);
    lcd.send_string(clrMsg);
    
}

void selColourExit()
{
}

void selColourAction()
{
    char clrMsg[32];
    
    if(encoderTurned() == true)
    {
        if(encUp == true)
        {
            if(*colourAddr < 255)
            {
                *colourAddr = *colourAddr + 5;
            }
        }
        else
        {
            if(*colourAddr > 0)
            {
                *colourAddr = *colourAddr - 5;
            }
        }

        setLcdColour();

        lcd.setCursor(0, 1);
        sprintf(clrMsg, "%d  ", *colourAddr);
        lcd.send_string(clrMsg);

        sprintf(clrMsg, "R = %d, G = %d, B = %d", promData.redLevel, promData.greenLevel, promData.blueLevel);
        Serial.println(clrMsg);
        
        savePromData();
    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }
}

void setFmTxEnter()
{
    Serial.println("FM transmitter");
    
//                   1234567890123456
    lcd.send_string("Transmitter:-   ");
    tmpTxState = promData.txState;
    showTxState();
}

void setFmTxExit()
{
    if(tmpTxState != promData.txState)
    {
        txOn(tmpTxState);
        promData.txState = tmpTxState;
        savePromData();
    }
}

void setFmTxAction()
{
    if(encoderTurned() == true)
    {
        if(tmpTxState == true)
        {
            tmpTxState = false;
        }
        else
        {
            tmpTxState = true;
        }
        showTxState();
    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }    
}

void setFmTxFreqEnter()
{
    Serial.println("Set transmit frequency");
  
//                   1234567890123456
    lcd.send_string("TX frequency:-  ");
    tmpTxFreq = promData.txFreq;
    showTxFreq();  
}

void setFmTxFreqExit()
{
    if(tmpTxFreq != promData.txFreq)
    {
        txSetFreq(tmpTxFreq);
        promData.txFreq = tmpTxFreq;
        savePromData();
    }
}

void setFmTxFreqAction()
{
    if(encoderTurned() == true)
    {
        if(encUp == true)
        {
            tmpTxFreq = tmpTxFreq + fmStep;
            if(tmpTxFreq > fmMax)
            {
                tmpTxFreq = fmMin; 
            }
        }
        else
        {
            if(tmpTxFreq > fmMin)
            {
                tmpTxFreq = tmpTxFreq - fmStep;
            }
            else
            {
                tmpTxFreq = fmMax;
            }
        }

        showTxFreq();
    }

    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }
}

void selMenuEnter()
{
    Serial.println("Menu");

    lcd.send_string("Option:-");
    
    menuStateId = 0;
    showMenuItem();
}

void selMenuExit()
{
}

void selMenuAction()
{
    if(encoderTurned() == true)
    {
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

    lcd.send_string("Volume:-");
    
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
    if(encoderTurned() == true)
    {
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
    topLineCleared = false;
}

void selFmFrequencyExit()
{
}

void selFmFrequencyAction()
{
    if(encoderTurned() == true)
    {
        if(encUp == true)
        {
            promData.fmFreq = promData.fmFreq + fmStep;
            if(promData.fmFreq > fmMax)
            {
                promData.fmFreq = fmMin; 
            }
        }
        else
        {
            if(promData.fmFreq > fmMin)
            {
                promData.fmFreq = promData.fmFreq - fmStep;
            }
            else
            {
                promData.fmFreq = fmMax;
            }
        }

        if(topLineCleared == false)
        {
            topLineCleared = true;
            clearLine(0);
        }
        
        showFmFrequency(1);
        Dab.tune(promData.fmFreq);
        savePromData();
    }
    else
    {
        topLineCleared = false;
        showSignal();
    }  
}

void selServiceEnter()
{
    Serial.println("Select service");

    if(promData.band == BAND_FM)
    {
        selFmFrequencyEnter();
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

            do
            {
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
                startDabService();
            }
            while(Dab.type == SERVICE_DATA);

            showService();

            savePromData();
        }
  
        if(Dab.quality < MIN_QUALITY)
        {
            if(lowSignal == false)
            {
                lcd.setRGB(255, 0, 0);
                lcd.setCursor(0, 1);
                //               01234567890123456
                lcd.send_string(">> LOW SIGNAL << ");
                lowSignal = true;
            }
        }
        else
        {
            if(lowSignal == true)
            {
                lowSignal = false;
                setLcdColour();
                if(Dab.numberofservices == 0)
                {
                    Dab.tune(promData.fIndex);
                    if(Dab.servicevalid() == true)
                    {
                        setService();
                        showService();
                    }
                }
                else
                {
                    showService();
                }
            }
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

            savePromData();
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

    if(promData.band == BAND_FM)
    {
        changeState(STATE_SELSERVICE);
    }
    else
    {
        lcd.setCursor(0, 0);
        lcd.send_string("Tune:-");

        tmpFreqIndex = promData.fIndex;
        showFrequency(tmpFreqIndex, 1);
    }
}

void selFreqExit()
{
    int c;
    char srvMsg[32];

    if(promData.band == BAND_FM)
    {
        selFmFrequencyExit();
    }
    else
    {
        if(promData.fIndex != tmpFreqIndex)
        {
            promData.fIndex = tmpFreqIndex;
            savePromData();
        }

        setFrequency();

        if(Dab.servicevalid() == true)
        {
            serviceIndex = 0;
            do
            {
                startDabService();
                if(Dab.type == SERVICE_DATA)
                {
                    serviceIndex++;
                }
            }
            while(Dab.type == SERVICE_DATA);

            Serial.print("Ensemble name - ");
            Serial.println(Dab.Ensemble);

            sprintf(srvMsg, "Found %d services\n", Dab.numberofservices);                
            Serial.print(srvMsg);

            //              1--0x1234--1234567890123456
            Serial.println("   SID     Name");
            Serial.println("-----------------------------");
            for(c = 0; c < Dab.numberofservices; c++)
            {
                sprintf(srvMsg, "%d  0x%04lx  %s\n", c, Dab.service[c].ServiceID, Dab.service[c].Label);
                Serial.print(srvMsg);
            }
        }
        else
        {
            serviceIndex = VALUE_INVALID;
            Serial.println("No DAB service!");
        }
        
        Serial.println("");
    }
}

void selFreqAction()
{
    if(encoderTurned() == true)
    {
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

    if(buttonPressed() == true)
    {
        changeState(STATE_SHOWSTATUS);
    }
}

void showStatusEnter()
{
    Serial.println("Show status");
    if(promData.band == BAND_DAB)
    {
        showEnsemble();
    }
    else
    {
        showFmFrequency(1);
    }
}

void showStatusExit()
{  
}

void showStatusAction()
{
    showSignal();

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
            startDabService();
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
                startDabService();
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
        if(encoderTurned() == true)
        {
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
    clearProm();
}

void clearPromExit()
{
}

void clearPromAction()
{
    if(buttonPressed() == true)
    {
        changeState(STATE_SELSERVICE);
    }  
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
#ifdef __ENABLE_FMRX
    Serial.print("Set band");
    tmpBand = promData.band;
    lcd.send_string("Mode:-");
    showBand();
#endif
}

void setBandExit()
{
#ifdef __ENABLE_FMRX
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
            setService();
        }
    }
#endif
}

void setBandAction()
{
#ifdef __ENABLE_FMRX
    if(encoderTurned() == true)
    {
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
#else
    changeState(STATE_SELSERVICE);
#endif
}

void setup()
{
    Serial.begin(9600);
    Serial.print("\n\n\n\n\n\n");
    Serial.println("ATmega1284P DAB receiver\n\n");
    
    // Calls Wire.begin() so needs to be before any use of I2C PROM!
    lcd.init();

    readProm();
#ifndef __ENABLE_FMRX
    promData.band = BAND_DAB;
#endif

    setLcdColour();
    lcd.send_string("Starting...");
    lcd.customSymbol(LOCK_CHR, lockChrBmp);
    lcd.customSymbol(RSSI_CHR, rssiChrBmp);
    lcd.customSymbol(SNR_CHR, snrChrBmp);
    lcd.customSymbol(WRITE_CHR, writeChrBmp);

    rxSetup();
    txSetup();

    lcd.clear();
    if(promData.band == BAND_FM)
    {
        Dab.tune(promData.fmFreq);  
    }
    else
    {
        if(setFrequency() == true)
        {
            setService();
        }
    }    

    writePromDelayMs = WRITE_SECS * 1000;
    writePromMs = 0;

    lowSignal = false;

    fmMin = FM_MIN / 10;
    fmMax = FM_MAX / 10;
    fmStep = FM_STEP / 10;

    pinMode(__ENCBUTTON_PIN, INPUT);
    enableInterrupt(__ENCBUTTON_PIN, encButtonInterruptHandler, FALLING);

    pinMode(__ENCDT_PIN, INPUT);
    pinMode(__ENCCLK_PIN, INPUT);
    enableInterrupt(__ENCCLK_PIN, encInterruptHandler, FALLING);

    pinMode(__LED_PIN, OUTPUT);

    enterState(STATE_SELSERVICE);
}

void loop()
{
    unsigned long int nowMs;

    rxLoop();
    txLoop();

    curState.stateDef -> actionFn();

    delay(TICKTIME);

    doLed();

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
            lcd.setCursor(15, 0);
            lcd.write_char(WRITE_CHR);
            writeProm();
            showLocked();
        }
    }
}
