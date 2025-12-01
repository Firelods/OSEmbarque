#include "soft_i2c.h"
#include <Wire.h>

// Registres I2C (16 registres disponibles)
static volatile uint8_t registers[16];
static volatile uint8_t current_register = 0;
static volatile bool register_selected = false;

void soft_i2c_set_register(uint8_t reg, uint8_t value)
{
    if (reg < sizeof(registers))
        registers[reg] = value;
}

uint8_t soft_i2c_get_register(uint8_t reg)
{
    if (reg < sizeof(registers))
        return registers[reg];
    return 0xFF;
}

// Callback appelé quand le master envoie des données
void receiveEvent(int numBytes)
{
    if (numBytes > 0)
    {
        // Premier byte = adresse du registre
        current_register = Wire.read();
        register_selected = true;
        numBytes--;
        
        // Bytes suivants = données à écrire dans les registres
        while (numBytes > 0 && Wire.available())
        {
            if (current_register < sizeof(registers))
            {
                registers[current_register] = Wire.read();
                current_register++;
            }
            else
            {
                Wire.read(); // Ignorer les données en surplus
            }
            numBytes--;
        }
    }
}

// Callback appelé quand le master demande des données
void requestEvent()
{
    if (register_selected && current_register < sizeof(registers))
    {
        Wire.write(registers[current_register]);
        current_register++;
        
        // Retour au début si on dépasse
        if (current_register >= sizeof(registers))
            current_register = 0;
    }
    else
    {
        Wire.write(0xFF); // Valeur par défaut
    }
}

void soft_i2c_init(uint8_t address)
{
    // Initialiser tous les registres à 0
    for (uint8_t i = 0; i < sizeof(registers); i++)
        registers[i] = 0;
    
    // Initialiser Wire en mode slave
    Wire.begin(address);
    
    // Enregistrer les callbacks
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
}
