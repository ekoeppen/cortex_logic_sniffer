#include "sam3x8e.h"

extern volatile uint32_t reset_control;

int32_t usart1_rcv_char = -1;
int32_t usart2_rcv_char = -1;

void init_led(void)
{
}

void init_button(void)
{
}

void init_log(void)
{
}

void log_string(char *str)
{
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
