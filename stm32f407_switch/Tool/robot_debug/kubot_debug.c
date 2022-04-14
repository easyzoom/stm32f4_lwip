#include "kubot_debug.h"

int fputc(int ch, FILE *f)
{
    LL_USART_TransmitData8(UART4, (uint8_t)ch);
    while(!LL_USART_IsActiveFlag_TXE(UART4));
    return ch;
}
