#include <Arduino.h>
#include <Waveshare_LCD1602_RGB.h>

#include "dab1284.h"
#include "types.h"
#include "state.h"

extern Waveshare_LCD1602_RGB lcd;
extern STATEINFO curState;
extern STATEDEF stateDefs[];
extern volatile boolean encUp;
extern volatile boolean encClk;
extern volatile boolean encButton;

void resetStateTimeout()
{
    curState.lastMs = millis();
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

    setLcdColour();
    lcd.clear();
    
    curState.stateDef -> enterFn();
}

void changeState(STATEID newState)
{
    curState.stateDef -> exitFn();

    enterState(newState);
}
