#include "stm32f10x.h"
#include "stm32_p103.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#define RESET_CMD 0x01
#define DIVIDER 16
#define BUTTON_PRESSED 0x02

int32_t usart1_rcv_char = -1;
int32_t usart2_rcv_char = -1;
volatile uint32_t reset_control;
uint32_t samples[2048];
uint32_t samples_count;

void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
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

void send_byte(USART_TypeDef *uart, uint8_t b)
{
    USART_SendData(uart, b);
    while(USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET);
}

uint8_t read_byte(USART_TypeDef *uart)
{
    while(USART_GetFlagStatus(uart, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(uart);
}

uint8_t read_byte_int(USART_TypeDef *uart)
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

void send_string(USART_TypeDef *uart, uint8_t *string)
{
    while (*string != 0) {
        send_byte(uart, *string++);
    }
}

void send_hex(USART_TypeDef *uart, uint32_t n)
{
    uint8_t c;
    int i;

    for (i = 0; i < 8; i++) {
        c = (n >> 28);
        send_byte(uart, c + (c > 9 ? 'a' - 10 : '0'));
        n <<= 4;
    }
}

uint32_t read_long(USART_TypeDef *uart)
{
    return read_byte(uart) + (read_byte(uart) << 8) + (read_byte(uart) << 16) + (read_byte(uart) << 24);
}

uint32_t read_long_int(USART_TypeDef *uart)
{
    return read_byte_int(uart) + (read_byte_int(uart) << 8) + (read_byte_int(uart) << 16) + (read_byte_int(uart) << 24);
}

void init_log(void)
{
    init_usart(USART1, 115200);
    enable_usart_interrupts(USART1);
    enable_usart(USART1);
}

void log(char *str)
{
    send_string(USART1, str);
}

void log_nl()
{
    log("\r\n");
}

void log_hex(uint32_t n)
{
    send_hex(USART1, n);
}

void print_samples(void)
{
    int i;
    uint32_t timer;

    for (timer =0, i = 0; i < ARRAY_COUNT(samples) && !reset_control; i++) {
        if (samples[i] & 0x80000000) {
            timer++;
        }
        log_hex(samples[i]);
        send_byte(USART1, ' ');
        log_hex(((0x00ffffff - (samples[i] & 0x00ffffff)) / DIVIDER) + timer * 0x01000000);
        log("\r\n");
    }
}

void delay(uint32_t n)
{
    while (n--) ;
}

void send_samples(void)
{
    int i, j, b;
    uint32_t sample, last_sample;
    uint32_t t, prev_t, sample_t;
    uint32_t timer;

    log("Sending samples.\r\n");
    i = 0;
    t = 0;
    sample_t = 0;
    prev_t = 0xffffffff;
    timer = 0;
    reset_control = 0;

    for (i = 0; i < ARRAY_COUNT(samples) && !reset_control; i++) {
        sample = samples[i];
        if (sample & 0x80000000) {
            timer++;
        } else {
            sample_t = ((0x00ffffff - (sample & 0x00ffffff)) / DIVIDER) + timer * 0x01000000;
            while (t < sample_t && !reset_control) {
                send_byte(USART2, (last_sample >> 24) & 0x7f);
                t++;
            }
            if (t != prev_t) {
                send_byte(USART2, (sample >> 24) & 0x7f);
                last_sample = sample;
                prev_t = t;
            }
        }
    }
    log("Sending done.\r\n");
}

void start_sampling(void)
{
    log("Start sampling.\r\n");
    GPIOC->BRR = 0x00001000;
    memset(samples, ARRAY_COUNT(samples), 0);
    samples_count = sampler(samples, ARRAY_COUNT(samples), &reset_control);
    GPIOC->BSRR = 0x00001000;
    log("Sampling done, count: ");
    log_hex(samples_count);
    log(" control: ");
    log_hex(reset_control);
    log("\r\n");
    send_samples();
}

void generic_long_command()
{
    uint32_t parameter;

    parameter = read_long_int(USART2);
    log("Parameter: ");
    log_hex(parameter);
    log("\r\n");
}

void sump_handler(void)
{
    uint8_t command;

    while (1) {
        command = read_byte_int(USART2);
        log("Got command: ");
        log_hex(command);
        log("\r\n");
        switch (command) {
        case 0x00: break;
        case 0x01: start_sampling(); break;
        case 0x02: send_string(USART2, "1ALS"); break;
        case 0x11: break;
        case 0x13: break;
        case 0xc0:
        case 0xc4:
        case 0xc8:
        case 0xcc:
        case 0xc1:
        case 0xc5:
        case 0xc9:
        case 0xcd:
        case 0xc2:
        case 0xc6:
        case 0xca:
        case 0xce:
        case 0x80:
        case 0x81:
        case 0x82: generic_long_command(); break;
        }
    }
}

int main(void)
{
    int input_state, old_state = 0;
    int i;
    uint32_t t, t0;

    init_led();
    init_button();
    enable_button_interrupts();
    init_log();

    init_usart(USART2, 115200);
    enable_usart_interrupts(USART2);
    enable_usart(USART2);

    log("Ready.\r\n");
    sump_handler();
}
