#include "ir.h"

#define IR_PIN PB0   // Arduino D8

void ir_init(void)
{
    DDRB &= ~(1 << IR_PIN);   // Set PB0 as INPUT
    PORTB |= (1 << IR_PIN);   // Enable pull-up (optional but recommended)
}

uint8_t ir_detect(void)
{
    // FC-51 outputs:
    //   LOW  -> obstacle detected
    //   HIGH -> no obstacle
    return (PINB & (1 << IR_PIN)) ? 0 : 1;
}
