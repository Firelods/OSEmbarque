#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ir.h"
#include "servo.h"
#include "lcd_grove.h"
#include "soft_i2c.h"


// ------------ PIN DEFINITIONS ------------
#define RED_LED     PD2
#define GREEN_LED   PD4
#define WHITE_LED   PD3     // <<< moved from D6 to D3
#define LIGHT_PIN   PC0

// ------------ I2C REGISTER DEFINITIONS ------------
#define REG_CAR_STATE       0
#define REG_LIGHT_STATE     1
#define REG_SERVO_ANGLE     2
#define REG_LED_STATE       3
#define REG_RELEASE_COUNTER 4
#define REG_SYSTEM_STATUS   5
#define REG_SERVO_COMMAND   6  // Commande manuelle du servo depuis le master
#define REG_CHANGE_FLAG     7  // Flag indiquant qu'une donnée a changé (1=changé, 0=stable)

#define BARRIER_OPEN_DURATION 100 // 100 * 50ms = 5000ms = 5 seconds


static SemaphoreHandle_t lcdSem;

volatile uint8_t car_state = 0;
volatile uint8_t prev_car_state = 255;
volatile uint16_t current_servo_angle = 0;
volatile uint8_t current_release_counter = 0; // Shared with LED task
volatile uint8_t is_dark_state = 0;           // Shared with LED task
volatile uint8_t prev_light_state = 255;
volatile uint8_t prev_servo_angle = 255;
volatile uint8_t prev_led_state = 255;

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

// Helper to mark data as changed
static void mark_data_changed(void)
{
    soft_i2c_set_register(REG_CHANGE_FLAG, 1);
}

// ===================================================
//                      TASKS
// ===================================================

// Task 1: Infrared Sensor Task
// Reads the IR sensor and updates the car presence state.
static void vIrTask(void *p)
{
    for(;;)
    {
        car_state = ir_detect();

        if (car_state != prev_car_state)
        {
            prev_car_state = car_state;
            xSemaphoreGive(lcdSem);
            mark_data_changed();  // Mark that data has changed
        }

        // Update I2C register
        soft_i2c_set_register(REG_CAR_STATE, car_state);

        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

// Task 2: Light Sensor Task
// Reads the light sensor and updates the dark/light state.
static void vLightSensorTask(void *p)
{
    for(;;)
    {
        is_dark_state = is_dark();

        // Check if changed
        if (is_dark_state != prev_light_state)
        {
            prev_light_state = is_dark_state;
            mark_data_changed();
        }

        // Update I2C register
        soft_i2c_set_register(REG_LIGHT_STATE, is_dark_state);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task 3: Servo Motor Task
// Manages the Parking Logic (release counter) and Servo control (Auto/Manual).
static void vServoTask(void *p)
{
    uint8_t release_counter = 0;
    bool manual_servo_mode = false;

    for(;;)
    {
        // 1. Check for Manual Command via I2C
        uint8_t servo_command = soft_i2c_get_register(REG_SERVO_COMMAND);
        
        // Command 255: Reactivate Automatic Mode
        if (servo_command == 255)
        {
            manual_servo_mode = false;
            soft_i2c_set_register(REG_SERVO_COMMAND, 0);  // Clear command
        }
        // Valid Angle Command (0-180): Activate Manual Mode
        else if (servo_command != 0 && servo_command <= 180)
        {
            // Convertir les degrés (0-180) en unités servo (0-1620)
            // 1 degré = 9 unités (180° * 9 = 1620)
            uint16_t servo_units = servo_command * 9;
            servo_set_angle(servo_units);
            current_servo_angle = servo_units;
            manual_servo_mode = true;
            soft_i2c_set_register(REG_SERVO_COMMAND, 0);  // Clear command
        }

        // 2. Manage Release Counter (System State Logic)
        // Counter runs regardless of manual mode to keep LED state consistent
        if (car_state)
        {
            release_counter = 0;
        }
        else
        {
            if (release_counter < BARRIER_OPEN_DURATION)
                release_counter++;
        }
        current_release_counter = release_counter; // Update global for LED task

        // 3. Automatic Servo Control
        if (!manual_servo_mode)
        {
            if (car_state)
            {
                // Car detected -> Open barrier
                servo_set_angle(1080);
                current_servo_angle = 1080;
            }
            else
            {
                // Car gone -> Wait for counter -> Close barrier
                if (release_counter >= BARRIER_OPEN_DURATION)
                {
                    servo_set_angle(0);
                    current_servo_angle = 0;
                }
            }
        }

        // Check if servo angle changed
        if ((uint8_t)current_servo_angle != prev_servo_angle)
        {
            prev_servo_angle = (uint8_t)current_servo_angle;
            mark_data_changed();
        }

        // Update I2C registers
        soft_i2c_set_register(REG_SERVO_ANGLE, (uint8_t)current_servo_angle);
        soft_i2c_set_register(REG_RELEASE_COUNTER, release_counter);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task 4: LED Task
// Controls all LEDs based on shared state (Light, Car, Counter).
static void vLedTask(void *p)
{
    for(;;)
    {
        uint8_t led_state = 0;

        // White LED Control (Ambient Light)
        if (is_dark_state)
        {
            PORTD |=  (1<<WHITE_LED);
            led_state |= 0x04; // bit 2 for WHITE
        }
        else
        {
            PORTD &= ~(1<<WHITE_LED);
        }

        // Red/Green LED Control (Traffic Light)
        if (car_state)
        {
            // Car detected -> RED (Stop/Occupied)
            PORTD |=  (1<<RED_LED);
            PORTD &= ~(1<<GREEN_LED);
            led_state |= 0x01; // bit 0 for RED
        }
        else
        {
            if (current_release_counter >= BARRIER_OPEN_DURATION)
            {
                // Car gone + Delay passed -> GREEN (Free)
                PORTD |=  (1<<GREEN_LED);
                PORTD &= ~(1<<RED_LED);
                led_state |= 0x02; // bit 1 for GREEN
            }
            else
            {
                // Car gone but still waiting -> maintain RED
                PORTD |=  (1<<RED_LED);
                PORTD &= ~(1<<GREEN_LED);
                led_state |= 0x01; // bit 0 for RED
            }
        }

        // Check if LED state changed
        if (led_state != prev_led_state)
        {
            prev_led_state = led_state;
            mark_data_changed();
        }

        // Update I2C
        soft_i2c_set_register(REG_LED_STATE, led_state);
        soft_i2c_set_register(REG_SYSTEM_STATUS, 0x01);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ===================================================
//                     MAIN
// ===================================================
int main(void)
{
    soft_i2c_init(0x32);   // adresse I2C esclave

    // Initialiser les registres
    soft_i2c_set_register(REG_SERVO_COMMAND, 0);  // Pas de commande (0 = inactif)
    soft_i2c_set_register(REG_CHANGE_FLAG, 0);       // No changes yet
    
    leds_init();
    light_sensor_init();
    ir_init();
    servo_init();   // now using Timer0 OC0A on D6

    lcdSem = xSemaphoreCreateBinary();

    // Create Tasks
    xTaskCreate(vIrTask,          "IR",   100, NULL, 3, NULL); // Detection priority
    xTaskCreate(vServoTask,       "SERV", 130, NULL, 2, NULL); // Logic priority
    xTaskCreate(vLedTask,         "LED",  100, NULL, 2, NULL); // Visual priority
    xTaskCreate(vLightSensorTask, "LGT",  80,  NULL, 1, NULL); // Low priority
    vTaskStartScheduler();

    while(1);
}
