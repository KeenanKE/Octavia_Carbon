#include <SPI.h>                     // For SPI communication with ADS1118
#include <Wire.h>                    // For I2C communication with OLED
#include <Adafruit_GFX.h>           // Adafruit graphics library
#include <Adafruit_SSD1306.h>       // OLED driver library

// ----- OLED Display Configuration -----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1            // No reset pin used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----- SPI Pins for ADS1118 (connected to ESP32 VSPI bus) -----
#define CS_PIN     5                // Chip select for ADS1118
#define SCLK_PIN  18                // SPI clock
#define MISO_PIN  19                // Master In Slave Out
#define MOSI_PIN  23                // Master Out Slave In

// ----- RTD Selection Pin -----
#define RTD_SELECT_PIN 4            // GPIO to detect selected RTD type (LOW = Pt100, HIGH = Pt1000)

// ----- Constants -----
const float VREF = 5.0;             // Reference voltage of ADS1118 (assuming it's powered from 5V)
const float ADC_RESOLUTION = 32768.0; // 16-bit signed ADC full-scale value
const float I_EXCITATION = 0.00025;   // 250 µA excitation current set by LM134

void setup() {
  // ----- Configure pin modes -----
  pinMode(CS_PIN, OUTPUT);                // Chip Select for SPI
  pinMode(RTD_SELECT_PIN, INPUT_PULLUP); // Read RTD selection switch
  
  // ----- Initialize SPI -----
  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  // ----- Initialize Serial Monitor -----
  Serial.begin(115200);

  // ----- Initialize OLED Display -----
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 OLED display not found"));
    while (true); // Stop execution if display is not found
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

// ----- Read a single conversion result from ADS1118 -----
uint16_t readADS1118() {
  // Configuration: single-shot, AIN0, 4.096V range, 128 SPS, default settings
  uint16_t config = 0x8583;

  digitalWrite(CS_PIN, LOW);          // Start SPI transaction
  SPI.transfer16(config);             // Send config (starts conversion)
  delay(10);                          // Wait for conversion to complete
  uint16_t result = SPI.transfer16(config); // Read ADC result
  digitalWrite(CS_PIN, HIGH);         // End SPI transaction

  return result;
}

// ----- Convert voltage to resistance using Ohm's Law -----
float resistanceFromVoltage(float v) {
  return v / I_EXCITATION; // R = V / I
}

// ----- Convert resistance to temperature (Pt100 or Pt1000) -----
float temperatureFromResistance(float R, bool isPt100) {
  float R0 = isPt100 ? 100.0 : 1000.0;   // Nominal resistance at 0°C
  float alpha = 0.00385;                 // Temperature coefficient for RTDs
  return (R - R0) / (R0 * alpha);        // Inverse of: R = R0 * (1 + αT)
}

void loop() {
  // ----- Determine which RTD is selected -----
  bool isPt100 = digitalRead(RTD_SELECT_PIN) == LOW;

  // ----- Read raw ADC data -----
  uint16_t raw = readADS1118();
  int16_t signed_raw = (int16_t)raw;     // Convert to signed 16-bit integer
  float voltage = (signed_raw / ADC_RESOLUTION) * VREF; // Convert ADC code to voltage

  // ----- Calculate resistance and temperature -----
  float resistance = resistanceFromVoltage(voltage);
  float temperature = temperatureFromResistance(resistance, isPt100);

  // ----- Display results on OLED -----
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("RTD Type: ");
  display.println(isPt100 ? "Pt100" : "Pt1000");

  display.print("Voltage: ");
  display.print(voltage, 4);
  display.println(" V");

  display.print("Resistance: ");
  display.print(resistance, 1);
  display.println(" Ohms");

  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.display();

  // ----- Delay before next reading -----
  delay(1000);
}
