#include "sam3x8e.h"
#include "pmc/pmc.h"
#include "wdt/wdt.h"
#include "pio/pio.h"
#include "pio/pio_sam3x8e.h"
#include "uart/uart.h"
#include "usart/usart.h"
#include "common.h"

extern volatile uint32_t reset_control;

int32_t uart_rcv_char = -1;

void UART_Handler(void)
{
    uint32_t status;

    status = UART->UART_SR;
    if (status & UART_SR_OVRE || status & UART_SR_FRAME) {
        UART->UART_CR |= UART_CR_RSTSTA;
    } else if (status & UART_SR_RXRDY) {
        uart_rcv_char = UART->UART_RHR;
        if (uart_rcv_char == 0 || uart_rcv_char == 3) {
            reset_control |= RESET_CMD;
        }
    }
}

void init_board(void)
{
    SystemInit();
    wdt_disable(WDT);
    pmc_enable_periph_clk(ID_PIOC);
    pio_configure(PIOC, PIO_INPUT,
            PIO_PC1 | PIO_PC2 | PIO_PC3 | PIO_PC4 | PIO_PC5 | PIO_PC6 | PIO_PC7 | PIO_PC8,
            PIO_PULLUP);
}

void init_led(void)
{
    pmc_enable_periph_clk(ID_PIOB);
    pio_configure(PIOB, PIO_OUTPUT_0, PIO_PB27, PIO_DEFAULT);
}

void init_button(void)
{
}

void init_log(void)
{
    sam_usart_opt_t usart_opt = {
        .baudrate = 115200,
        .char_length = US_MR_CHRL_8_BIT,
        .parity_type = US_MR_PAR_NO,
        .stop_bits = US_MR_NBSTOP_1_BIT,
        .channel_mode = US_MR_CHMODE_NORMAL
    };

    pmc_enable_periph_clk(ID_PIOA);
    pmc_enable_periph_clk(ID_USART0);

    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA10A_RXD0, PIO_PULLUP);
    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA11A_TXD0, PIO_DEFAULT);

    usart_init_rs232(USART0, &usart_opt, SystemCoreClock);
    usart_reset_tx(USART0);
    usart_enable_tx(USART0);
}

static void log_char(char c)
{
    while (!usart_is_tx_ready(USART0) && !usart_is_tx_empty(USART0));
    usart_write(USART0, c);
}

void log_string(char *str)
{
    while (*str != '\0') {
        log_char(*str);
        str++;
    }
}

void log_nl()
{
    log_string("\r\n");
}

void log_hex(uint32_t n)
{
    uint8_t c;
    int i;

    for (i = 0; i < 8; i++) {
        c = (n >> 28);
        log_char(c + (c > 9 ? 'a' - 10 : '0'));
        n <<= 4;
    }
}

void set_led(uint32_t on)
{
    pio_set_output(PIOB, PIO_PB27, on, 0, PIO_PULLUP);
}

void init_uart(void)
{
    struct sam_uart_opt uart_opt = {
        .ul_baudrate = 115200,
        .ul_mode = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO,
    };

    pmc_enable_periph_clk(ID_PIOA);
    pmc_enable_periph_clk(ID_UART);

    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA8A_URXD, PIO_PULLUP) ;
    pio_configure(PIOA, PIO_PERIPH_A, PIO_PA9A_UTXD, PIO_DEFAULT) ;

    uart_opt.ul_mck = SystemCoreClock;
    uart_init(UART, &uart_opt);

    uart_enable_interrupt(UART, UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME);
    NVIC_EnableIRQ(UART_IRQn);
}

void send_byte(uint8_t c)
{
    while (!uart_is_tx_ready(UART) && !uart_is_tx_empty(UART));
    uart_write(UART, c);
}

void send_string(char *str)
{
    while (*str != '\0') {
        send_byte(*str);
        str++;
    }
}

uint8_t read_byte(void)
{
    uint8_t r;
    int32_t *c;

    c = &uart_rcv_char;
    while (*c == -1) __WFI();
    r = *c & 0xff;
    *c = -1;
    return r;
}

uint32_t read_long(void)
{
    return read_byte() + (read_byte() << 8) + (read_byte() << 16) + (read_byte() << 24);
}

void __libc_init_array(void)
{
}
