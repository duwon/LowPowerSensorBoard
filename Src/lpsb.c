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

void lpsb_start(void)
{
  printUartLogo();                                         //print Logo
  HAL_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, GPIO_PIN_SET); //LD0 LED On
}

void lpsb_while(void)
{
  HAL_Delay(1000);
  HAL_GPIO_TogglePin(LD0_GPIO_Port, LD0_Pin); //LD0 LED Toggle
}
