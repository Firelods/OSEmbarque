#!/usr/bin/env python3
"""
Scanner I2C - Détecte tous les périphériques I2C sur le bus
"""

import sys
try:
    import smbus2
except ImportError:
    print("❌ Erreur: module smbus2 non installé")
    print("   Installation: pip install smbus2")
    sys.exit(1)

I2C_BUS = 1

def scan_i2c():
    """Scanne le bus I2C pour trouver tous les périphériques"""
    print("🔍 Scan du bus I2C...\n")
    print("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f")
    
    try:
        bus = smbus2.SMBus(I2C_BUS)
        found_devices = []
        
        for row in range(0, 0x80, 0x10):
            print(f"{row:02x}: ", end="")
            
            for col in range(0x10):
                addr = row + col
                
                # Skip reserved addresses
                if addr < 0x03 or addr > 0x77:
                    print("   ", end="")
                    continue
                
                try:
                    # Try to read from device
                    bus.read_byte(addr)
                    print(f"{addr:02x} ", end="")
                    found_devices.append(addr)
                except:
                    print("-- ", end="")
            
            print()
        
        bus.close()
        
        print(f"\n{'='*60}")
        if found_devices:
            print(f"✓ {len(found_devices)} périphérique(s) I2C trouvé(s):")
            for addr in found_devices:
                print(f"  - 0x{addr:02X} (décimal: {addr})")
                if addr == 0x32:
                    print("    ^ C'est notre Arduino! ✓")
        else:
            print("❌ Aucun périphérique I2C détecté")
            print("\nVérifications à faire:")
            print("  1. Câblage SDA/SCL correct")
            print("  2. Arduino allumé et programmé")
            print("  3. Résistances pull-up (4.7kΩ) sur SDA et SCL")
            print("  4. I2C activé sur Raspberry Pi:")
            print("     sudo raspi-config → Interface Options → I2C → Enable")
        print("="*60)
        
        return found_devices
        
    except Exception as e:
        print(f"\n❌ Erreur lors du scan: {e}")
        print("\nVérifiez que l'I2C est activé:")
        print("  sudo raspi-config → Interface Options → I2C → Enable")
        return []

if __name__ == "__main__":
    scan_i2c()
