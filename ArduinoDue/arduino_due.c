#include "sam3x8e.h"
#include "pmc/pmc.h"
#include "pio/pio.h"
#include "pio/pio_sam3x8e.h"
#include "uart/uart.h"

extern volatile uint32_t reset_control;

int32_t usart1_rcv_char = -1;
int32_t usart2_rcv_char = -1;

void init_board(void)
{
    SystemInit();
}

void init_led(void)
{
    pio_configure(PIOB, PIO_OUTPUT_0, PIO_PB27, PIO_DEFAULT) ;
}

void init_button(void)
{
}

void init_log(void)
{
    struct sam_uart_opt uart_opt = {
	.ul_baudrate = 115200,
	.ul_mode = UART_MR_CHMODE_NORMAL,
	.ul_chmode = UART_MR_PAR_NO
    };

    uart_opt.ul_mck = SystemCoreClock;
    pmc_enable_periph_clk(ID_UART);
    uart_init(UART, &uart_opt);
    uart_enable_tx(UART);
}

void log_string(char *str)
{
    while (*str != '\0') {
        uart_write(UART, *str);
        str++;
    }
}

void log_nl()
{
    log_string("\r\n");
}

void log_hex(uint32_t n)
{
}

void set_led(uint32_t on)
{
    pio_set_output(PIOB, PIO_PB27, on, 0, PIO_PULLUP);
}

void init_uart(void)
{
}

void send_byte(uint8_t byte)
{
}

void send_string(char *string)
{
}

uint8_t read_byte(void)
{
}

uint32_t read_long(void)
{
    return read_byte() + (read_byte() << 8) + (read_byte() << 16) + (read_byte() << 24);
}

void __libc_init_array(void)
{
}
