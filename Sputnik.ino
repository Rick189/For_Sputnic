// заметки
// myswitch убираем у нас радио на Serial1
// SD убираем у нас оно на Serial2
// GPS на Serial3
// остальное вроде нормально

//Импортируем библиотеки
//#include "SD.h"
#include <Wire.h> 
#include <MS5611.h>
#include "MAX6675_Thermocouple.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <TinyGPS++.h>
//#include <RCSwitch.h>

//Ввод данных
//int loops = 0;
//Радио модуль
//RCSwitch mySwitch = RCSwitch();
//Акселерометр
MPU6050 mpu;
//Термопара на 1000
uint8_t thermoDO = 4;
uint8_t thermoCS = 5;
uint8_t thermoCLK = 6;
MAX6675_Thermocouple* thermocouple = NULL;
MS5611 ms5611;
double referencePressure;
TinyGPSPlus gps;
boolean gpsLedState = false;
long loops = 0;

boolean enableGPS = false;
boolean enableLogger = false;
boolean enableRadio = false;

void setup() {
   //pinMode(10, OUTPUT); 
   //pinMode(11, OUTPUT);
   //pinMode(12, OUTPUT);
   //pinMode(13, OUTPUT);
   //Объявляем пины термопары
   thermocouple = new MAX6675_Thermocouple(thermoDO, thermoCS, thermoCLK);
   Serial.begin(9600);
   Serial.println("[Kosmodesantnik] Program start");
   Serial.println("==============================");
   // led animation setup
   for(uint8_t i = 0; i < 3; i++) {
      pinMode(31 + i, OUTPUT);
   }
   // ms5611
   if(!ms5611.begin()) Serial.println("[Kosmodesantnik] MS5611 error!");
   // mpu
   Wire.begin();
   mpu.initialize();
   if(!mpu.testConnection()) Serial.println("[Kosmodesantnik] MPU6050 error!");
   // получаем начальное давление для правильных вычислений 0_0
   referencePressure = ms5611.readPressure();
   // setup logger
   if(enableLogger) {
      Serial2.begin(9600);
      Serial.println("Logger setup complete!");
   }
   // setup radio
   // radio led
   pinMode(35, OUTPUT);
   if(enableRadio) {
      pinMode(8, OUTPUT);
      pinMode(9, OUTPUT);
      Serial1.begin(9600);
      Serial.println("Radio setup complete!");
   }
   // setup gps
   // gps led
   pinMode(37, OUTPUT);
   if(enableGPS) {
      Serial3.begin(9600);
      Serial.println("GPS setup complete!");
   }
   Serial.println("[Kosmodesantnik] Setup complete");
}

void loop() {
  // led animation старт цикла
  for(uint8_t i = 0; i < 3; i++) {
     digitalWrite(31 + i, HIGH);
     delay(125);
  }
  // очищаем светодиод радио
  if(enableRadio) {
     digitalWrite(35, LOW);
  }
   
  Serial.println("[Kosmodesantnik] Read barometer sensor [temperature, pressure, absolute/relative altitude]");
  // Получение предворительных данных
  uint32_t rawTemp = ms5611.readRawTemperature();
  uint32_t rawPressure = ms5611.readRawPressure();
  // Получение настоящих данных
  double realTemperature = ms5611.readTemperature();
  long realPressure = ms5611.readPressure();
  // Подсчет и получение
  float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
  float celsius = thermocouple->readCelsius();
  
  Serial.print("[Kosmodesantnik] celsius: ");
  Serial.print(celsius); Serial.print("°");
  Serial.print(", rawTemp: ");
  Serial.print(rawTemp);
  Serial.print(", rawPressure: ");
  Serial.print(rawPressure);
  Serial.print(", realTemperature: ");
  Serial.print(realTemperature);
  Serial.print(", realPressure: ");
  Serial.print(realPressure);
  Serial.print(", absoluteAltitude: ");
  Serial.print(absoluteAltitude);
  Serial.print(", relativeAltitude: ");
  Serial.println(relativeAltitude);
   
  Serial.println("[Kosmodesantnik] Read IMU sensor [accelerometer x/y/z, gyroscope x/y/z]");
  
  // чтение сырых данных акселерометра/гироскопа
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  // преобразование в настоящие данные
  float axf, ayf, azf;
  float gxf, gyf, gzf;
  axf = ((float)ax / 32768.0F * 2.0F);
  ayf = ((float)ay / 32768.0F * 2.0F);
  azf = ((float)az / 32768.0F * 2.0F);
  gxf = ((float)gx / 32768.0F * 250.0F);
  gyf = ((float)gy / 32768.0F * 250.0F);
  gzf = ((float)gz / 32768.0F * 250.0F);
  
  Serial.print("[Kosmodesantnik] Accel: ");
  Serial.print(axf);
  Serial.print(", ");
  Serial.print(ayf);
  Serial.print(", ");
  Serial.println(azf);
  Serial.print("[Kosmodesantnik] Gyro: ");
  Serial.print(gxf);
  Serial.print(", ");
  Serial.print(gyf);
  Serial.print(", ");
  Serial.println(gzf);
   
  if(enableLogger) {
     Serial2.print("[Kosmodesantnik] T+"); Serial2.print(millis() / 1000); Serial2.println(" s,");
     Serial.println("Write data to logger...");
  }
   
  if(enableGPS) {
     Serial.print("[Kosmodesantnik] GPS: Sats, HDOP, lat, lon, age, date, altitude[m], course[°], speed[km/h]: ");
     Serial.print(gps.satellites.value()); Serial.print(", ");
     Serial.print(gps.hdop.hdop()); Serial.print(", ");
     Serial.print(gps.location.lat(), 5); Serial.print(", ");
     Serial.print(gps.location.lng(), 5); Serial.print(", ");
     Serial.print(gps.location.age()); Serial.print(", [");
     Serial.print(gps.date.day()); Serial.print(","); Serial.print(gps.date.month()); Serial.print(","); Serial.print(gps.date.year()); Serial.print("], [");
     Serial.print(gps.time.hour()); Serial.print(":"); Serial.print(gps.time.minute()); Serial.print(":"); Serial.print(gps.time.second()); Serial.print("], ");
     Serial.print(gps.altitude.meters()); Serial.print(", ");
     Serial.print(gps.course.deg()); Serial.print(", ");
     Serial.println(gps.speed.kmph());
    
     long gpsTimeout = 1000;
     unsigned long startTime = millis();
     do {
       while(Serial3.available()) {
         gps.encode(Serial3.read());
       }
       digitalWrite(37, gpsLedState ? HIGH : LOW);
       gpsLedState = !gpsLedState;
     } while(millis() - startTime < gpsTimeout);
  }
   
  if(enableRadio && loops % 20 == 0) {
     // кодировку пакета
     Serial1.print("[Kosmodesantnik] Packet: T+"); Serial1.print(millis() / 1000); Serial1.print(" s,");
     Serial.println("Transmit packet by radio...");
     digitalWrite(35, HIGH);
  }
  
  // led animation конец цикла
  for(uint8_t i = 0; i < 3; i++) {
     digitalWrite(31 + i, LOW);
     delay(125);
  }
  
  loops += 1;
  delay(1000);
  //const double celsius = thermocouple->readCelsius();
  //const double celsius_1 = thermocouple_1->readCelsius();
  
  //Включаем 2 реле
  //int value = mySwitch.getReceivedValue();
  //if (value == "St" or loops == 100){
  //  digitalWrite(10, HIGH);
  //  digitalWrite(12, HIGH);
  //
  //  Если 1 реле не работает, влючаем второе
  //  if (celsius < 500){
  //     digitalWrite(10, HIGH);
  //     if (digitalRead(10) == LOW){
  //        digitalWrite(11, HIGH);
  //     } else{
  //        digitalWrite(11, LOW);
  //     }
  //  }
  //  if (celsius_1 < 1000){
  //     digitalWrite(12, HIGH);
  //     if (digitalRead(12) == LOW){
  //        digitalWrite(13, HIGH);
  //     } else{
  //        digitalWrite(13, LOW);
  //     }
  //  }  
  //  loops = 99
  //}

  /*mySwitch.send("; R1:", digitalRead(10));
  mySwitch.send("; R2:", digitalRead(11));
  mySwitch.send("; R3:", digitalRead(12));
  mySwitch.send("; R4", digitalRead(13));
  mySwitch.send("; C:", celsius);
  mySwitch.send("; C_1:", celsius_1);*/
    
  //Сохранение на SD карту
  //Получение данных с акселерометра
  //Vector rawAccel = mpu.readRawAccel();
  //Vector rawGyro = mpu.readRawGyro();
  //Открываем файл
  //File dataFile = SD.open("LoggerCD.txt", FILE_WRITE);
  // если файл доступен, записываем в него
  //if (dataFile) {
    //Значение с реле
    /*dataFile.println("R1:"); 
    dataFile.print(digitalRead(10));
    dataFile.print("; R2:"); 
    dataFile.print(digitalRead(11));
    dataFile.print("; R3:"); 
    dataFile.print(digitalRead(12));
    dataFile.print("; R4:"); 
    dataFile.print(digitalRead(13));*/
    //Температура с термопар
    /*dataFile.print("; C:"); 
    dataFile.print(celsius);
    dataFile.print("; C_1:"); 
    dataFile.print(celsius_1);*/
    //Модуль датчиков
    //Акси
    /*dataFile.print("; AX:");
    dataFile.print(rawAccel.XAxis);
    dataFile.print("; AY");
    dataFile.print(rawAccel.YAxis);
    dataFile.print("; AZ:");
    dataFile.print(rawAccel.ZAxis);*/
    //Гиро
    /*dataFile.print("; GX");
    dataFile.print(rawGyro.XAxis);
    dataFile.print("; GY");
    dataFile.print(rawGyro.YAxis);
    dataFile.print("; GZ");
    dataFile.println(rawGyro.ZAxis);*/
    
    /*dataFile.print("; rT:"); 
    dataFile.print(rawTemp);
    dataFile.print("; rP:"); 
    dataFile.print(rawPressure);
    dataFile.print("; T:"); 
    dataFile.print(realTemperature);
    dataFile.print("; P:"); 
    dataFile.print(realPressure);
    dataFile.print("; A:"); 
    dataFile.print(absoluteAltitude);
    dataFile.close();*/
  //}
  //loops = loops + 1;
  //delay(1000);
}
