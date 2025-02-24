# Directories
DIR_Config=./lib/waveshare/Config
DIR_EPD=./lib/waveshare/e-Paper
DIR_FONTS=./lib/waveshare/Fonts
DIR_GUI=./lib/waveshare/GUI
DIR_Examples=./lib/waveshare/Examples
DIR_Main=./
DIR_BIN=./bin

# Define the e-Paper display type and sources
EPD=epd7in5V2
OBJ_C_EPD=$(DIR_EPD)/EPD_7in5_V2.c
OBJ_C_Examples=$(DIR_Examples)/EPD_7in5_V2_test.c

# Source files
OBJ_C=$(wildcard $(OBJ_C_EPD) $(DIR_GUI)/*.c ${OBJ_C_Examples} ${DIR_Examples}/ImageData2.c ${DIR_Examples}/ImageData.c ${DIR_FONTS}/*.c ${DIR_Main}/*.cpp ${DIR_Main}/*.c)
OBJ_O=$(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

# RPI Hardware-specific object files
RPI_DEV_C=$(wildcard $(DIR_BIN)/dev_hardware_SPI.o $(DIR_BIN)/RPI_gpiod.o $(DIR_BIN)/DEV_Config.o)

# Debug flag
DEBUG=-D DEBUG

# Define the GPIO library to use
USELIB_RPI=USE_LGPIO_LIB

# Libraries for Raspberry Pi hardware control
LIB_RPI=-Wl,--gc-sections
ifeq ($(USELIB_RPI), USE_BCM2835_LIB)
    LIB_RPI += -lbcm2835 -lm 
else ifeq ($(USELIB_RPI), USE_WIRINGPI_LIB)
    LIB_RPI += -lwiringPi -lm 
else ifeq ($(USELIB_RPI), USE_LGPIO_LIB)
    LIB_RPI += -llgpio -lm 
else ifeq ($(USELIB_RPI), USE_DEV_LIB)
    LIB_RPI += -lgpiod -lm 
endif
DEBUG_RPI=-D $(USELIB_RPI) -D RPI

# Compiler and flags
CC=g++
MSG=-g -O -ffunction-sections -fdata-sections -Wall
CFLAGS+=$(MSG) -D $(EPD) -std=c++17

# Target output executable
TARGET=piArtFrame

# Phony target for RPI and cleaning
.PHONY: RPI clean

# Main targets
all: RPI

RPI: RPI_DEV RPI_epd

# Link the object files to create the final executable
RPI_epd: ${OBJ_O}
	@echo $(@)
	$(CC) $(CFLAGS) -D RPI $(OBJ_O) $(RPI_DEV_C) -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_EPD) -o $(TARGET) $(LIB_RPI) $(DEBUG)

# Create bin directory if it doesn't exist
$(shell mkdir -p $(DIR_BIN))

# Compile C files from Examples
${DIR_BIN}/%.o: $(DIR_Examples)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_EPD) $(DEBUG)

# Compile C files from e-Paper directory
${DIR_BIN}/%.o: $(DIR_EPD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_Config) $(DEBUG)

# Compile C files from Fonts directory
${DIR_BIN}/%.o: $(DIR_FONTS)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(DEBUG)

# Compile C files from GUI directory
${DIR_BIN}/%.o: $(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_Config) $(DEBUG)

# Compile C and CPP files from Main directory
${DIR_BIN}/%.o: $(DIR_Main)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_Examples) -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_Main) -I $(DIR_EPD) $(DEBUG)

${DIR_BIN}/%.o: $(DIR_Main)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_Examples) -I $(DIR_Config) -I $(DIR_GUI) -I $(DIR_Main) -I $(DIR_EPD) $(DEBUG)

# Build the RPI-specific hardware files
RPI_DEV:
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c $(DIR_Config)/dev_hardware_SPI.c -o $(DIR_BIN)/dev_hardware_SPI.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c $(DIR_Config)/RPI_gpiod.c -o $(DIR_BIN)/RPI_gpiod.o $(LIB_RPI) $(DEBUG)
	$(CC) $(CFLAGS) $(DEBUG_RPI) -c $(DIR_Config)/DEV_Config.c -o $(DIR_BIN)/DEV_Config.o $(LIB_RPI) $(DEBUG)

# Clean up object files and the target executable
clean:
	rm -f $(DIR_BIN)/*.* 
	rm -f $(TARGET)
