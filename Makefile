INCLUDES=-I.
INCLUDES+= -IarduinoLibsAndCore/cores/arduino
INCLUDES+= -IarduinoLibsAndCore/libraries/Wire/src
INCLUDES+= -IarduinoLibsAndCore/libraries/Wire/src/utility
INCLUDES+= -IarduinoLibsAndCore/variants/standard
INCLUDES+= -IFreeRTOS-Kernel/include
INCLUDES+= -IFreeRTOS-Kernel/portable/GCC/ATMega328
INCLUDES+= -Idrivers

vpath %.cpp arduinoLibsAndCore/libraries/Wire/src
vpath %.cpp drivers
vpath %c arduinoLibsAndCore/libraries/Wire/src/utility
vpath %c arduinoLibsAndCore/cores/arduino
vpath %c FreeRTOS-Kernel/
vpath %c FreeRTOS-Kernel/portable/MemMang
vpath %c FreeRTOS-Kernel/portable/GCC/ATMega328/
vpath %.c drivers

BUILD_DIR=Build

CC=avr-gcc
CPP=avr-g++

MMCU=-mmcu=atmega328p

CFLAGS= -g -Os -w -std=gnu11 -ffunction-sections -fdata-sections \
        -MMD ${MMCU} -DF_CPU=16000000L \
        -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR

CPPFLAGS= -g -Os -w -std=gnu++11 -fpermissive -fno-exceptions \
          -ffunction-sections -fdata-sections \
          -Wno-error=narrowing -MMD -x c++ -CC \
          ${MMCU} -DF_CPU=16000000L \
          -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR

PROGRAM=ParkingRTOS

all: $(BUILD_DIR)/$(PROGRAM).elf $(BUILD_DIR)/$(PROGRAM).hex

# ------------------------
#      LINK (NO LTO)
# ------------------------
$(BUILD_DIR)/$(PROGRAM).elf: Build/timers.o Build/tasks.o Build/queue.o Build/list.o Build/croutine.o \
							Build/heap_1.o Build/port.o Build/Wire.o Build/twi.o Build/wiring_digital.o \
Build/ir.o Build/servo.o Build/lcd_grove.o Build/soft_i2c.o Build/main.o

	$(CPP) $(MMCU) -Wl,--gc-sections \
	       $^ -o $@

# ------------------------
#  compile .c files
# ------------------------
Build/%.o: %.c
	mkdir -p Build
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

# ------------------------
#  compile .cpp files
# ------------------------
Build/%.o: %.cpp
	mkdir -p Build
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $< -o $@

OBJCOPY=avr-objcopy

$(BUILD_DIR)/$(PROGRAM).hex: $(BUILD_DIR)/$(PROGRAM).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

PORT=/dev/ttyACM0

upload: $(BUILD_DIR)/$(PROGRAM).hex
	avrdude -F -V -c arduino -p atmega328p \
	        -P $(PORT) -b 115200 \
	        -U flash:w:$(BUILD_DIR)/$(PROGRAM).hex

clean:
	rm -rf Build
