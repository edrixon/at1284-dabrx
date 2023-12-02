#include <Arduino.h>

#include <Wire.h>

#include "dab1284.h"
#include "types.h"
#include "prom.h"

#ifdef __SIMULATE_PROM

#define PROMSIZE 512
unsigned char rawProm[PROMSIZE];

#else

#define PROMSIZE         8192
#define PROMDUMPSIZE      256  // limit for serial diags..
#define PROM_I2C_ADDRESS 0x50

#endif

extern PROMTYPE promData;
extern unsigned long int writePromMs;

//
// PROM has the address of a valid data structure at the start
// This is followed by one or more data structures
// As the PROM wears and writes fail, the address is increased by the size of a PROM data structure
// to point to a "new" area in the device which is used until it fails and the process repeats
// 

unsigned char readPromByte(unsigned int addr)
{
#ifdef __SIMULATE_PROM
    return rawProm[addr];
#else
   Wire.beginTransmission(PROM_I2C_ADDRESS);
 
   Wire.write((int)(addr >> 8));   // MSB
   Wire.write((int)(addr & 0xFF)); // LSB
 
   Wire.endTransmission();
 
   Wire.requestFrom(PROM_I2C_ADDRESS, 1);
 
   return Wire.read();
#endif
}

boolean writePromByte(unsigned int addr, unsigned char b)
{
    unsigned char promByte;
    boolean rtn;

    rtn = false;

    // Only write if new value is different to value already in PROM  
    promByte = readPromByte(addr);
    if(promByte != b)
    {
        // Write data byte

#ifdef __SIMULATE_PROM
        rawProm[addr] = b;
#else

        Wire.beginTransmission(PROM_I2C_ADDRESS);
 
        Wire.write((int)(addr >> 8));   // MSB
        Wire.write((int)(addr & 0xFF)); // LSB

        Wire.write(b);
 
        Wire.endTransmission();

        delay(5);
        
#endif

        // read it back, if it's the same, return true
        promByte = readPromByte(addr);
        if(promByte == b)
        {
            rtn = true;
        }
        else
        {
            Serial.println("");
            Serial.print("Byte write failed at ");
            Serial.println(addr, HEX);
        }
    }
    else
    {
        return true;
    }

    return rtn;
}

void dumpProm()
{
    unsigned int addr;
    char hexStr[10];
    int maxAddr;

#ifdef __SIMULATE_PROM
    maxAddr = PROMSIZE;
#else
    maxAddr = PROMDUMPSIZE;
#endif

    addr = 0;
    sprintf(hexStr, "%04x:  ", addr);
    Serial.print(hexStr);
    while(addr < maxAddr)
    {
        sprintf(hexStr, "%02x ", readPromByte(addr));
        Serial.print(hexStr);
        addr++;

        if(addr % 16 == 0)
        {
            Serial.println("");
            
            if(addr < maxAddr)
            {
                sprintf(hexStr, "%04x:  ", addr);
                Serial.print(hexStr);
            }
        }
    }
}

void writePromWord(unsigned int addr, unsigned int w)
{
    writePromByte(addr, (unsigned char)((w & 0xff00) >> 8));
    writePromByte(addr + 1, (unsigned char)(w & 0x00ff));
}

unsigned int readPromWord(unsigned int addr)
{
    unsigned int rtn;

    rtn = ((unsigned int)readPromByte(addr) << 8) | readPromByte(addr + 1);

    return rtn;
}

unsigned char promDataCs()
{
    unsigned char *bPtr;
    int dLength;
    unsigned char cSum;

    promData.cSum = 0;
    cSum = 0;
    dLength = sizeof(promData);
    bPtr = (unsigned char *)&promData;
    while(dLength)
    {
        cSum = cSum + *bPtr;
        bPtr++;
        dLength--;
    }

    return cSum;
}

void writeProm()
{
    unsigned int addr;
    unsigned char *bPtr;
    int dataLength;
    unsigned char cSum;
    char dbgTxt[80];
    
    Serial.println("Saving PROM data");
  
    writePromMs = 0;

    // Save promData into EEPROM

    // Calculate checksum
    cSum = promDataCs();
    promData.cSum = cSum;

    // Read data location
    addr = readPromWord(0);

    // Write and read back PROM data to that location
    // if fails, point to next PROM location and try again
    dataLength = sizeof(promData);
    bPtr = (unsigned char *)&promData;

    sprintf(dbgTxt, "Writing 0x%02x bytes starting at 0x%04x...\n", dataLength, addr);
    Serial.print(dbgTxt);

    while(dataLength)
    {
        if(writePromByte(addr, *bPtr) == false)
        {
            dataLength = sizeof(promData);
            bPtr = (unsigned char *)&promData;
            addr = readPromWord(0) + sizeof(promData);
            writePromWord(0, addr);

            sprintf(dbgTxt, "Starting again at 0x%04x...\n", addr);
            Serial.print(dbgTxt);
        }
        else
        {
            bPtr++;
            dataLength--;
            addr++;
        }
    }

    dumpProm();
}

void eraseProm()
{
    unsigned int addr;
    
    Serial.print("Loading PROM with 0xff.... ");
    for(addr = 0; addr < PROMSIZE; addr++)
    {
        writePromByte(addr, 0xff);
    }
    Serial.println("done");
    dumpProm();
}

void loadPromDefaults()
{
    Serial.println("Loading PROM defaults");
    
    // Load data structure with default values
    promData.fIndex = DEFAULT_DAB_FREQ;
    promData.serviceId = DEFAULT_DAB_SERVICEID;
    promData.fmFreq = DEFAULT_FM_FREQ;
    promData.band = DEFAULT_BAND;

    promData.locked = DEFAULT_LOCK;

    promData.volume = DEFAULT_VOLUME;

    // Initialise data start address in PROM
    writePromWord(0, 2);

    // Write data structure into PROM
    writeProm();
}

void clearProm()
{
    loadPromDefaults();
//    eraseProm();
}

void readProm()
{
    unsigned int addr;
    unsigned char *bPtr;
    int dLength;
    unsigned char readCsum;
    char dbgMsg[80];
    
    // Read promData from EEPROM
    // if data is invalid, load with defaults and write back to EEPROM

    Serial.println("Reading PROM...");

    // Read PROM data address
    addr = readPromWord(0);
    if(addr == 0 || addr > PROMSIZE - sizeof(promData))
    {
        Serial.println("Invalid data address");

        // in case PROM is just blank, try loading defaults
        loadPromDefaults();
    }
    else
    {
        sprintf(dbgMsg, "Trying data at 0x%04x... ", addr);
        Serial.print(dbgMsg);
      
        // Read PROM data from that location
        dLength = sizeof(promData);
        bPtr = (unsigned char *)&promData;
        while(dLength)
        {
            *bPtr = readPromByte(addr);
            addr++;
            dLength--;
            bPtr++;
        }

        // Validate
        readCsum = promData.cSum;
        if(promDataCs() != readCsum)
        {
            Serial.println("Bad checksum");
            loadPromDefaults();
        }
        else
        {
            Serial.println("PROM data is valid");
        }
    }
}

void savePromData()
{
    char dbgMsg[80];

    // Called to schedule a PROM update
    // Every new request cancels previous one and restarts the schedule timer
    // Avoids excessive PROM writing (eg every service selection change) 

    sprintf(dbgMsg, "PROM write scheduled in %d seconds\n", WRITE_SECS);
    Serial.print(dbgMsg);

    // Start EEPROM write timer
    writePromMs = millis();
}
