#include "delay.h"

#define Timebase_Source_is_SysTick 0 //��Timebase SourceΪSysTickʱ��Ϊ1,��ʹ��FreeRTOS��Timebase SourceΪ������ʱ��ʱ��Ϊ0

#if (!Timebase_Source_is_SysTick)
extern TIM_HandleTypeDef htim1;    //��ʹ��FreeRTOS��Timebase SourceΪ������ʱ��ʱ���޸�Ϊ��Ӧ�Ķ�ʱ��
#define Timebase_htim htim1

#define Delay_GetCounter() __HAL_TIM_GetCounter(&Timebase_htim)
#define Delay_GetAutoreload() __HAL_TIM_GetAutoreload(&Timebase_htim)
#else
#define Delay_GetCounter() (SysTick->VAL)
#define Delay_GetAutoreload() (SysTick->LOAD)
#endif

static uint16_t fac_us = 0;
static uint32_t fac_ms = 0;


void delay_init(void)
{ 
#if (!Timebase_Source_is_SysTick)
  fac_ms = 1000000;        //��Ϊʱ���ļ�����ʱ��Ƶ����HAL_InitTick()�б���Ϊ��1MHz
  fac_us = fac_ms / 1000;
#else
  fac_ms = SystemCoreClock / 1000;
  fac_us = fac_ms / 1000;
#endif
}


void delay_us(uint32_t nus)
{ 
  uint32_t ticks = 0;
  uint32_t told = 0;
  uint32_t tnow = 0;
  uint32_t tcnt = 0;
  uint32_t reload = 0;

  reload = Delay_GetAutoreload();

  ticks = nus * fac_us;

  told = Delay_GetCounter();

  while (1)
  { 
    tnow = Delay_GetCounter();

    if (tnow != told)
    { 
      if (tnow < told)
      { 
        tcnt += told - tnow;
      }
      else
      { 
        tcnt += reload - tnow + told;
      }
      told = tnow;
      if (tcnt >= ticks)
      { 
        break;
      }
    }
  }
}


void delay_ms(uint32_t nms)
{ 
  uint32_t ticks = 0;
  uint32_t told = 0;
  uint32_t tnow = 0;
  uint32_t tcnt = 0;
  uint32_t reload = 0;

  reload = Delay_GetAutoreload();

  ticks = nms * fac_ms;

  told = Delay_GetCounter();

  while (1)
  { 
    tnow = Delay_GetCounter();

    if (tnow != told)
    { 
      if (tnow < told)
      { 
        tcnt += told - tnow;
      }
      else
      { 
        tcnt += reload - tnow + told;
      }
      told = tnow;
      if (tcnt >= ticks)
      { 
        break;
      }
    }
  }
}


void HAL_Delay(uint32_t Delay)
{ 
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  
// 
// if (wait < HAL_MAX_DELAY)
// { 
// wait += (uint32_t)(uwTickFreq);
// }

  while ((HAL_GetTick() - tickstart) < wait)
  { 
  }
}
