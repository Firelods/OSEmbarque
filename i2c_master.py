#!/usr/bin/env python3
"""
Master I2C pour communiquer avec l'Arduino Uno (slave I2C)
Ce script permet de récupérer l'état du système de parking depuis l'Arduino
Basé sur l'implémentation soft_i2c avec système de registres
"""

import smbus2
import time
import argparse

# Configuration I2C
SLAVE_ADDRESS = 0x32  # Adresse I2C de l'Arduino slave (0x32 par défaut)
I2C_BUS = 1           # Bus I2C (1 pour Raspberry Pi)

# Registres I2C (système de 16 registres)
REG_CAR_STATE = 0      # État de détection de voiture (0 ou 1)
REG_LIGHT_STATE = 1    # État du capteur de lumière (0=clair, 1=sombre)
REG_SERVO_ANGLE = 2    # Angle actuel du servo (0-180)
REG_LED_STATE = 3      # État des LEDs (bit0=RED, bit1=GREEN, bit2=WHITE)
REG_RELEASE_COUNTER = 4  # Compteur de release (0-40)
REG_SYSTEM_STATUS = 5  # Status général du système
REG_SERVO_COMMAND = 6  # Commande manuelle servo
REG_CHANGE_FLAG = 7    # Flag indiquant si données ont changé (1=changé, 0=stable)


class ParkingMaster:
    """Classe pour gérer la communication I2C avec le système de parking Arduino"""
    
    def __init__(self, bus_num=I2C_BUS, slave_addr=SLAVE_ADDRESS):
        """
        Initialise le bus I2C
        
        Args:
            bus_num: Numéro du bus I2C
            slave_addr: Adresse I2C de l'Arduino slave
        """
        self.bus = smbus2.SMBus(bus_num)
        self.slave_addr = slave_addr
        
    def read_register(self, reg):
        """
        Lit un registre I2C
        
        Args:
            reg: Numéro du registre (0-15)
            
        Returns:
            Valeur du registre ou None en cas d'erreur
        """
        try:
            value = self.bus.read_byte_data(self.slave_addr, reg)
            time.sleep(0.001)  # Délai de 1ms entre les lectures
            return value
        except Exception as e:
            print(f"Erreur lors de la lecture du registre {reg}: {e}")
            return None
    
    def write_register(self, reg, value):
        """
        Écrit dans un registre I2C
        
        Args:
            reg: Numéro du registre (0-15)
            value: Valeur à écrire (0-255)
            
        Returns:
            True si succès, False sinon
        """
        try:
            self.bus.write_byte_data(self.slave_addr, reg, value)
            time.sleep(0.001)  # Délai de 1ms après écriture
            return True
        except Exception as e:
            print(f"Erreur lors de l'écriture du registre {reg}: {e}")
            return False
        
    def get_car_state(self):
        """
        Récupère l'état de détection de voiture
        
        Returns:
            1 si une voiture est détectée, 0 sinon
        """
        return self.read_register(REG_CAR_STATE)
    
    def get_light_state(self):
        """
        Récupère l'état du capteur de lumière
        
        Returns:
            1 si il fait sombre, 0 sinon
        """
        return self.read_register(REG_LIGHT_STATE)
    
    def get_servo_angle(self):
        """
        Récupère l'angle actuel du servo
        
        Returns:
            Angle du servo (0-180 degrés)
        """
        return self.read_register(REG_SERVO_ANGLE)
    
    def get_led_state(self):
        """
        Récupère l'état des LEDs
        
        Returns:
            Byte avec les bits: bit0=RED, bit1=GREEN, bit2=WHITE
        """
        return self.read_register(REG_LED_STATE)
    
    def get_release_counter(self):
        """
        Récupère le compteur de release
        
        Returns:
            Valeur du compteur (0-40)
        """
        return self.read_register(REG_RELEASE_COUNTER)
    
    def get_system_status(self):
        """
        Récupère le status général du système

        Returns:
            0x01 si système OK, autre valeur sinon
        """
        return self.read_register(REG_SYSTEM_STATUS)

    def check_data_changed(self):
        """
        Vérifie si des données ont changé depuis la dernière lecture

        Returns:
            True si des données ont changé, False sinon
        """
        flag = self.read_register(REG_CHANGE_FLAG)
        if flag == 1:
            # Reset the flag after reading
            self.write_register(REG_CHANGE_FLAG, 0)
            return True
        return False
    
    def get_all_status(self, force=False):
        """
        Récupère tous les états

        Args:
            force: Si True, lit les données même si rien n'a changé

        Returns:
            Dict avec tous les états ou None en cas d'erreur
            Si rien n'a changé et force=False, retourne un dict avec changed=False
        """
        try:
            # Check if data has changed
            if not force and not self.check_data_changed():
                return {'changed': False}

            car_state = self.get_car_state()
            light_state = self.get_light_state()
            servo_angle = self.get_servo_angle()
            led_state = self.get_led_state()
            release_counter = self.get_release_counter()

            if None in [car_state, light_state, servo_angle, led_state, release_counter]:
                return None

            # Type narrowing: we know these are not None now
            assert led_state is not None

            return {
                'changed': True,
                'car_detected': bool(car_state),
                'is_dark': bool(light_state),
                'servo_angle': servo_angle,
                'led_red': bool(led_state & 0x01),
                'led_green': bool(led_state & 0x02),
                'led_white': bool(led_state & 0x04),
                'release_counter': release_counter
            }
        except Exception as e:
            print(f"Erreur lors de la lecture du status complet: {e}")
            return None
    
    def set_servo_angle(self, angle):
        """
        Définit manuellement l'angle du servo
        ATTENTION: Le servo restera à cet angle jusqu'à réactivation du mode automatique
        
        Args:
            angle: Angle désiré (0-180 degrés) ou 255 pour réactiver le mode automatique
        """
        if angle == 255:
            # 255 = réactiver le mode automatique
            return self.write_register(6, 255)
        elif not 0 <= angle <= 360:
            print("Angle invalide. Doit être entre 0 et 180, ou 255 pour mode automatique.")
            return False
        
        return self.write_register(6, angle)
    
    def close(self):
        """Ferme la connexion I2C"""
        self.bus.close()


def display_status(status):
    """Affiche le status de manière formatée"""
    if status is None:
        print("❌ Impossible de récupérer le status")
        return

    # Check if data has changed
    if not status.get('changed', True):
        print("⏸️  Aucun changement détecté")
        return

    print("\n" + "="*50)
    print("📊 STATUS DU SYSTÈME DE PARKING")
    print("="*50)
    print(f"🚗 Voiture détectée    : {'OUI ⚠️' if status['car_detected'] else 'NON ✓'}")
    print(f"🌙 Luminosité          : {'SOMBRE 🌑' if status['is_dark'] else 'CLAIR ☀️'}")
    print(f"🔧 Angle du servo      : {status['servo_angle']}°")
    print(f"💡 LED Rouge          : {'ON 🔴' if status['led_red'] else 'OFF'}")
    print(f"💡 LED Verte          : {'ON 🟢' if status['led_green'] else 'OFF'}")
    print(f"💡 LED Blanche        : {'ON ⚪' if status['led_white'] else 'OFF'}")
    print(f"⏱️  Compteur release   : {status['release_counter']}/100")
    print("="*50 + "\n")


def monitor_mode(master, interval=1.0, force=False):
    """Mode de monitoring continu

    Args:
        master: Instance ParkingMaster
        interval: Intervalle entre les lectures (secondes)
        force: Si True, affiche toujours même si rien n'a changé
    """
    print("🔄 Mode monitoring activé (Ctrl+C pour quitter)")
    if force:
        print(f"📡 Mode FORCE : Lecture toutes les {interval}s (même si pas de changement)")
    else:
        print(f"📡 Mode OPTIMISÉ : Affichage uniquement si changement détecté")
    print()

    try:
        while True:
            status = master.get_all_status(force=force)
            display_status(status)
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\n👋 Arrêt du monitoring")


def main():
    parser = argparse.ArgumentParser(description='Master I2C pour système de parking Arduino')
    parser.add_argument('--bus', type=int, default=I2C_BUS, help='Numéro du bus I2C')
    parser.add_argument('--addr', type=int, default=SLAVE_ADDRESS, help='Adresse I2C du slave (hex)')
    parser.add_argument('--monitor', action='store_true', help='Mode monitoring continu')
    parser.add_argument('--interval', type=float, default=1.0, help='Intervalle de monitoring (secondes)')
    parser.add_argument('--force', action='store_true', help='Force la lecture même si pas de changement')
    parser.add_argument('--servo', type=int, help='Définir l\'angle du servo (0-180) ou 255 pour mode auto')
    parser.add_argument('--reset', action='store_true', help='Réinitialiser le système')
    
    args = parser.parse_args()
    
    # Créer l'instance du master
    master = ParkingMaster(bus_num=args.bus, slave_addr=args.addr)
    
    try:
        if args.reset:
            print("🔄 Réinitialisation du système...")
            print("⚠️  La réinitialisation n'est pas implémentée côté slave")
            print("    Veuillez redémarrer l'Arduino manuellement")
        
        elif args.servo is not None:
            if args.servo == 255:
                print("🔄 Réactivation du mode automatique du servo...")
            else:
                print(f"🔧 Définition de l'angle du servo à {args.servo}°...")
                print("⚠️  Mode manuel activé : utilisez --servo 255 pour revenir en automatique")
            
            if master.set_servo_angle(args.servo):
                print("✓ Commande envoyée avec succès")
            else:
                print("❌ Échec de l'envoi de la commande")
        
        elif args.monitor:
            monitor_mode(master, args.interval, force=args.force)

        else:
            # Lecture unique du status (toujours en mode force)
            status = master.get_all_status(force=True)
            display_status(status)
    
    finally:
        master.close()


if __name__ == "__main__":
    main()
