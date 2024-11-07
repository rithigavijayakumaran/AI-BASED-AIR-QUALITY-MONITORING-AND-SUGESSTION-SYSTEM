// run code on platformIO IDE

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD (I2C address 0x27 for a 16x2 LCD)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins for MQ sensors
const int mq7Pin = A0;     // MQ-7 CO sensor
const int mq135Pin = A1;   // MQ-135 Air Quality sensor
const int mq136Pin = A2;   // MQ-136 H2S sensor
const int methanePin = A3; // Methane sensor (MQ-4)
const int ozonePin = A4;   // Ozone sensor (MQ-131)

// Use hardware Serial2 for Nova PM sensor
#define pmsSerial Serial2

// Declare function prototype
bool readPMData(int &pm1_0, int &pm2_5, int &pm10);

void setup() {
  Serial.begin(115200);  // For Serial Monitor
  pmsSerial.begin(9600); // PMS sensor baud rate

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Start message
  lcd.setCursor(0, 0);
  lcd.print("AQI Prediction");
  delay(2000);  // Display initial message for 2 seconds
  lcd.clear();
}

void loop() {
  // Reading analog values from MQ sensors
  int mq7Value = analogRead(mq7Pin);       // CO level
  int mq135Value = analogRead(mq135Pin);   // Air quality
  int mq136Value = analogRead(mq136Pin);   // H2S level
  int methaneValue = analogRead(methanePin); // Methane (CH4) level

  // Read values from the Nova PM sensor (PMS5003)
  int pm1_0, pm2_5, pm10;
  if (readPMData(pm1_0, pm2_5, pm10)) {
    Serial.print("PM 1.0: ");
    Serial.println(pm1_0);
    Serial.print("PM 2.5: ");
    Serial.println(pm2_5);
    Serial.print("PM 10: ");
    Serial.println(pm10);
  }

  // Display readings on Serial Monitor
  Serial.println("------------ Sensor Values ------------");
  Serial.print("MQ-7 CO: "); Serial.println(mq7Value);
  Serial.print("MQ-135 : "); Serial.println(mq135Value);
  Serial.print("MQ-136 H2S: "); Serial.println(mq136Value);
  Serial.print("Methane (CH4): "); Serial.println(methaneValue);
  
  // Send data format for the model: "DATA:<mq7Value>,<mq135Value>,<mq136Value>,<pm2_5>,<pm10>,<methaneValue>;"
  Serial.print("DATA:");
  Serial.print(mq7Value); Serial.print(",");      // MQ-7 CO
  Serial.print(mq135Value); Serial.print(",");    // MQ-135 Air Quality
  Serial.print(mq136Value); Serial.print(",");    // MQ-136 H2S
  Serial.print(pm2_5); Serial.print(",");          // PM2.5
  Serial.print(pm10); Serial.print(",");           // PM10
  Serial.print(methaneValue); Serial.println(";"); // Methane (CH4)

  delay(2000);  // Delay before updating

  // Read prediction from Python script
  if (Serial.available() > 0) {
    String prediction = Serial.readStringUntil('\n');
    int separator = prediction.indexOf(',');
    String aqiValue = prediction.substring(0, separator);
    String aqiCategory = prediction.substring(separator + 1);

    // Display prediction on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AQI: ");
    lcd.print(aqiValue);
    lcd.setCursor(0, 1);
    lcd.print("Category: ");
    lcd.print(aqiCategory);
  }
}

// Function to read data from Nova PM sensor
bool readPMData(int &pm1_0, int &pm2_5, int &pm10) {
  if (pmsSerial.available() >= 32) {
    uint8_t buffer[32];
    pmsSerial.readBytes(buffer, 32);

    if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
      pm1_0 = (buffer[10] << 8) | buffer[11];  // PM 1.0
      pm2_5 = (buffer[12] << 8) | buffer[13];  // PM 2.5
      pm10  = (buffer[14] << 8) | buffer[15];  // PM 10
      return true;
    }
  }


  return false;
}
