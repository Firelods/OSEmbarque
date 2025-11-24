PROGRAM=MyBlinkTest
PORT=/dev/ttyACM0

BUILD_DIR=Build
MCU=atmega328p
F_CPU=16000000L

CPP=avr-g++
CC=avr-gcc
OBJCOPY=avr-objcopy

INCLUDES = -I.

# -- Sources dans la racine --
SRCS_C   := $(filter %.c, $(wildcard *.c))
SRCS_CPP := $(filter %.cpp, $(wildcard *.cpp))

OBJS_C   := $(patsubst %.c,   $(BUILD_DIR)/%.o, $(SRCS_C))
OBJS_CPP := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS_CPP))

OBJS = $(OBJS_C) $(OBJS_CPP)

CFLAGS = -g -Os -std=gnu11 -ffunction-sections -fdata-sections -flto \
         -mmcu=$(MCU) -DF_CPU=$(F_CPU)

CPPFLAGS = -g -w -std=gnu++11 -fpermissive -fno-exceptions \
           -ffunction-sections -fdata-sections -fno-threadsafe-statics \
           -MMD -flto -x c++ -CC -mmcu=$(MCU) \
           -DF_CPU=$(F_CPU) -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR \
           -I.

LDFLAGS = -w -Os -g -flto -fuse-linker-plugin -Wl,--gc-sections -mmcu=$(MCU)


all: $(BUILD_DIR)/$(PROGRAM).elf $(BUILD_DIR)/$(PROGRAM).hex

# ------ Compilation C ------
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

# ------ Compilation C++ ------
$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $< -o $@

# ------ Link ------
$(BUILD_DIR)/$(PROGRAM).elf: $(OBJS)
	$(CPP) $(LDFLAGS) $(OBJS) -o $@

# ------ HEX ------
$(BUILD_DIR)/$(PROGRAM).hex: $(BUILD_DIR)/$(PROGRAM).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# ------ Upload ------
upload: $(BUILD_DIR)/$(PROGRAM).hex
	avrdude -F -V -c arduino -p ATMEGA328P -P ${PORT} -b 115200 \
	        -U flash:w:$(BUILD_DIR)/$(PROGRAM).hex

clean:
	rm -rf $(BUILD_DIR)
