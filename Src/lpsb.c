#include <stdio.h>
#include <string.h>
#include "lpsb.h"
#include "usart.h"
#include "gpio.h"
#include "adc.h"
#include "rtc.h"

#define FW_VERSION "0.1"

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
  while (HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF) != HAL_OK)
    ;
  return ch;
}

void printUartLogo(void)
{
  printf("\r\n");
  printf("\r\n=============================================");
  printf("\r\n|       (C) COPYRIGHT 2019 INPRO Inc.       |");
  printf("\r\n|                                           |");
  printf("\r\n|   LPSB Test   V%s   %s %s |", FW_VERSION, __DATE__, __TIME__);
  printf("\r\n=============================================");
  printf("\r\n\r\n");
}

uint8_t getID(void)
{
  /**
   * 1. RTC 백업레지스터에서 ID 값 읽어 옴
   * 2. ID 체크를 25회 이하거나 iID가 0이면
   * 2-1. ID를 200ms 간격으로 체크
   * 2-2. ID를 백업 레지스터에 저장
   * 3. ID 값 리턴
   **/
  uint16_t rtcBackupReg = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);

  uint8_t iID = rtcBackupReg & 0x00FF;
  uint8_t IDReadCount = (rtcBackupReg >> 8) & 0x00ff;
  debug_printf("DR2 : %x\r\n", rtcBackupReg);
  if ((IDReadCount < 25) && (iID == 0))
  {
    while (iID == 0)
    {
      iID = HAL_GPIO_ReadPin(DIP6_GPIO_Port, DIP6_Pin);
      iID = (iID << 1) + HAL_GPIO_ReadPin(DIP5_GPIO_Port, DIP5_Pin);
      iID = (iID << 1) + HAL_GPIO_ReadPin(DIP4_GPIO_Port, DIP4_Pin);
      iID = (iID << 1) + HAL_GPIO_ReadPin(DIP3_GPIO_Port, DIP3_Pin);
      iID = (iID << 1) + HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin);
      iID = (iID << 1) + HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin);
      iID = (iID << 1) + HAL_GPIO_ReadPin(DIP0_GPIO_Port, DIP0_Pin);

      IDReadCount++;
      if ((IDReadCount % 5) == 0)
      {
        break;
      }
      HAL_Delay(200);
    }
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, ((uint16_t)IDReadCount << 8) + iID);
  }

  return iID;
}

uint32_t getBATLevel(void)
{
  if (HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, 100);

  return HAL_ADC_GetValue(&hadc);
}

void controlDCOn(DC_Pin pin)
{
  HAL_GPIO_WritePin(CONTROL_DC12V_GPIO_Port, pin, GPIO_PIN_SET);
}

void controlDCOff(DC_Pin pin)
{
  HAL_GPIO_WritePin(CONTROL_DC12V_GPIO_Port, pin, GPIO_PIN_RESET);
}

GPIO_PinState lpsb_GetSensor(Sensor_Pin pin)
{
  return HAL_GPIO_ReadPin(SENSOR0_GPIO_Port, 1 << pin);
}

#pragma pack(push, 1)
struct _comPacket
{
  uint16_t stx;
  uint8_t id;
  uint8_t sensor0;
  uint8_t sensor1;
  uint8_t sensor2;
  uint8_t batLevel;
  uint8_t checksum;
  uint8_t etx;
};
#pragma pack(pop)

void sendPacket(uint8_t id, bool sen0, bool sen1, bool sen2, uint8_t batLevel)
{
  struct _comPacket comPacket;

  comPacket.stx = 0x1234;
  comPacket.id = id;
  comPacket.sensor0 = sen0;
  comPacket.sensor1 = sen1;
  comPacket.sensor2 = sen2;
  comPacket.batLevel = batLevel;
  comPacket.checksum = 0x99;
  comPacket.etx = 0x43;

  uint8_t *ptr;
  ptr = (uint8_t *)&comPacket;
  for (int i = 0; i < sizeof(comPacket); i++)
    debug_printf("%2X ", *ptr++);

  while (HAL_UART_Transmit(&hlpuart1, ptr, 9, 0xFFFF) != HAL_OK)
    ;
}

void lpsb_EnterStanbyMode(uint32_t wakeupTime)
{
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  __HAL_RCC_PWR_CLK_ENABLE();      /* Enable Power Control clock */
  HAL_PWREx_EnableUltraLowPower(); /* Enable Ultra low power mode */
  HAL_PWREx_EnableFastWakeUp();    /* Enable the fast wake up from Ultra low power mode */

  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); /* Clear Standby flag */
  }

  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);                                       /* Disable all used wakeup sources*/
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);                                            /* Clear all related wakeup flags*/
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, wakeupTime, RTC_WAKEUPCLOCK_RTCCLK_DIV16); /* Enable wakeup */

  HAL_PWR_EnterSTANDBYMode();
}

void lpsb_start(void)
{
#ifdef _DEBUG_
  printUartLogo(); /* Print Logo */
#endif
  controlDCOn(DC12V); /* Sensor Module power on */
  controlDCOn(DC3V);  /* Wirelss Module power on */

  HAL_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, GPIO_PIN_SET); /* LD0 LED On */

  uint8_t lpsb_ID = getID(); /* Read ID. Using DIP(S1) switch */
  uint32_t lpsb_BATLevel = getBATLevel();

  debug_printf("ID: 0x%x\r\n", lpsb_ID);
  debug_printf("BAT Level: %d\r\n", lpsb_BATLevel); //12V 840, 5V 295, 3.3V 220

  uint8_t sen0 = lpsb_GetSensor(SEN0);
  uint8_t sen1 = lpsb_GetSensor(SEN1);
  uint8_t sen2 = lpsb_GetSensor(SEN2);

  debug_printf("Sensor %d, %d, %d\r\n", sen0, sen1, sen2); /* make packet */

  sendPacket(lpsb_ID, sen0, sen1, sen2, lpsb_BATLevel);

  uint16_t sending_cnt = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1); /* Read bakcup register. 16bit */
  debug_printf("Sening Cnt : %d \r\n", sending_cnt++);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, sending_cnt);

  
}

uint8_t wirelss_mode = MODE_WAIT;
void lpsb_while(void)
{
  switch (wirelss_mode)
  {
  case MODE_SEND:

    break;
  case MODE_RESEND:
    lpsb_EnterStanbyMode(WAKEUP_10SEC);
    break;
  case MODE_LOWPOWER:
    lpsb_EnterStanbyMode(WAKEUP_1HOUR);
    break;
  case MODE_WAIT:
    HAL_GPIO_TogglePin(LD0_GPIO_Port, LD0_Pin); /* LD0 LED Toggle */
    HAL_Delay(100);
    //wirelss_mode = MODE_RESEND;
    break;
  default:
    break;
  }

}
