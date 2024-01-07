void dabGetShortLabel(unsigned long int serviceId, char *shortLabel, unsigned char *pty);

void DABSpiMsg(unsigned char *data, uint32_t len);
void startReceiver();
boolean setFrequency();
void startDabService();
boolean setService();
void processServiceData(void);
void rxSetup(void);
void rxLoop(void);
