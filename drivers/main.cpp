#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "led.h"
#include "button.h"
#include "ultrasonic.h"
#include "motor.h"

//
// -----------------------
//     UART DEBUG
// -----------------------
//

void uart_init() {
    // 115200 bauds @ 16 MHz
    uint16_t ubrr = 8;
    UBRR0H = (ubrr >> 8);
    UBRR0 = ubrr;
    UCSR0B = (1 << TXEN0);                     // enable TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);    // 8N1
}

void uart_send(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_print(const char* s) {
    while (*s) uart_send(*s++);
}

void uart_print_uint(uint16_t n) {
    char buf[10];
    itoa(n, buf, 10);
    uart_print(buf);
}

//
// -----------------------
//     TEST FUNCTIONS
// -----------------------
//

void test_led() {
    uart_print("[TEST] LED toggle\r\n");
    led_toggle();
}

void test_button() {
    if (button_get_event()) {
        uart_print("[TEST] Button pressed\r\n");
        led_toggle();
        _delay_ms(150);
    }
}

void test_ultrasonic() {
    uart_print("[TEST] Ultrasonic measure... ");
    uint16_t d = us_measure_cm();
    uart_print_uint(d);
    uart_print(" cm\r\n");
    _delay_ms(300);
}

void test_motor() {
    uart_print("[TEST] Motor PWM speed sweep\r\n");

    // Ramp-up
    for (uint8_t i = 0; i < 255; i += 25) {
        motor_set_speed(i);
        uart_print("  Speed = "); uart_print_uint(i); uart_print("\r\n");
        _delay_ms(150);
    }

    // Ramp-down
    for (int i = 255; i >= 0; i -= 25) {
        motor_set_speed(i);
        uart_print("  Speed = "); uart_print_uint(i); uart_print("\r\n");
        _delay_ms(150);
    }
}

//
// -----------------------
//        MAIN
// -----------------------
//

int main(void)
{
    // Initialize all drivers
    led_init();
    button_init();
    // motor_init();
    us_init();
    uart_init();

    sei();  // enable interrupts for button

    uart_print("\r\n=== SMART SENSOR - DRIVER SELFTEST ===\r\n");

    led_set(0);
    // motor_set_speed(0);

    //
    // Boucle de test infinie
    //
    while (1)
    {
        uart_print("\r\n>>> Running complete driver test cycle...\r\n");

        test_led();
        _delay_ms(250);

        test_button();  // non bloquant

        test_ultrasonic();

        // test_motor();

        uart_print(">>> END TEST CYCLE\r\n");
        uart_print("--------------------------\r\n");

        _delay_ms(1000);
    }

    return 0;
}
