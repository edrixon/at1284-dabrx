void txSetFreq(unsigned int txFreq);
void txSetup();
void txLoop();
void txOn(boolean turnOn);
void txSetRDSstationName(char *ps);
void txSetRDSpiCode(unsigned short int pi);


#define RDSBUFFERSIZE  16
#define RDSPAGESIZE    8
#define RDSPAGESHIFT   1
#define RDSPAGES       8
#define RDSREFRESHTIME 100    // 100's of milliseconds...

#define PEMPH_50US 1        // 50us pre-emphasis - magic number from datasheet
#define AUDIO_DEV  6500     // deviation values in 10's of kHz - total 75kHz
#define RDS_DEV    250
#define PILOT_DEV  750
#define INPUT_LEVEL 0x3100  // 250mV p-p input

#define RDS_PI   0xbeef
