#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ADS1118 SPI
#define CS 5    // D5
#define V_REF 3.3  // ADS1118 reference voltage
#define GAIN 51.0  // INA333 gain
#define RTD_CURRENT 0.001  // 1 mA from LM134

// RTD constants
#define ALPHA 0.00385  // 3850 ppm/°C

void setup() {
  Serial.begin(115200);
  SPI.begin(); // SCK=18, MISO=19, MOSI=23, CS=5
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found"));
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  delay(100);
}

uint16_t readADS1118() {
  uint16_t config = 0b1000001000000011; // AIN0-AIN1, ±4.096V, single-shot
  byte configMSB = config >> 8;
  byte configLSB = config & 0xFF;

  digitalWrite(CS, LOW);
  SPI.transfer(configMSB);
  SPI.transfer(configLSB);
  digitalWrite(CS, HIGH);

  delay(10); // Wait for conversion

  digitalWrite(CS, LOW);
  byte resMSB = SPI.transfer(0x00);
  byte resLSB = SPI.transfer(0x00);
  digitalWrite(CS, HIGH);

  return ((resMSB << 8) | resLSB);
}

void loop() {
  int16_t raw = readADS1118();
  float voltage = (raw * V_REF) / 32768.0;
  float rtd_voltage = voltage / GAIN;
  float rtd_resistance = rtd_voltage / RTD_CURRENT;

  // Determine RTD type from resistance range
  String rtdType;
  float tempC;
  if (rtd_resistance < 200.0) {
    rtdType = "Pt100";
    tempC = (rtd_resistance - 100.0) / (ALPHA * 100.0);
  } else {
    rtdType = "Pt1000";
    tempC = (rtd_resistance - 1000.0) / (ALPHA * 1000.0);
  }

  // Output to Serial
  Serial.print("RTD: ");
  Serial.print(rtdType);
  Serial.print(" | Resistance: ");
  Serial.print(rtd_resistance, 2);
  Serial.print(" Ohms | Temp: ");
  Serial.print(tempC, 2);
  Serial.println(" C");

  // Output to OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("RTD Type: "); display.println(rtdType);
  display.print("Resistance: "); display.print(rtd_resistance, 1); display.println(" Ohm");
  display.print("Temp: "); display.print(tempC, 1); display.println(" C");
  display.display();

  delay(1000);
}
