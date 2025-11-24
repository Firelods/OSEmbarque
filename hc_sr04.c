#include "hc_sr04.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

HC_SR04::HC_SR04(TPin* trig, TPin* echo)
{
    // Save pointer to pins
    this->trig = trig;
    this->echo = echo;
    // Set pins as output and input
    this->trig->mode(EPinMode::OUTPUT);
    this->echo->mode(EPinMode::INPUT);
}

float HC_SR04::dist_cm()
{
    // Send trigger
    trig->set(0); // Set trigger as low
    _delay_us(2); // Wait 2 micro seconds
    cli(); // Disable interrupts
    trig->set(1); // Set trigger as high
    _delay_us(10); // Wait 10 micro seconds
    trig->set(0); // Set trigger as low

    TCNT1 = 0;  // Reset Timer1
    TCCR1B = (1 << CS11); // Start Timer1 with prescaler 8
    while (!echo->read()); // Wait for Echo HIGH
    TCNT1 = 0; // Reset Timer1 again
    while (echo->read()); // Wait for Echo LOW
    TCCR1B = 0; // Stop Timer1
    sei(); // Enable interrupts

    return ((float)TCNT1 / 2.0) * 0.0343 / 2.0;
}