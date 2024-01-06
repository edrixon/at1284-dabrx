void txSetFreq(unsigned int txFreq);
void txSetup();
void txLoop();
void txOn(boolean turnOn);
void txSetRDSstationName(char *ps);
void txSetRDSpiCode(unsigned short int pi);
void txSetRDSptype(unsigned char pty);

#define PEMPH_50US     1      // 50us pre-emphasis - magic number from datasheet
#define AUDIO_DEV      65000  // deviation values in Hz - total 75kHz
#define RDS_DEV        2500
#define PILOT_DEV      7500
#define INPUT_LEVEL    0x3100 // 250mV p-p input

#define RDS_PI         0xbeef // Gets changed to match DAB service ID
