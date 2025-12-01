#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ir.h"
#include "servo.h"
#include "lcd_grove.h"

// ------------ PIN DEFINITIONS ------------
#define RED_LED     PD2
#define GREEN_LED   PD4
#define WHITE_LED   PD3     // <<< moved from D6 to D3
#define LIGHT_PIN   PC0

static SemaphoreHandle_t lcdSem;

volatile uint8_t car_state = 0;
volatile uint8_t prev_car_state = 255;

// ------------ INIT ------------
static void leds_init(void)
{
    DDRD |= (1<<RED_LED) | (1<<GREEN_LED) | (1<<WHITE_LED);
}

static void light_sensor_init(void)
{
    DDRC &= ~(1<<LIGHT_PIN);
    PORTC |= (1<<LIGHT_PIN);
}

static uint8_t is_dark(void)
{
    return !(PINC & (1<<LIGHT_PIN));
}

// ===================================================
//                      TASKS
// ===================================================

static void vAcqTask(void *p)
{
    for(;;)
    {
        car_state = ir_detect();

        if (car_state != prev_car_state)
        {
            prev_car_state = car_state;
            xSemaphoreGive(lcdSem);
        }

        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

static void vControlTask(void *p)
{
    uint8_t release_counter = 0;

    for(;;)
    {
        if (is_dark())
            PORTD |=  (1<<WHITE_LED);
        else
            PORTD &= ~(1<<WHITE_LED);

        if (car_state)
        {
            PORTD |=  (1<<RED_LED);
            PORTD &= ~(1<<GREEN_LED);

            servo_set_angle(180);
            release_counter = 0;
        }
        else
        {
            if (release_counter < 40)
                release_counter++;
            else
            {
                PORTD |=  (1<<GREEN_LED);
                PORTD &= ~(1<<RED_LED);
                servo_set_angle(0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void vLCDTask(void *p)
{
    lcd_init();
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_print("Parking Ready");

    for(;;)
    {
        xSemaphoreTake(lcdSem, portMAX_DELAY);

        lcd_clear();
        lcd_set_cursor(0,0);

        if (car_state)
            lcd_print("Car detected");
        else
            lcd_print("Free spot");
    }
}

// ===================================================
//                     MAIN
// ===================================================
int main(void)
{
    leds_init();
    light_sensor_init();
    ir_init();
    servo_init();   // now using Timer0 OC0A on D6

    lcdSem = xSemaphoreCreateBinary();

    xTaskCreate(vAcqTask,     "ACQ",   150, NULL, 3, NULL);
    xTaskCreate(vControlTask, "CTRL",  200, NULL, 2, NULL);
    xTaskCreate(vLCDTask,     "LCD",   200, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1);
}
