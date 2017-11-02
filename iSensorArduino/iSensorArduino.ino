/* This demo shows how to display the CCS811 readings on an Adafruit I2C OLED. 
 * (We used a Feather + OLED FeatherWing)
 */
 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include <Adafruit_SSD1306.h>
#include "Adafruit_CCS811.h"

#include "Adafruit_HTU21DF.h"
#include <BH1750.h>

Adafruit_CCS811 ccs;
Adafruit_SSD1306 display = Adafruit_SSD1306();

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

BH1750 lightMeter;

const uint8_t vref_pin = A0;
const uint8_t uv_pin = A1;
const uint8_t noise_pin = A2;

void setup() {  
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }

  lightMeter.begin(BH1750_ONE_TIME_HIGH_RES_MODE);

  pinMode(vref_pin, INPUT);
  pinMode(uv_pin, INPUT);
  pinMode(noise_pin, INPUT);
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  //display.clearDisplay();
  //display.display();
  //delay(500);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);

  analogReference(EXTERNAL);
  for(uint8_t i = 0; i < 3; i++) {
    analogRead(A0);
  }

  delay(3000);
}


void loop() {
  ccs.SWReset();
  
  if(ccs.available()) {
    display.clearDisplay();
    display.setCursor(0,0);
    
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
      uint16_t eCO2 = ccs.geteCO2();
      display.print("eCO2: ");
      display.print(eCO2);
      display.print("ppm\n");

      uint16_t TVOC = ccs.getTVOC();
      display.print("TVOC: ");
      display.print(TVOC);
      display.print("ppb\n");

      float temperature = htu.readTemperature();
      display.print("Temp:");
      display.print(temperature, 1);
      display.print("C");

      float humidity = htu.readHumidity();
      display.print(" Humi:");
      display.print(humidity, 1);
      display.print("%\n");

      uint16_t light = lightMeter.readLightLevel();
      display.print("Light: ");
      display.print(light);
      display.print("lux");
      display.display();
      delay(5000);

      display.clearDisplay();
      display.setCursor(0, 0);
      uint16_t vref_adc = averageAnalogRead(vref_pin);
      float vref = vref_adc * 3.3 / 1024;
      display.print("Voltage:");
      display.print(vref, 1);
      display.print("V\n");
      
      float uv_adc = averageAnalogRead(uv_pin);
      float uv_voltage = vref * uv_adc / 1024;
      float uv_intensity = mapfloat(uv_voltage, 0.99, 2.80, 0.0, 15.0);
      display.print("UV:");
      display.print(uv_intensity);
      display.print("mW/cm^2\n");      
      
      uint16_t noise_adc = averageAnalogRead(noise_pin);
      display.print("Noise:");
      display.print(noise_adc);
      display.print("ADC\n");

      display.display();
      delay(5000); 
    } else {
      Serial.println("ERROR!");
      while(1);
    }
  }
}

//Takes an average of readings on a given pin
//Returns the average
uint16_t averageAnalogRead(uint8_t pinToRead)
{
  uint8_t number_of_readings = 4;
  uint16_t accumulate_reading = 0; 

  for(uint8_t x = 0 ; x < number_of_readings ; x++) {
    accumulate_reading += analogRead(pinToRead);
    Serial.println(accumulate_reading);
  }
  accumulate_reading /= number_of_readings;
  Serial.print("ADC: ");
  Serial.println(accumulate_reading);

  return accumulate_reading;  
}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
