#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ir.h"
#include "servo.h"
#include "lcd_grove.h"

#define RED_LED   PD2
#define GREEN_LED PD3
#define WHITE_LED PD6
#define LIGHT_PIN PC0    // A0 for light sensor

void leds_init(void)
{
    DDRD |= (1<<RED_LED) | (1<<GREEN_LED) | (1<<WHITE_LED);
    DDRC &= ~(1<<LIGHT_PIN);   // A0 as input
}

uint8_t is_dark(void)
{
    // simple digital threshold: LOW = dark (LDR + pull-up)
    return !(PINC & (1<<LIGHT_PIN));
}

int main(void)
{
    leds_init();
    ir_init();
    servo_init();
    lcd_init();     // <-- new

    sei();

    // Initial LCD message
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Parking Ready");

    uint8_t last_car_state = 255;   // force first update

    while (1)
    {
        // LIGHT SENSOR -> white LED
        if (is_dark())
            PORTD |= (1<<WHITE_LED);
        else
            PORTD &= ~(1<<WHITE_LED);

        // IR DETECTOR -> car present or not
        uint8_t car = ir_detect();

        if (car != last_car_state)
        {
            if (car) {
                // spot occupied
                PORTD |=  (1<<RED_LED);
                PORTD &= ~(1<<GREEN_LED);

                // barrier down
                servo_set_angle(90);

                lcd_clear();
                lcd_set_cursor(0,0);
                lcd_print("Car detected");
            }
            else {
                // spot free
                PORTD |=  (1<<GREEN_LED);
                PORTD &= ~(1<<RED_LED);

                // barrier up
                servo_set_angle(0);

                lcd_clear();
                lcd_set_cursor(0,0);
                lcd_print("Free spot");
            }

            last_car_state = car;
        }

        _delay_ms(200);
    }

    return 0;
}
