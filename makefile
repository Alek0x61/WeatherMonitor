CC = gcc
CFLAGS = -D WIRING_PI -D WIRING_PI_I2C
LIBS = -lwiringPi -lsqlite3

SRCS = main.c display/SSD1306.c wiringPiService/wiringPiController.c repository/connection.c repository/weatherData.c tempHumiSensor/dht22.c

TARGET = main

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) -d

clean:
	rm -f $(TARGET)

.PHONY: run clean
