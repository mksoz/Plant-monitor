// Blinking rate in milliseconds
#define BLINKING_RATE_MS  500
#define INIT   0
#define TEST   1
#define NORMAL 2
#define HOUR   120 // 120 * 30s = 1h
#define DIV 8191.5
#define TWO_SEC 2000000
#define ONE_SEC 1000000
#define THIRTY_SEC 30000000
#define MAIN_TASK_DELAY_MS  1000
#define MSG_TO_RINT_LEN     128
#define REG_WHO_AM_I  0x0D
#define REG_SYSMOD    0x0B
#define REG_CTRL_REG  0x2A // CTRL_REG1 must be set for ignoring LSBs reg
//The measured acceleration data is stored 
// as 2’s complement 14-bit numbers
#define REG_OUT_X_MSB 0x01 //The most significant 8-bits 
#define REG_OUT_X_LSB 0x02 // so applications needing only 8-bit results can use 
                           // these 3 registers and ignore LSBs
#define REG_OUT_Y_MSB 0x03
#define REG_OUT_Y_LSB 0x04
#define REG_OUT_Z_MSB 0x05
#define REG_OUT_Z_LSB 0x06
#define GRAVITY 9.8066
#define MAX_TEMP 50
#define MIN_TEMP  -10
#define MAX_HUM   75
#define MIN_HUM   25
#define MAX_LIGHT 0.8
#define MIN_LIGHT 0
#define MAX_SOIL  0.8
#define MIN_SOIL  0
#define MAX_X     7
#define MAX_Y     10
#define MAX_Z     7
#define MIN_X     -6
#define MIN_Y     2
#define MIN_Z     -6