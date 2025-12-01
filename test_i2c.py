#!/usr/bin/env python3
"""
Script de test basique de la communication I2C avec l'Arduino
Teste la connectivité et la lecture/écriture des registres
"""

import sys
try:
    import smbus2
except ImportError:
    print("❌ Erreur: module smbus2 non installé")
    print("   Installation: pip install smbus2")
    sys.exit(1)

import time

# Configuration
SLAVE_ADDRESS = 0x32
I2C_BUS = 1

def test_i2c_detection():
    """Test 1: Détection du slave I2C"""
    print("\n" + "="*60)
    print("TEST 1: Détection du slave I2C")
    print("="*60)
    
    try:
        bus = smbus2.SMBus(I2C_BUS)
        print(f"✓ Bus I2C {I2C_BUS} ouvert avec succès")
        
        # Tentative de lecture pour vérifier la présence
        try:
            bus.read_byte_data(SLAVE_ADDRESS, 0)
            print(f"✓ Slave détecté à l'adresse 0x{SLAVE_ADDRESS:02X}")
            bus.close()
            return True
        except Exception as e:
            print(f"❌ Slave non détecté à l'adresse 0x{SLAVE_ADDRESS:02X}")
            print(f"   Erreur: {e}")
            bus.close()
            return False
            
    except Exception as e:
        print(f"❌ Impossible d'ouvrir le bus I2C {I2C_BUS}")
        print(f"   Erreur: {e}")
        return False

def test_register_read():
    """Test 2: Lecture des registres"""
    print("\n" + "="*60)
    print("TEST 2: Lecture des registres")
    print("="*60)
    
    try:
        bus = smbus2.SMBus(I2C_BUS)
        success = True
        
        registers = {
            0: "REG_CAR_STATE",
            1: "REG_LIGHT_STATE",
            2: "REG_SERVO_ANGLE",
            3: "REG_LED_STATE",
            4: "REG_RELEASE_COUNTER",
            5: "REG_SYSTEM_STATUS"
        }
        
        for reg, name in registers.items():
            try:
                value = bus.read_byte_data(SLAVE_ADDRESS, reg)
                print(f"✓ Registre {reg} ({name:20s}): 0x{value:02X} ({value:3d})")
            except Exception as e:
                print(f"❌ Erreur lecture registre {reg} ({name}): {e}")
                success = False
        
        bus.close()
        return success
        
    except Exception as e:
        print(f"❌ Erreur lors de l'ouverture du bus: {e}")
        return False

def test_register_write():
    """Test 3: Écriture dans un registre (REG 6 - test)"""
    print("\n" + "="*60)
    print("TEST 3: Écriture dans un registre")
    print("="*60)
    
    try:
        bus = smbus2.SMBus(I2C_BUS)
        test_reg = 6  # Registre de test
        test_values = [0x00, 0x55, 0xAA, 0xFF]
        
        for value in test_values:
            try:
                bus.write_byte_data(SLAVE_ADDRESS, test_reg, value)
                print(f"✓ Écriture 0x{value:02X} dans registre {test_reg}")
                time.sleep(0.1)
                
                # Vérification (si le slave implémente la lecture de ce registre)
                read_value = bus.read_byte_data(SLAVE_ADDRESS, test_reg)
                if read_value == value:
                    print(f"  ✓ Vérification OK: lu 0x{read_value:02X}")
                else:
                    print(f"  ⚠️  Valeur différente: lu 0x{read_value:02X} (attendu 0x{value:02X})")
                    
            except Exception as e:
                print(f"❌ Erreur écriture 0x{value:02X}: {e}")
                bus.close()
                return False
        
        bus.close()
        return True
        
    except Exception as e:
        print(f"❌ Erreur lors de l'ouverture du bus: {e}")
        return False

def test_continuous_read():
    """Test 4: Lecture continue des registres principaux"""
    print("\n" + "="*60)
    print("TEST 4: Lecture continue (5 secondes)")
    print("="*60)
    print("Appuyez sur Ctrl+C pour arrêter plus tôt\n")
    
    try:
        bus = smbus2.SMBus(I2C_BUS)
        start_time = time.time()
        
        while time.time() - start_time < 5:
            try:
                car = bus.read_byte_data(SLAVE_ADDRESS, 0)
                light = bus.read_byte_data(SLAVE_ADDRESS, 1)
                servo = bus.read_byte_data(SLAVE_ADDRESS, 2)
                leds = bus.read_byte_data(SLAVE_ADDRESS, 3)
                counter = bus.read_byte_data(SLAVE_ADDRESS, 4)
                
                print(f"Car:{car} Light:{light} Servo:{servo:3d}° LEDs:0x{leds:02X} Cnt:{counter:2d}", end='\r')
                time.sleep(0.2)
                
            except Exception as e:
                print(f"\n❌ Erreur lecture: {e}")
                bus.close()
                return False
        
        print("\n✓ Lecture continue terminée avec succès")
        bus.close()
        return True
        
    except KeyboardInterrupt:
        print("\n⚠️  Arrêt par l'utilisateur")
        bus.close()
        return True
    except Exception as e:
        print(f"\n❌ Erreur: {e}")
        return False

def test_timing():
    """Test 5: Performance de lecture"""
    print("\n" + "="*60)
    print("TEST 5: Performance de lecture")
    print("="*60)
    
    try:
        bus = smbus2.SMBus(I2C_BUS)
        iterations = 100
        
        start = time.time()
        for _ in range(iterations):
            bus.read_byte_data(SLAVE_ADDRESS, 0)
        duration = time.time() - start
        
        avg_time = (duration / iterations) * 1000  # en ms
        print(f"✓ {iterations} lectures effectuées en {duration:.2f}s")
        print(f"  Temps moyen par lecture: {avg_time:.2f}ms")
        print(f"  Fréquence max théorique: {1000/avg_time:.1f} Hz")
        
        bus.close()
        return True
        
    except Exception as e:
        print(f"❌ Erreur: {e}")
        return False

def main():
    print("\n" + "🔧 "*20)
    print("     TEST DE COMMUNICATION I2C - SYSTÈME PARKING")
    print("🔧 "*20)
    print(f"\nConfiguration:")
    print(f"  - Bus I2C: {I2C_BUS}")
    print(f"  - Adresse slave: 0x{SLAVE_ADDRESS:02X} ({SLAVE_ADDRESS})")
    
    # Exécution des tests
    tests_results = []
    
    tests_results.append(("Détection slave", test_i2c_detection()))
    
    if tests_results[-1][1]:  # Si la détection a réussi
        tests_results.append(("Lecture registres", test_register_read()))
        tests_results.append(("Écriture registre", test_register_write()))
        tests_results.append(("Lecture continue", test_continuous_read()))
        tests_results.append(("Performance", test_timing()))
    else:
        print("\n⚠️  Tests suivants annulés (slave non détecté)")
    
    # Résumé
    print("\n" + "="*60)
    print("RÉSUMÉ DES TESTS")
    print("="*60)
    
    for test_name, result in tests_results:
        status = "✓ PASS" if result else "❌ FAIL"
        print(f"{status} - {test_name}")
    
    passed = sum(1 for _, result in tests_results if result)
    total = len(tests_results)
    
    print(f"\nRésultat: {passed}/{total} tests réussis")
    
    if passed == total:
        print("🎉 Tous les tests sont passés!")
        return 0
    else:
        print("⚠️  Certains tests ont échoué")
        return 1

if __name__ == "__main__":
    try:
        exit_code = main()
        sys.exit(exit_code)
    except KeyboardInterrupt:
        print("\n\n👋 Arrêt des tests par l'utilisateur")
        sys.exit(0)
