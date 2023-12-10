enum STATEID
{
    STATE_SELSERVICE,
    STATE_SELMENU,
    STATE_SELFREQ,
    STATE_SELVOLUME,
    STATE_SHOWSTATUS,
    STATE_FINDENSEMBLES,
    STATE_SELENSEMBLE,
    STATE_SHOWTIME,
    STATE_CLEARPROM,
    STATE_SETBAND,
    STATE_SELRED,
    STATE_SELGREEN,
    STATE_SELBLUE
};

enum BAND { BAND_DAB, BAND_FM };

typedef struct
{ 
    unsigned char fIndex; 
    unsigned char volume;
    unsigned short int serviceId;
    unsigned int fmFreq;
    BAND band;
    boolean locked;
    unsigned char redLevel;
    unsigned char greenLevel;
    unsigned char blueLevel;
    unsigned char cSum;
} PROMTYPE;

typedef struct
{
    void (*enterFn)();      // Called when entering this state
    void (*exitFn)();       // Called when exiting this state
    void (*actionFn)();     // Called each time round loop() 
    int timeoutSecs;        // How many seconds before automatic state change
    STATEID timeoutState;   // Which state to change to on timeout
} STATEDEF;

typedef struct
{
    STATEDEF *stateDef;
    unsigned long int lastMs;
    unsigned long int timeoutMs;
} STATEINFO;
