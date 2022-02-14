/* admsr.h */
/* command byte, I/O address, and bit field definitions */


/* io base address using 82C55 control chip */

#define IOBASE 0x300
#define IOPORTA IOBASE
#define IOPORTB IOBASE + 1
#define IOPORTC IOBASE + 2
#define CONTROL IOBASE + 3

/* WRITE ADDRESS TO THE 82C55 PORT B WHICH ARE DECODED ON THE INTERFACE */
/* BOARD. THE ADDRESS IS THE ADDRESS ON THE INTERFACE BOARD WHICH ARE */
/* OUTPUT ON PORTB. AFTER WRITING THE ADDRESS THE VALUE IS THEN WRITTEN */
/* TO PORT A WHICH IS THEN STROBED INTO THE APPROPRIATE ADDR. */

#define INTRFCCOMMAND 0x80
#define DAC0LOADD 0xA0
#define DAC0HIADD 0xB0
#define DAC1LOADD 0xC0
#define DAC1HIADD 0xD0
#define CNTRWRADDR 0xE0

/* READ ADDRESSES ARE OUTPUT ON PORT B. THE INTERFACE STROBES THE DATA */
/* INTO PORT A FOR INPUT. IT IS ESSENTIAL TO KNOW THAT SOME READ DO NOT */
/* RETURN A VALUE BUT SIMPLE STORE THE ADDR. */

#define MUXCHANNEL 0x10
#define IOREAD 0x20
#define ADTRIG 0x30
#define ADNEXT 0x40
#define CNTRRDADDR 0x60
#define IOMODES 0xC4
#define ABORT 0xFF
#define INPUTMODE 0x9B

/* flags used to control actions when sending bytes to LATCH */

#define SET_BIT 1
#define CLR_BIT 0

/* THESE ARE THE BIT VALUES IN THE INTRFCNTRL BYTE VARIABLE WHICH ARE */
/* USED TO CONTROL THE INTERFACE                                      */

#define DATAREQ    1
#define BATSEL     2
#define ZEROCAL    4
#define READY      8
#define MARK      16
#define RUNNING   32
#define INTEN     64
#define CPURESET 128

/* COMMAND DEFINITION CONSTANTS. THESE ARE THE VALUES SENT TO THE */
/* STRETCHER INTERFACE COMMAND REGISTERS. */

#define FIRST_TEMP      1
#define NEXT_TEMP       2
#define STOP_XFER       3
#define START_CAPTURE   4
#define STOP_CAPTURE    5
#define SEND_MSB_COUNT  6
#define SEND_LSB_COUNT  7
#define STATUS_REQ      8
#define CAL_REQ         9
#define SCI_TAP        10
#define ALT_STATUS_REQ 0x1F;

extern float A_D_Avg[];

/* Low level routines in dio24 module */

void ADMeasure(long A_D_Buff[]);
void AvgAD(float *A_D_Avg, int Count);
void ADMux(int GROUP);
void LATCHOUT(const int BITVAL, int set_clr);
void DAC0OUT(int Value);
void DAC1OUT(int Value);
void Command(int Cmd, int *CmdEcho);

