#include "board.h"
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_chip.h>
#include <em_emu.h>
#include <em_timer.h>
#include <em_system.h>
#include <em_usart.h>

extern volatile uint32_t reset_control;

volatile uint8_t usart0_rcv_char = -1;
volatile uint8_t usart1_rcv_char = -1;

int main(void);

void init_board(void)
{
    CHIP_Init();

    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
    CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_8);

    CMU_ClockEnable(cmuClock_GPIO, true);
}

void USART0_RX_IRQHandler(void)
{
    if (USART0->STATUS & USART_STATUS_RXDATAV) {
        usart0_rcv_char = USART_Rx(USART0);
    }
}

void USART1_RX_IRQHandler(void)
{
    if (USART1->STATUS & USART_STATUS_RXDATAV) {
        usart1_rcv_char = USART_Rx(USART1);
    }
}

#ifdef LOGGING

void init_log(void)
{
    USART_InitAsync_TypeDef init  = USART_INITASYNC_DEFAULT;

    CMU_ClockEnable(cmuClock_USART0, true);
    GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 11, gpioModeInput, 0);

    init.enable = usartDisable;
    USART_InitAsync(USART0, &init);

    USART0->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | USART_ROUTE_LOCATION_LOC0;
    USART_IntClear(USART0, USART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(USART0_RX_IRQn);

    USART_IntEnable(USART0, USART_IF_RXDATAV);
    NVIC_EnableIRQ(USART0_RX_IRQn);

    USART_Enable(USART0, usartEnable);
}

void log_string(char *str)
{
    while (*str != '\0') {
        USART_Tx(USART0, *str);
        str++;
    }
}

void log_nl(void)
{
    log_string("\r\n");
}

void log_hex(uint32_t n)
{
    uint8_t c;
    int i;

    for (i = 0; i < 8; i++) {
        c = (n >> 28);
        USART_Tx(USART0, c + (c > 9 ? 'a' - 10 : '0'));
        n <<= 4;
    }
}

#else

void init_log(void)
{
}

void log_string(char *str)
{
}

void log_nl(void)
{
}

void log_hex(uint32_t n)
{
}

#endif

void init_button(void)
{
}

void init_led(void)
{
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPullDrive, 0);
    GPIO_DriveModeSet(gpioPortA, gpioDriveModeHigh);
}

void init_uart(void)
{
    USART_InitAsync_TypeDef init  = USART_INITASYNC_DEFAULT;

    CMU_ClockEnable(cmuClock_USART1, true);

    GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);

    init.enable = usartDisable;
    USART_InitAsync(USART1, &init);

    USART1->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | USART_ROUTE_LOCATION_LOC0;
    USART_IntClear(USART1, USART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(USART1_RX_IRQn);

    USART_IntEnable(USART1, USART_IF_RXDATAV);
    NVIC_EnableIRQ(USART1_RX_IRQn);

    USART_Enable(USART1, usartEnable);
}

void set_led(uint32_t on)
{
    if (on) {
        GPIO_PinOutClear(gpioPortA, 0);
    } else {
        GPIO_PinOutSet(gpioPortA, 0);
    }
}

void send_byte(uint8_t byte)
{
    USART_Tx(USART1, byte);
}

void send_string(char *str)
{
    while (*str != '\0') {
        USART_Tx(USART1, *str);
        str++;
    }
}

uint8_t read_byte(void)
{
    uint8_t r;
    volatile uint8_t *c;

    c = &usart1_rcv_char;
    while (*c == -1) __WFI();
    r = *c;
    *c = -1;
    return r;
}

uint32_t read_long(void)
{
    return read_byte() + (read_byte() << 8) + (read_byte() << 16) + (read_byte() << 24);
}

int _start(void)
{
    return main();
}
