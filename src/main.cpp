/*
 * Testing STM32F405, Display, 6 DOF, and GPS
 */

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GPS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ECHO_GPS false
Adafruit_GPS gps(&Wire);

uint32_t timer = millis();

void setup()
{
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();

  if (!gps.begin(0x10)) {
    display.write("GPS begin failure");
    display.display();
    for(;;);
  }

  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  gps.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  gps.println(PMTK_Q_RELEASE);
}

void DisplayGpsInfo()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(gps.latitude, 4);
  display.print(' ');
  display.print(gps.lat);
  display.print(',');
  display.print(gps.longitude, 4);
  display.print(' ');
  display.println(gps.lon);
  display.display();
}

void loop()
{
  // read data from the GPS in the 'main loop'
  char c = gps.read();
  // if you want to debug, this is a good time to do it!
  if (ECHO_GPS)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (gps.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    // Serial.println(gps.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!gps.parse(gps.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer

    DisplayGpsInfo();

    Serial.print("\nTime: ");
    if (gps.hour < 10) { Serial.print('0'); }
    Serial.print(gps.hour, DEC); Serial.print(':');
    if (gps.minute < 10) { Serial.print('0'); }
    Serial.print(gps.minute, DEC); Serial.print(':');
    if (gps.seconds < 10) { Serial.print('0'); }
    Serial.print(gps.seconds, DEC); Serial.print('.');
    if (gps.milliseconds < 10) {
      Serial.print("00");
    } else if (gps.milliseconds > 9 && gps.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(gps.milliseconds);
    Serial.print("Date: ");
    Serial.print(gps.day, DEC); Serial.print('/');
    Serial.print(gps.month, DEC); Serial.print("/20");
    Serial.println(gps.year, DEC);
    Serial.print("Fix: "); Serial.print((int)gps.fix);
    Serial.print(" quality: "); Serial.println((int)gps.fixquality);
    if (gps.fix) {
      Serial.print("Location: ");
      Serial.print(gps.latitude, 4); Serial.print(gps.lat);
      Serial.print(", ");
      Serial.print(gps.longitude, 4); Serial.println(gps.lon);
      Serial.print("Speed (knots): "); Serial.println(gps.speed);
      Serial.print("Angle: "); Serial.println(gps.angle);
      Serial.print("Altitude: "); Serial.println(gps.altitude);
      Serial.print("Satellites: "); Serial.println((int)gps.satellites);
    }
  }
}
