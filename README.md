# Système de Parking Connecté (Arduino + Raspberry Pi)

Ce projet implémente un système de gestion de parking intelligent utilisant un Arduino (comme esclave I2C) pour la gestion des capteurs/actionneurs et une Raspberry Pi (comme maître I2C) pour l'interface de contrôle Web.

## Prérequis Matériels

*   **Arduino Uno/Nano** (ou compatible ATMega328p)
*   **Raspberry Pi** (avec interface I2C activée)
*   **Composants électroniques** :
    *   Ultrasons (Liaison I2C simulée ou directe)
    *   Servo-moteur (Porte)
    *   LEDs (Rouge, Verte, Blanche)
    *   Capteur de luminosité (Photorésistance)
*   Câbles de connexion (Jumper wires)
    *   **IMPORTANT**: Relier les masses (GND) de l'Arduino et de la Raspberry Pi ensemble.
    *   Relier SDA et SCL pour la communication I2C (avec adaptation de niveau 3.3V/5V si nécessaire).

## Prérequis Logiciels

Assurez-vous d'avoir les outils suivants installés sur votre système (Linux/Raspberry Pi OS) :

### Système
```bash
sudo apt update
sudo apt install -y git build-essential python3 python3-pip
sudo apt install -y avr-libc gcc-avr avrdude
```

### Python
Les dépendances Python sont listées dans `web_interface/requirements.txt`.
```bash
pip3 install -r web_interface/requirements.txt
```

---

## Installation et Lancement

### 1. Partie Arduino (Esclave)

Le code C++ pour l'Arduino se trouve à la racine du projet. Il utilise FreeRTOS pour la gestion des tâches.

1.  Connectez l'Arduino au port USB de la Raspberry Pi/PC.
2.  Compilez et téléversez le code :

```bash
# Compiler le projet
make

# Téléverser sur l'Arduino (port par défaut /dev/ttyACM0)
make upload
```
*Note : Si votre Arduino est sur un autre port, modifiez la variable `PORT` dans le `Makefile` ou lancez `make upload PORT=/dev/ttyUSB0`.*

### 2. Partie Interface Web (Maître)

L'interface Web permet de visualiser l'état du parking et de contrôler la barrière.

1.  Assurez-vous que l'I2C est activé sur la Raspberry Pi (`sudo raspi-config`).
2.  Lancez le serveur Flask :

```bash
cd web_interface
python3 app.py
```

3.  Ouvrez votre navigateur et accédez à : `http://<IP_RASPBERRY>:5000`

---

## Outils de Debugging (CLI)

Un script Python en ligne de commande est disponible à la racine pour tester la communication I2C sans passer par l'interface web.

**Utilisation de `i2c_master.py` :**

```bash
# Afficher l'aide
python3 i2c_master.py --help

# Monitoring continu des capteurs
python3 i2c_master.py --monitor

# Ouvrir la barrière (Servo 90°)
python3 i2c_master.py --servo 90

# Remettre la barrière en mode automatique
python3 i2c_master.py --servo 255
```

## Structure du Projet

*   `main.cpp` : Point d'entrée du code Arduino (FreeRTOS tasks).
*   `drivers/` : Pilotes pour les périphériques Arduino.
*   `FreeRTOS-Kernel/` : Noyau du système temps réel.
*   `i2c_master.py` : Librairie Python maître pour communiquer avec l'Arduino.
*   `web_interface/` : Code source de l'interface Web (Flask + HTML/JS).