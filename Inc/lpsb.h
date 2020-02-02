#ifndef __LPSB_H__
#define __LPSB_H__

#include <stdbool.h>
#include "main.h"

#define _DEBUG_
#ifdef _DEBUG_
#define    debug_printf(...)      //printf(__VA_ARGS__)
#else
#define    debug_printf(...)
#endif

#define WAKEUP_18HOUR     (28597088U)
#define WAKEUP_12HOUR        (43200U)
#define WAKEUP_1HOUR          (3600U)
#define WAKEUP_30MIN          (1800U)
#define WAKEUP_1MIN             (60U)
#define WAKEUP_30SEC            (30U)
#define WAKEUP_10SEC            (10U)

 
#define TX_INTERVAL_TIME            WAKEUP_12HOUR
#define NUMBER_RETRANSMISSION       2

#define ID1             ( 0x1FF80050 )
#define ID2             ( 0x1FF80054 )
#define ID3             ( 0x1FF80064 )

void lpsb_start(void);
void lpsb_while(void);

typedef enum{
    DC12V = CONTROL_DC12V_Pin,
    DC3V = CONTROL_DC3V_Pin
} DC_Pin;

typedef enum{
    SEN0 = 8,
    SEN1 = 11,
    SEN2 = 12,   
    SENSOR0 = SEN0,
    SENSOR1 = SEN1,
    SENSOR2 = SEN2
} Sensor_Pin;

typedef enum{
    MODE_SEND = 0,
    MODE_RESEND,
    MODE_STOP,
    MODE_LOWPOWER,
    MODE_WAIT,
    MODE_END
} Wireless_Mode;

#endif  /* __LPSB_H__ */
