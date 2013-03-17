#include "board.h"
#include "common.h"

uint32_t samples[2048];
uint32_t samples_count;

volatile uint32_t reset_control;

void print_samples(void)
{
    int i;
    uint32_t timer;

    for (timer =0, i = 0; i < ARRAY_COUNT(samples) && !reset_control; i++) {
        if (samples[i] & 0x80000000) {
            timer++;
        }
        log_hex(samples[i]);
        log_string(" ");
        log_hex(((0x00ffffff - (samples[i] & 0x00ffffff)) / DIVIDER) + timer * 0x01000000);
        log_string("\r\n");
    }
}

void delay(uint32_t n)
{
    while (n--) ;
}

void send_samples(void)
{
    int i;
    uint32_t sample, last_sample;
    uint32_t t, prev_t, sample_t;
    uint32_t timer;

    log_string("Sending samples.\r\n");
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
                send_byte((last_sample >> 24) & 0x7f);
                t++;
            }
            if (t != prev_t) {
                send_byte((sample >> 24) & 0x7f);
                last_sample = sample;
                prev_t = t;
            }
        }
    }
    log_string("Sending done.\r\n");
}

void start_sampling(void)
{
    int i;

    log_string("Start sampling.\r\n");
    for (i = 0; i < ARRAY_COUNT(samples); i++) samples[i] = 0;
    samples_count = sampler(samples, ARRAY_COUNT(samples), &reset_control, SAMPLING_GPIO);
    log_string("Sampling done, count: ");
    log_hex(samples_count);
    log_string(" control: ");
    log_hex(reset_control);
    log_string("\r\n");
    send_samples();
}

void generic_long_command()
{
    uint32_t parameter;

    parameter = read_long();
    log_string("Parameter: ");
    log_hex(parameter);
    log_string("\r\n");
}

void sump_handler(void)
{
    uint8_t command;

    while (1) {
        set_led(0);
        command = read_byte();
        set_led(1);
        log_string("Got command: ");
        log_hex(command);
        log_string("\r\n");
        switch (command) {
        case 0x00: break;
        case 0x01: start_sampling(); break;
        case 0x02: send_string("1ALS"); break;
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
    init_board();
    init_led();
    init_button();
    init_log();
    init_uart();

    log_string("Ready.\r\n");
    sump_handler();
    return 0;
}

