#include <stdio.h>
#include <string.h>
#include "lpsb.h"
#include "usart.h"
#include "gpio.h"
#include "adc.h"

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
  //return GPIOA->IDR;
  uint8_t iID = 0;
  iID = HAL_GPIO_ReadPin(DIP6_GPIO_Port, DIP6_Pin);
  iID = (iID << 1) + HAL_GPIO_ReadPin(DIP5_GPIO_Port, DIP5_Pin);
  iID = (iID << 1) + HAL_GPIO_ReadPin(DIP4_GPIO_Port, DIP4_Pin);
  iID = (iID << 1) + HAL_GPIO_ReadPin(DIP3_GPIO_Port, DIP3_Pin);
  iID = (iID << 1) + HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin);
  iID = (iID << 1) + HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin);
  iID = (iID << 1) + HAL_GPIO_ReadPin(DIP0_GPIO_Port, DIP0_Pin);

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
    printf("%2X ", *ptr++);
}

void lpsb_start(void)
{
  printUartLogo();                                         //print Logo
  HAL_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, GPIO_PIN_SET); //LD0 LED On

  uint8_t lpsb_ID = getID();
  uint32_t lpsb_BATLevel = getBATLevel();

  printf("ID: 0x%x\r\n", lpsb_ID);
  printf("BAT Level: %d\r\n", lpsb_BATLevel);

  controlDCOn(DC12V);
  controlDCOn(DC3V);

  uint8_t sen0 = lpsb_GetSensor(SEN0);
  uint8_t sen1 = lpsb_GetSensor(SEN1);
  uint8_t sen2 = lpsb_GetSensor(SEN2);

  printf("Sensor %d, %d, %d\r\n", sen0, sen1, sen2);

  sendPacket(lpsb_ID, sen0, sen1, sen2, lpsb_BATLevel);
}

void lpsb_while(void)
{
  HAL_Delay(1000);
  HAL_GPIO_TogglePin(LD0_GPIO_Port, LD0_Pin); //LD0 LED Toggle
}
