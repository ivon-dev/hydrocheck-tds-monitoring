#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns and 2 rows

#define TdsSensorPin A1
#define VREF 5.0      // Analog reference voltage (Volt) of the ADC
#define SCOUNT 30     // Number of sample points

int analogBuffer[SCOUNT];    // Store the analog values read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;

float averageVoltage = 0, tdsValue = 0, temperature = 25;

void setup() {
  Serial.begin(9600);
  pinMode(TdsSensorPin, INPUT);
  lcd.init(); // Initialize the LCD
  lcd.backlight();

  // Print the monitoring start header once
  Serial.println("-- MONITORING START --");
  Serial.println("---------------------------");
}

void loop() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();

    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
                - 255.86 * compensationVoltage * compensationVoltage
                + 857.39 * compensationVoltage) * 0.5;

    // Serial Monitor Output
    Serial.print("Raw Voltage: ");
    Serial.print(averageVoltage, 2);
    Serial.println(" V");

    Serial.print("Compensated Voltage: ");
    Serial.print(compensationVoltage, 2);
    Serial.println(" V");

    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" °C");

    Serial.print("TDS Value: ");
    Serial.print(tdsValue, 0);
    Serial.println(" ppm");
    Serial.println("---------------------------");

    // LCD Display
    char lcdLine1[17]; // 16 characters + null terminator
    char lcdLine2[17];

    // Format TDS value with leading zeros (4 digits)
    sprintf(lcdLine1, "TDS: %04d PPM", (int)tdsValue);

    // Format temperature with leading zeros (2 digits)
    sprintf(lcdLine2, "Temp: %02d C", (int)temperature);

    lcd.setCursor(0, 0);
    lcd.print(lcdLine1);
    lcd.setCursor(0, 1);
    lcd.print(lcdLine2);
  }
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];

  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;

  return bTemp;
}
