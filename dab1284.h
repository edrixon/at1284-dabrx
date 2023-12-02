void clearLine(int ln);

#define VALUE_INVALID -1

// How many seconds before writing PROM data
#define WRITE_SECS 20

// ms delay in main loop
#define TICKTIME 10

// How many ms for a "long" button press
#define LONGPRESS_MS 1000

// Limits for volume setting
#define MAX_VOLUME 63
#define MIN_VOLUME 0

// FM tuning parameters - frequencies x10kHz
#define FM_MIN     7600
#define FM_MAX    10800
#define FM_STEP       5

// Default values for EEPROM settings
#define DEFAULT_DAB_FREQ      29        // 225.648MHz - BBC DAB
#define DEFAULT_DAB_SERVICEID 0xc224    // Radio 4
#define DEFAULT_FM_FREQ       9280      // Radio 4, Douglas
#define DEFAULT_VOLUME        60
#define DEFAULT_BAND          BAND_DAB  // DAB mode
#define DEFAULT_LOCK          false     // Dial not locked

#define LOCK_CHR 0
#define RSSI_CHR 1
#define SNR_CHR  2
