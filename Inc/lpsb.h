#ifndef __LPSB_H__
#define __LPSB_H__

#include <stdbool.h>
#include "main.h"

#define WAKEUP_1HOUR    (7149272U)
#define WAKEUP_1MIN     (119154U)
#define WAKEUP_30SEC    (59577U)
#define WAKEUP_10SEC    (19859U)

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

#endif  /* __LPSB_H__ */
