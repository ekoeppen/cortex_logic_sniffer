#ifndef __COMMON_H
#define __COMMON_H

void init_board(void);

void init_log(void);
void init_button(void);
void init_led(void);
void init_uart(void);

void log_string(char *str);
void log_nl(void);
void log_hex(uint32_t n);

void set_led(uint32_t on);

void send_byte(uint8_t byte);
void send_string(char *string);
uint8_t read_byte(void);
uint32_t read_long(void);

uint32_t sampler(uint32_t *samples, uint32_t count, volatile uint32_t *reset_control);

extern volatile uint32_t reset_control;

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#define RESET_CMD 0x01
#define BUTTON_PRESSED 0x02

#endif /* __COMMON_H */
