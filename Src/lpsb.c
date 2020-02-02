#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lpsb.h"
#include "usart.h"
#include "gpio.h"
#include "adc.h"
#include "rtc.h"

/* Flash 용량문제로 하기 코드 삭제
#define FW_VERSION "0.1"

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif 

PUTCHAR_PROTOTYPE
{
  while (HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF) != HAL_OK)
    ;
  return ch;
}
*/ 

char hex2ascii(char hex)
{
  if (hex < 0x0A)
    hex += 0x30;
  else
    hex += 0x37;
  return (hex);
}

void PRINT(uint8_t *tempBuffer, uint8_t len)
{
  /* Flash 용량 초과로 printf 대체용 함수 
     tempBuffer 포인터 HEX 값을 ASCII로 출력
  */
  for (int i = 0; i < len; i++)
  {
    uint8_t tempAscii[2] = {hex2ascii(tempBuffer[i] >> 4), hex2ascii(tempBuffer[i] & 0xF)};
    while (HAL_UART_Transmit(&huart2, tempAscii, 2, 0xFFFF) != HAL_OK)
      ;
  }
  char tempCharBuffer[2] = {'\r', '\n'};
  while (HAL_UART_Transmit(&huart2, (uint8_t *)&tempCharBuffer, 2, 0xFFFF) != HAL_OK)
    ;
}

uint8_t UID[8] = {
    0,
};
void getUniqueId(void)
{
  UID[7] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 24;
  UID[6] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 16;
  UID[5] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 8;
  UID[4] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3));
  UID[3] = ((*(uint32_t *)ID2)) >> 24;
  UID[2] = ((*(uint32_t *)ID2)) >> 16;
  UID[1] = ((*(uint32_t *)ID2)) >> 8;
  UID[0] = ((*(uint32_t *)ID2));
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

uint8_t getBATLevel(void)
{
  if (HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, 100);
  uint32_t adc_value = HAL_ADC_GetValue(&hadc); //12V 840, 5V 295, 3.3V 220
  adc_value -= 180;

  return adc_value > 31 ? 31 : (uint8_t)adc_value;
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
typedef struct
{
  uint8_t stx : 2; //0x02
  uint16_t id : 10;
  uint8_t s_idx : 2;
  uint8_t r_idx : 2;
  uint8_t command : 3; //Relay, Holding, Sensor,
  uint8_t batLevel : 5;
  uint8_t sensor0 : 1;
  uint8_t sensor1 : 1;
  uint8_t sensor2 : 1;
  uint8_t etc : 1;
  uint8_t parity : 2;
  uint8_t etx : 2; //0x03
} _comPacket;
#pragma pack(pop)

char make_parity(_comPacket data)
{
  unsigned long bit_data, bit_data_H = 0; //, bit_data_L;
  unsigned char i, count1 = 0, count2 = 0, parity1 = 0, parity2 = 0;

  memcpy(&bit_data, &data, sizeof(data));
  bit_data_H = bit_data >> 13;
  for (i = 0; i < 13; i++)
    if ((bit_data_H >> i) & 0x01)
      count2++;
  if (count2 % 2)
    parity2 = 1;
  // bit_data_L=bit_data >> 2;
  for (i = 0; i < 13; i++)
    if ((bit_data >> i) & 0x01)
      count1++;
  if (count1 % 2)
    parity1 = 1;
  return (parity2 << 1 | parity1);
}

_comPacket comTxPacket;
void sendPacket(uint8_t id, bool sen0, bool sen1, bool sen2, uint8_t batLevel)
{

  comTxPacket.stx = 0x02;
  comTxPacket.id = id;
  comTxPacket.s_idx = 0;
  comTxPacket.r_idx = 0;
  comTxPacket.command = 7;
  comTxPacket.batLevel = batLevel;
  comTxPacket.sensor0 = sen0;
  comTxPacket.sensor1 = sen1;
  comTxPacket.sensor2 = sen2;
  comTxPacket.parity = make_parity(comTxPacket);
  comTxPacket.etx = 0x03;

  uint8_t *ptr;
  ptr = (uint8_t *)&comTxPacket;
  PRINT(ptr, 4);

  while (HAL_UART_Transmit(&hlpuart1, ptr, 4, 0xFFFF) != HAL_OK)
    ;
}

void resendPacket(void)
{
  uint8_t *ptr;
  ptr = (uint8_t *)&comTxPacket;
  PRINT(ptr, 4);

  while (HAL_UART_Transmit(&hlpuart1, ptr, 4, 0xFFFF) != HAL_OK)
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

  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);                                         /* Disable all used wakeup sources*/
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);                                              /* Clear all related wakeup flags*/
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, wakeupTime, RTC_WAKEUPCLOCK_CK_SPRE_16BITS); /* Enable wakeup */

  HAL_PWR_EnterSTANDBYMode();
}

_comPacket comRxPacket; /* 200202 사용하지 않음 */
uint8_t comRxCh; /* UART 수신 Data 저장 변수 */
void lpsb_start(void)
{
  srand((*(uint32_t *)ID1) ^ (*(uint32_t *)ID2) ^ (*(uint32_t *)ID3));

  controlDCOn(DC3V);  /* Wirelss Module power on */
  controlDCOn(DC12V); /* Sensor Module power on */
  HAL_GPIO_WritePin (COM_NRST_GPIO_Port, COM_NRST_Pin, GPIO_PIN_SET); /* Pull-up wirelss module reset pin */
  HAL_Delay(1);

  uint16_t sending_cnt = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) + 1; /* Read bakcup register(16bit - wakeup count) and add 1 */
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, sending_cnt);
  PRINT((uint8_t *)&sending_cnt, 2);

  HAL_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, GPIO_PIN_SET); /* LD0 LED On */

  uint8_t lpsb_ID = getID(); /* Read ID. Using DIP(S1) switch */
  uint8_t lpsb_BATLevel = getBATLevel();

  uint8_t sen0 = lpsb_GetSensor(SEN0);
  uint8_t sen1 = lpsb_GetSensor(SEN1);
  uint8_t sen2 = lpsb_GetSensor(SEN2);
  controlDCOff(DC12V); /* Sensor Module power off */

  HAL_UART_Receive_DMA(&hlpuart1, (uint8_t *)&comRxCh, 1);  /* Start recive DMA interrupt  */
  HAL_UART_Receive_DMA(&huart2, (uint8_t *)&comRxCh, 1);    /* Debug UART Code. Delete */

  sendPacket(lpsb_ID, sen0, sen1, sen2, lpsb_BATLevel); /* Transmit the data (ID, Sensor, Battery Level) */


  //getUniqueId();
  //PRINT(UID, 8);
  //PRINT(&lpsb_ID, 1);
  //PRINT(&sen0,1);
  //PRINT(&sen1,1);
  //PRINT(&sen2,1);
  //debug_printf("ID: 0x%x\r\n", lpsb_ID);
  //debug_printf("BAT Level: %d\r\n", lpsb_BATLevel);
  //debug_printf("Sensor %d, %d, %d\r\n", sen0, sen1, sen2); /* make packet */
  //debug_printf("Sening Cnt : %d \r\n", sending_cnt++);
}

uint8_t wirelss_mode = MODE_WAIT;
bool flag_AckOk = false;
uint8_t countReSend = 0;

//테스트 변수 200202
uint8_t rx_buffer[10];
uint8_t rx_buffer_index = 0;
//테스트 변수 200202

void lpsb_while(void)
{
  /* 1. 400ms ~ 600ms 대기 
     2. ACK을 받았거나 NUMBER_RETRANSMISSION 만큼 재전송 했으면 MODE_LOWPOWER
     3. 2번이 아닌 경우 MODE_RESEND 모드
  */
  switch (wirelss_mode)
  {
  case MODE_SEND:

    break;
  case MODE_RESEND:
    comRxPacket.stx = 0;
    countReSend++;
    wirelss_mode = MODE_WAIT;
    resendPacket();
    break;
  case MODE_LOWPOWER:
    countReSend = 0;
    lpsb_EnterStanbyMode(TX_INTERVAL_TIME);
    break;
  case MODE_WAIT:
    HAL_GPIO_TogglePin(LD0_GPIO_Port, LD0_Pin); /* LD0 LED Toggle */
    HAL_Delay(400 + (rand() % 200));

    //테스트코드 200202 start - 송수신 데이터 맞는지 확인
      if (rx_buffer_index != 0)
      {
        PRINT(rx_buffer, 4);
        rx_buffer_index = 0;

        uint32_t *txPtr = (uint32_t *)&comTxPacket;
        uint32_t *rxPtr = (uint32_t *)&rx_buffer;

        if (txPtr[0] == rxPtr[0])
        {
          flag_AckOk = true;
        }
        else
        {
          flag_AckOk = false;
        }
        rx_buffer[0] = 0;
      }
    //테스트코드 200202 end

    if ((countReSend == NUMBER_RETRANSMISSION) || (flag_AckOk == true)) /* If resend NUMBER_RETRANSMISSION, enter lowpower mode. */
    {
      wirelss_mode = MODE_LOWPOWER;
    }
    else /* If packets are not received, resend packet. */
    {
      wirelss_mode = MODE_RESEND;
    }
    break;

//테스트코드 200202 start
  case MODE_END:
    HAL_GPIO_TogglePin(LD0_GPIO_Port, LD0_Pin); /* LD0 LED Toggle */
    HAL_Delay(400 + (rand() % 200));

    break;
//테스트코드 200202 end

  default:
    break;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == LPUART1) /* Wirelss Port */
  {
    //PRINT((uint8_t *)&comRxPacket,1);
    rx_buffer[rx_buffer_index++] = comRxCh;
    HAL_UART_Receive_DMA(&hlpuart1, (uint8_t *)&comRxCh, 1);
  }
  else if (huart->Instance == USART2) /* Debug Port */
  {
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)&comRxCh,1,0xFFFF); /* Debug Port에서 수신된 문자를 Wirelss Port로 송신 - 테스트용 */
    HAL_UART_Receive_DMA(&huart2, (uint8_t *)&comRxCh, 1);
  }
}
