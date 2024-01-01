void clearLine(int ln);
void hwError(char *str);
void DABSpiMsg(unsigned char *data, uint32_t len);

//#define __HARDWARE_V10
#define __HARDWARE_V11

#define VALUE_INVALID         -1

// How many seconds before writing PROM data
#define WRITE_SECS            20

// ms delay in main loop
#define TICKTIME              10

// How many ms for a "long" button press
#define LONGPRESS_MS          1000

// Limits for volume setting
#define MAX_VOLUME            63
#define MIN_VOLUME            0

// FM tuning parameters - frequencies in kHz
#define FM_MIN                76000     // 76 MHz   
#define FM_MAX                108000    // 108 MHz
#define FM_STEP               100       // 100 kHz

// Default values for EEPROM settings
#define DEFAULT_DAB_FREQ      29        // 225.648MHz - BBC DAB
#define DEFAULT_DAB_SERVICEID 0xc224    // Radio 4
#define DEFAULT_FM_FREQ       9280      // Radio 4, Douglas
#define DEFAULT_VOLUME        60
#define DEFAULT_BAND          BAND_DAB  // DAB mode
#define DEFAULT_LOCK          false     // Dial not locked
#define DEFAULT_RED           255       // Red level in backlight
#define DEFAULT_GREEN         255       // Green level
#define DEFAULT_BLUE          0         // Blue level
#define DEFAULT_TXFREQ        8750      // 87.5 MHz
#define DEFAULT_TXSTATE       true      // Transmitter enabled

// Custom LCD characters
#define LOCK_CHR              0
#define RSSI_CHR              1
#define SNR_CHR               2
#define WRITE_CHR             3

// Value of "quality" which causes "low signal" message
#define MIN_QUALITY           5

// Un-comment to enable FM band reception
#define __ENABLE_FMRX

// Values to write to turn LED on or off
#define LED_ON               1
#define LED_OFF              0
