//// ThingsSpeak+Arduino+DS18B20
/// Internetinis logeris temperaturai registruoti realiu laiku
// sukurta 2017 01
// PauliusPlus
/// padarytas branchas
/// kazkas pakeista.....




     #include "ThingSpeak.h"
    unsigned long myChannelNumber = 195874;
    const char * myWriteAPIKey = "QPW308ZM2HSYGTK9";

    #include <SPI.h>
    #include <Ethernet.h>
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //DE:AD:BE:EF:FE:ED, Internal static IP is 192.168.0.52
    EthernetClient client;

    #include <OneWire.h>
    #include <DallasTemperature.h>
    OneWire oneWire(14);
    DallasTemperature OneWire_Sensors(&oneWire);

    #include "max6675.h"

    int thermoDO = 7;
    int thermoCS = 8;
    int thermoCLK = 9;

    MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);


    float fltEnviromentTemp = 0.1, fltPlantTemp = 0.1, fltReservoirTemp = 0.1, highTemp = 0.1;
    int i, intPlantHumidity = 3, intSensorAlarm = 5, intRainAlarm = 7;
    boolean blnRainAlarmNotified = false;

    unsigned long previousMillis = 0;        // will store last time LED was updated

    // constants won't change :
    const long interval = 16000;           // interval at which to blink (milliseconds)

    // use this value to determine the size of the readings array.
    const int numReadings = 16;

    float readingsTemp[numReadings];      // the readings from the analog input
    int readIndexTemp = 0;              // the index of the current reading
    float totalTemp = 0.0;                  // the running total
    float averageTemp = 0.0;                // the average

    float readingsLight[numReadings];      // the readings from the analog input
    int readIndexLight = 0;              // the index of the current reading
    int totalLight = 0;                  // the running total
    int averageLight = 0;                // the average

/////-------------------------------------------- SETUP -----------------------------------------

    void setup() {
    Serial.begin(9600);

    //pinMode(RAIN_SENSOR_DIGITAL, INPUT);

    Serial.println(F("v." __DATE__ ", " __TIME__ ""));

    Serial.print(F("Starting network"));
    Ethernet.begin(mac);

    Serial.print(F(", ThingSpeak"));
    ThingSpeak.begin(client);

    Serial.println(F(" and sensors."));
    OneWire_Sensors.begin();
  //  Plant_dht.begin();


  // initialize all the readings in the array to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readingsTemp[thisReading] = 0;
    readingsLight[thisReading] = 0;
  }

    previousMillis = millis();

    }////setup







////++++++++++++++++++++++++++++++++++  LOOP  +++++++++++++++++++++++++++++++++++++++++++++++

    void loop() {

    unsigned long currentMillis = millis();

    //Serial.println(F("Reading sensors"));

    delay(1000); //Wait 1 second

/*
    // Rain sensor
    if (!(digitalRead(RAIN_SENSOR_DIGITAL))) {
    Serial.println(F("WARNING: Rain detected, ALARM activated"));
    intRainAlarm = 1;
    blnRainAlarmNotified = true;
    } else {
    intRainAlarm = 0;
    blnRainAlarmNotified = false;
    }
*/

  totalTemp -= readingsTemp[readIndexTemp];

  // read from the sensor:
  highTemp = thermocouple.readCelsius();
  Serial.print(F("H_Temp = "));
  Serial.print(highTemp);
  readingsTemp[readIndexTemp] = highTemp;

  // add the reading to the total:
  totalTemp += readingsTemp[readIndexTemp];

  // advance to the next position in the array:
  readIndexTemp++;

  // if we're at the end of the array...wrap around to the beginning:
  if (readIndexTemp >= numReadings) {
    readIndexTemp = 0;
  }

  // calculate the average:
  averageTemp = totalTemp / numReadings;
  Serial.print(F("  AVG_Temp = "));
  Serial.print(averageTemp, 2);


    float pinVoltage = analogRead(A1);
    Serial.print("  Light: ");
    Serial.println(pinVoltage);


 if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    //Onewire sensors
    OneWire_Sensors.requestTemperatures();
    fltEnviromentTemp = OneWire_Sensors.getTempCByIndex(0);
    fltPlantTemp = OneWire_Sensors.getTempCByIndex(1);
    fltReservoirTemp = OneWire_Sensors.getTempCByIndex(2);

    //DHT11 sensor
  //  intPlantHumidity = Plant_dht.readHumidity();

    Serial.print(F("Temp1 = "));
    Serial.print(fltEnviromentTemp);
    Serial.println(F("*C"));
    Serial.print(F("Temp2 = "));
    Serial.print(fltPlantTemp);
    Serial.println(F("*C"));
    Serial.print(F("Temp3 = "));
    Serial.print(fltReservoirTemp);
    Serial.println(F("*C"));



/*
    if ((fltEnviromentTemp == -127.00) || (fltPlantTemp == -127.00) || (fltReservoirTemp == -127.00) || isnan(intPlantHumidity) || (intPlantHumidity == 0))
    {
    Serial.println(F("WARNING: Sensor problem, ALARM activated"));
    intSensorAlarm = 1;
    } else {
    Serial.println(F("Sensors are OK"));
    intSensorAlarm = 0;
    }

    */

    ThingSpeak.setField(1, fltEnviromentTemp);
    ThingSpeak.setField(2, fltPlantTemp);
    ThingSpeak.setField(3, fltReservoirTemp);
    ThingSpeak.setField(4, fltReservoirTemp);
    ThingSpeak.setField(5, averageTemp);
    ThingSpeak.setField(6, pinVoltage);


    Serial.println(F("Updating ThingSpeak"));

    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // ThingSpeak only accept updates every 15 seconds

    Serial.println(F("DONE"));

 }/////if

   // Serial.println(F("Sleeping for 15 minutes"));
//delay(5000); //Wait 20 seconds

/*
    for (i = 0; i < 45; i++) { //15 minutes = 45 * 20,000 = 900,000
    delay(20000); //Wait 20 seconds
    Serial.println(F("Checking for rain"));



    if (!(digitalRead(RAIN_SENSOR_DIGITAL)))
    {
    Serial.println(F("WARNING: Rain detected, ALARM activated"));

    if (blnRainAlarmNotified == false)
    {
    Serial.println(F("Updating ThingSpeak"));
    ThingSpeak.writeField(myChannelNumber, 5, 1, myWriteAPIKey);
    Serial.println(F("Finished updating"));

    blnRainAlarmNotified = true;

    } else {
    Serial.println(F("Already updated ThingSpeak"));
    }

*/


    //}
   // }

    } ///loop


////======================================================================================
