#include "stm32_p103.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "common.h"

extern volatile uint32_t reset_control;

int32_t usart1_rcv_char = -1;
int32_t usart2_rcv_char = -1;

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        reset_control |= BUTTON_PRESSED;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
void USART1_IRQHandler(void)
{
    usart1_rcv_char = USART_ReceiveData(USART1);
}

void USART2_IRQHandler(void)
{
    usart2_rcv_char = USART_ReceiveData(USART2);
    if (usart2_rcv_char == 0) {
        reset_control |= RESET_CMD;
    }
}

void init_led(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO C clock. */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* Set the LED pin state such that the LED is off.  The LED is connected
     * between power and the microcontroller pin, which makes it turn on when
     * the pin is low.
     */
    GPIO_WriteBit(GPIOC,GPIO_Pin_12,Bit_SET);

    /* Configure the LED pin as push-pull output. */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void enable_button_interrupts(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the AFIO clock.  GPIO_EXTILineConfig sets registers in
     * the AFIO.
     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Connect EXTI Line 0 to the button GPIO Pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* Configure the EXTI line to generate an interrupt when the button is
     * pressed.  The button pin is high when pressed, so it needs to trigger
     * when rising from low to high. */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void init_button(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO A clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure the button pin as a floating input. */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    enable_button_interrupts();
}

static void init_usart(USART_TypeDef *uart, int speed)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_TypeDef *gpio;
    uint16_t USART_Rx_Pin, USART_Tx_Pin;

    if (uart == USART1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        gpio = GPIOA;
        USART_Rx_Pin = GPIO_Pin_10;
        USART_Tx_Pin = GPIO_Pin_9;
    } else if (uart == USART2) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        gpio = GPIOA;
        USART_Rx_Pin = GPIO_Pin_3;
        USART_Tx_Pin = GPIO_Pin_2;
    } else if (uart == USART3) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        gpio = GPIOB;
        USART_Rx_Pin = GPIO_Pin_3;
        USART_Tx_Pin = GPIO_Pin_2;
    }

    /* Configure Rx pin as floating input. */
    GPIO_InitStructure.GPIO_Pin = USART_Rx_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(gpio, &GPIO_InitStructure);

    /* Configure Tx as alternate function push-pull. */
    GPIO_InitStructure.GPIO_Pin = USART_Tx_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(gpio, &GPIO_InitStructure);

    /* Configure the USART */
    USART_InitStructure.USART_BaudRate = speed;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart, &USART_InitStructure);
}

static void enable_usart_interrupts(USART_TypeDef *uart)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable transmit and receive interrupts for the USART2. */
    USART_ITConfig(uart, USART_IT_TXE, DISABLE);
    USART_ITConfig(uart, USART_IT_RXNE, ENABLE);

    if (uart == USART1) NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    else if (uart == USART2) NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    else if (uart == USART3) NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void enable_usart(USART_TypeDef *uart)
{
    /* Enable the RS232 port. */
    USART_Cmd(uart, ENABLE);
}

static void send_byte_usart(USART_TypeDef *uart, uint8_t b)
{
    USART_SendData(uart, b);
    while(USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET);
}

static uint8_t read_byte_usart(USART_TypeDef *uart)
{
    int32_t *c;
    uint8_t r;

    if (uart == USART1) c = &usart1_rcv_char;
    else if (uart == USART2) c = &usart2_rcv_char;
    while (*c == -1) __WFI();
    r = *c & 0xff;
    *c = -1;
    return r;
}

static void send_string_usart(USART_TypeDef *uart, char *string)
{
    while (*string != 0) {
        send_byte_usart(uart, (uint8_t) *string++);
    }
}

static void send_hex_usart(USART_TypeDef *uart, uint32_t n)
{
    uint8_t c;
    int i;

    for (i = 0; i < 8; i++) {
        c = (n >> 28);
        send_byte_usart(uart, c + (c > 9 ? 'a' - 10 : '0'));
        n <<= 4;
    }
}

void init_log(void)
{
    init_usart(USART1, 115200);
    enable_usart_interrupts(USART1);
    enable_usart(USART1);
}

void log_string(char *str)
{
    send_string_usart(USART1, str);
}

void log_nl()
{
    log_string("\r\n");
}

void log_hex(uint32_t n)
{
    send_hex_usart(USART1, n);
}

void set_led(uint32_t on)
{
    if (on) {
        GPIOC->BRR = 0x00001000;
    } else {
        GPIOC->BSRR = 0x00001000;
    }
}

void init_uart(void)
{
    init_usart(USART2, 115200);
    enable_usart_interrupts(USART2);
    enable_usart(USART2);
}

void send_byte(uint8_t byte)
{
    send_byte_usart(USART2, byte);
}

void send_string(char *string)
{
    send_string_usart(USART2, string);
}

uint8_t read_byte(void)
{
    return read_byte_usart(USART2);
}

uint32_t read_long(void)
{
    return read_byte() + (read_byte() << 8) + (read_byte() << 16) + (read_byte() << 24);
}
