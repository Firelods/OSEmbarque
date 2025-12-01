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

static SemaphoreHandle_t lcdSem;

volatile uint8_t car_state = 0;
volatile uint8_t prev_car_state = 255;
volatile uint8_t current_servo_angle = 0;
volatile uint8_t current_release_counter = 0;

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

        // Update I2C register
        soft_i2c_set_register(REG_CAR_STATE, car_state);

        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

static void vControlTask(void *p)
{
    uint8_t release_counter = 0;
    bool manual_servo_mode = false;  // Mode manuel du servo activé

    for(;;)
    {
        uint8_t dark = is_dark();
        uint8_t led_state = 0;
        
        // Vérifier s'il y a une commande servo du master
        uint8_t servo_command = soft_i2c_get_register(REG_SERVO_COMMAND);
        
        // 255 = réactiver le mode automatique
        if (servo_command == 255)
        {
            manual_servo_mode = false;
            soft_i2c_set_register(REG_SERVO_COMMAND, 0xFF);
        }
        // Commande d'angle valide (0-180)
        else if (servo_command != 0xFF && servo_command <= 180)
        {
            servo_set_angle(servo_command);
            current_servo_angle = servo_command;
            manual_servo_mode = true;  // Activer le mode manuel
            soft_i2c_set_register(REG_SERVO_COMMAND, 0xFF);
        }

        // Gestion de la LED blanche (automatique selon luminosité)
        if (dark)
        {
            PORTD |=  (1<<WHITE_LED);
            led_state |= 0x04;  // bit 2 for WHITE
        }
        else
        {
            PORTD &= ~(1<<WHITE_LED);
        }

        // Logique automatique seulement si pas en mode manuel
        if (!manual_servo_mode)
        {
            if (car_state)
            {
                PORTD |=  (1<<RED_LED);
                PORTD &= ~(1<<GREEN_LED);
                led_state |= 0x01;  // bit 0 for RED

                servo_set_angle(180);
                current_servo_angle = 180;
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
                    led_state |= 0x02;  // bit 1 for GREEN
                    servo_set_angle(0);
                    current_servo_angle = 0;
                }
            }
        }
        // En mode manuel, garder les LEDs selon l'état actuel du parking
        else
        {
            if (car_state)
            {
                PORTD |=  (1<<RED_LED);
                PORTD &= ~(1<<GREEN_LED);
                led_state |= 0x01;
            }
            else if (release_counter >= 40)
            {
                PORTD |=  (1<<GREEN_LED);
                PORTD &= ~(1<<RED_LED);
                led_state |= 0x02;
            }
            
            // Gérer le compteur même en mode manuel
            if (car_state)
                release_counter = 0;
            else if (release_counter < 40)
                release_counter++;
        }

        current_release_counter = release_counter;

        // Update I2C registers
        soft_i2c_set_register(REG_LIGHT_STATE, dark);
        soft_i2c_set_register(REG_SERVO_ANGLE, current_servo_angle);
        soft_i2c_set_register(REG_LED_STATE, led_state);
        soft_i2c_set_register(REG_RELEASE_COUNTER, release_counter);
        soft_i2c_set_register(REG_SYSTEM_STATUS, 0x01);  // System OK

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
    soft_i2c_init(0x32);   // adresse I2C esclave
    
    // Initialiser le registre de commande servo à 0xFF (pas de commande)
    soft_i2c_set_register(REG_SERVO_COMMAND, 0xFF);
    
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
