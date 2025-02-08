#include "SSD1306.h"
#include "fonts/16px.c"

#define OLED_ADDR                    0x3C
#define SSD1306_COMMAND              0x00
#define SSD1306_DATA                 0x40

#define OLED_WIDTH                   128
#define OLED_PAGE_NUMBER             8

#define SSD1306_DISPLAYOFF           0xAE  // Display off
#define SSD1306_SETDISPLAYCLOCKDIV   0xD5  // Set display clock divide ratio
#define SSD1306_CLOCKDIV             0x80  // Suggested ratio
#define SSD1306_SETMULTIPLEX         0xA8  // Set multiplex ratio
#define SSD1306_HEIGHT               0x3F  // Height of the display (64)
#define SSD1306_SETDISPLAYOFFSET     0xD3  // Set display offset
#define SSD1306_NOOFFSET             0x00  // No offset
#define SSD1306_SETSTARTLINE         0x40  // Set start line address
#define SSD1306_CHARGEPUMP           0x8D  // Charge pump setting
#define SSD1306_ENABLECHARGEPUMP     0x14  // Enable charge pump
#define SSD1306_SETMEMORYMODE        0x20  // Set memory addressing mode
#define SSD1306_HORIZONTALMODE       0x00  // Horizontal addressing mode
#define SSD1306_SETSEGMENTREMAP      0xA1  // Set segment remap
#define SSD1306_SETCOMSCAN           0xC8  // Set COM output scan direction
#define SSD1306_SETCOMHARDWARE       0xDA  // Set COM pins hardware configuration
#define SSD1306_ALTCOMCONFIG         0x12  // Alternative COM pins configuration
#define SSD1306_SETCONTRAST          0x81  // Set contrast control
#define SSD1306_MAXCONTRAST          0x7F  // Maximum contrast
#define SSD1306_SETPRECHARGE         0xD9  // Set pre-charge period
#define SSD1306_RECOMMENDEDPRECHARGE 0xF1  // Pre-charge period (Recommended)
#define SSD1306_SETVCOMH             0xDB  // Set VCOMH deselect level
#define SSD1306_VCOMHDLEVEL          0x40  // VCOMH deselect level
#define SSD1306_ENTIREDISPLAYON      0xA4  // Entire display ON
#define SSD1306_NORMALDISPLAY        0xA6  // Normal display
#define SSD1306_DISPLAYON            0xAF  // Display ON
#define SSD1306_PAGEADDR             0xB0
#define SSD1306_COLUMNADDR           0x21
#define SSD1306_SETDIM               0x81

int fd;

void sendCommand(unsigned char cmd) {
    writeI2CReg(fd, SSD1306_COMMAND, cmd);
}

void sendData(unsigned char data) {
    writeI2CReg(fd, SSD1306_DATA, data);
}

void initOLED() {
    sendCommand(SSD1306_DISPLAYOFF);
    sendCommand(SSD1306_SETDISPLAYCLOCKDIV);
    sendCommand(SSD1306_CLOCKDIV);
    sendCommand(SSD1306_SETMULTIPLEX);
    sendCommand(SSD1306_HEIGHT);
    sendCommand(SSD1306_SETDISPLAYOFFSET);
    sendCommand(SSD1306_NOOFFSET);
    sendCommand(SSD1306_SETSTARTLINE);
    sendCommand(SSD1306_CHARGEPUMP);
    sendCommand(SSD1306_ENABLECHARGEPUMP);
    sendCommand(SSD1306_SETMEMORYMODE);
    sendCommand(SSD1306_HORIZONTALMODE);
    sendCommand(SSD1306_SETSEGMENTREMAP);
    sendCommand(SSD1306_SETCOMSCAN);
    sendCommand(SSD1306_SETCOMHARDWARE);
    sendCommand(SSD1306_ALTCOMCONFIG);
    sendCommand(SSD1306_SETCONTRAST);
    sendCommand(SSD1306_MAXCONTRAST);
    sendCommand(SSD1306_SETPRECHARGE);
    sendCommand(SSD1306_RECOMMENDEDPRECHARGE);
    sendCommand(SSD1306_SETVCOMH);
    sendCommand(SSD1306_VCOMHDLEVEL);
    sendCommand(SSD1306_ENTIREDISPLAYON);
    sendCommand(SSD1306_NORMALDISPLAY);
}

void changePage(int page) {
    sendCommand(SSD1306_PAGEADDR + page);
    sendCommand(0x00);
    sendCommand(0x10);
}

void clearDisplay() {
    for (int page = 0; page < OLED_PAGE_NUMBER; page++) {
        sendCommand(SSD1306_PAGEADDR + page);
        sendCommand(0x00);
        sendCommand(0x10);
        for (int col = 0; col < OLED_WIDTH; col++) {
            sendData(0x00);
        }
    }
}

void displayOff() {
    sendCommand(SSD1306_DISPLAYOFF);
    clearDisplay();
}

void displayOn() {
    sendCommand(SSD1306_DISPLAYON);
}

char setupSSD1306() {
    if (setupGPIO() == ERROR) {
        return ERROR;
    }

    I2CSetupResult i2cSetupRes = setupI2C(OLED_ADDR);
    if (i2cSetupRes.isSuccess == ERROR) {
        return ERROR;
    }
    fd = i2cSetupRes.fileDescriptior;

    initOLED();

    return SUCCESS;
}


void printOverflow(const char *str) {
    for (int fontPartIndex = 0; fontPartIndex < fontPart; fontPartIndex++) {
        int charIndex = 0;

        int horizontalLength = 0;
        int page = fontPartIndex;

        changePage(page);
        
        while (str[charIndex] != '\0') { 
            int asciiCharValue = str[charIndex];

            int offsetIndex = asciiCharValue - 31;
            
            int width = fontWidth[offsetIndex];
            int startOffset = (fontOffset[offsetIndex]);
            int partLength = width / fontPart;
            int partStartOffset = startOffset + (partLength * fontPartIndex);

            horizontalLength = horizontalLength + partLength;

            if (horizontalLength > OLED_WIDTH) {
                page += fontPart;
                horizontalLength = partLength;
                changePage(page);
            }

            for (int charsInPart = partStartOffset; charsInPart < partStartOffset + partLength; charsInPart++) {
                sendData(font[charsInPart]);
            }
            for (int i = 0; i < fontPart; i++) {
                horizontalLength++;
                sendData(0x00);
            }

            charIndex++;
        }
    }
}

void changeVerticalPosition(int x) {
    sendCommand(0x00 | (x & 0x0F)); 
    sendCommand(0x10 | ((x >> 4) & 0x0F)); 
}

void printOnCoordinates(const char *str, int x, int y) {
    int pageNum = y / 8;

    for (int fontPartIndex = 0; fontPartIndex < fontPart; fontPartIndex++) {
        int charIndex = 0;

        int horizontalLength = 0;
        int page = pageNum + fontPartIndex;

        changePage(page);

        if (x > 0) {
            changeVerticalPosition(x);
        }


        while (str[charIndex] != '\0') { 
            int asciiCharValue = str[charIndex];

            int offsetIndex = asciiCharValue - 31;
            
            int width = fontWidth[offsetIndex];
            int startOffset = (fontOffset[offsetIndex]);
            int partLength = width / fontPart;
            int partStartOffset = startOffset + (partLength * fontPartIndex);

            horizontalLength = horizontalLength + partLength;

            if (horizontalLength > OLED_WIDTH) {
                break;
            }

            for (int charsInPart = partStartOffset; charsInPart < partStartOffset + partLength; charsInPart++) {
                sendData(font[charsInPart]);
            }
            for (int i = 0; i < fontPart; i++) {
                horizontalLength++;
                sendData(0x00);
            }

            charIndex++;
        }
    }
}

void print(const char *str) {
    printOnCoordinates(str, 0, 0);
}