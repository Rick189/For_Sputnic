#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(10);
DallasTemperature temperatureSensors(&oneWire);
DeviceAddress temperatureSensorsAddress;
uint8_t ds18b20_0[8] = { 0x28, 0x49, 0x6B, 0x61, 0x0B, 0x00, 0x00, 0x42 };
uint8_t ds18b20_1[8] = { 0x28, 0x69, 0x56, 0x3F, 0x0D, 0x00, 0x00, 0x80 };
uint8_t temperatureSensorsDeviceCount = 0;
float temp0 = 0.0F, temp1 = 0.0F;

void setup() {
  Serial.begin(9600);
  Serial.println("Stratosputnik");
  Serial.println("=============");
  
  Serial.print("Temperature sensors - ");
  Serial.println(enableTemperatureSensors ? "ON" : "OFF");
  // датчики температуры  
  if(enableTemperatureSensors) {
    // получаем адрес и температуру со всех датчиков температуры
    temperatureSensors.begin();
    temperatureSensorsDeviceCount = temperatureSensors.getDeviceCount();
    Serial.print("Founded DS18B20's: ");
    Serial.println(temperatureSensorsDeviceCount);
    for(uint8_t i = 0; i < temperatureSensorsDeviceCount; i++) {
      Serial.print("DS18B20 #");
      Serial.print(i);
      Serial.print(" - ");
      temperatureSensors.getAddress(temperatureSensorsAddress, i);
      for(uint8_t j = 0; j < 8; j++) {
        if(temperatureSensorsAddress[j] < 0x10) Serial.print("0");
        Serial.print(temperatureSensorsAddress[j], HEX);
        if(j < 7) Serial.print(":");
      }
      Serial.println();
    }
  }
}

void loop() {
  // послать запрос на все датчики температуры
  temperatureSensors.requestTemperatures();
  
  // получаем температуру со всех датчиков температуры
  temp0 = temperatureSensors.getTempC(ds18b20_0);
  temp1 = temperatureSensors.getTempC(ds18b20_1);
  Serial.print("[DS18B20] Temperature [Outer sensor, Inner battery sensor]: ");
  Serial.print(temp0);
  Serial.print("°C, ");
  Serial.print(temp1);
  Serial.println("°C");
  
  delay(1000);
}
