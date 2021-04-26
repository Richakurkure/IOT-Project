void(* resetFunc) (void) = 0; // Restarting Reference point for ESP 32 if Restart required due to unable to Connect to MQTT or WIfi

#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <EEPROM.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#define DHTPIN 5     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define WIFI_SSID "Galaxy" //your WiFi SSID for which yout NodeMCU connects
#define WIFI_PASSWORD "richi1993"//Password of your wifi network 


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME  "r_kurkur"
#define AIO_KEY       "aio_mOAw77cctLNLHismg7dgSrQpF648"


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/


// Setup a feed called 'onoff' for subscribing to changes.

// Subscribe allows the ESP 32 to download data from the Adafruit IO Feeds on any change of value.

Adafruit_MQTT_Subscribe Text_Feed_Adafruit = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/Text_Feed"); // FeedName

// Publishing allows the ESP 32 to upload data to the Adafruit IO Feeds

Adafruit_MQTT_Publish Temperature_Sensor_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature_Sensor");
Adafruit_MQTT_Publish Humidity_Sensor_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity_Sensor");
Adafruit_MQTT_Publish Light_Sensor_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Light_Sensor");
Adafruit_MQTT_Publish Absolute_Pressure_Sensor_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Absolute_Pressure_Sensor");
Adafruit_MQTT_Publish Altitude_Sensor_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Altitude_Sensor");
Adafruit_MQTT_Publish Relative_Pressure_Sensor_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Relative_Pressure_Sensor");
Adafruit_MQTT_Publish Temperature_OW_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature_OW");
Adafruit_MQTT_Publish Humidity_OW_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity_OW");
Adafruit_MQTT_Publish Pressure_OW_Adafruit = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Pressure_OW");



//CUSTOMIZABLE VALUES



//PIN DEFINITIONS
int Built_IN_Led = 2;
int LDR_Pin = 32;

//OPERATIONS

int LDR_ADC_Value = 0;

int Light_Sensor_Max_Percentage = 100;
int Light_Sesnor_Min_Percentage = 0;
int LDR_ADC_Max = 3400;
int LDR_ADC_Min = 100;

int Light_Sensor_Percentage = 0;
int Uploading = 0;
int Reset_Total = 0;


//Memory Points

int eeAddress = 0;
int eeAddress_Temperature = 0;
int eeAddress_Humidity = 0;
int eeAddress_Light = 0;


//DHT Temperature & Humidty Sensor Varibales

float Temperature = 0;
float Humidity = 0;
DHT dht(DHTPIN, DHTTYPE);


//Pressure Sensor Variables

SFE_BMP180 pressure;

#define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters
double T, P, p0, a; // T -> Temperature, P -> Absolute_Pressure, p0 -> Relative_Pressure, a -> Alttidue


//LCD Variables


int LCD_Delay = 68;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 4;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);




//Ad Message

String Ad_Message;


//API Open Weather

float Temperature_OW = 0;
float Humidity_OW = 0;
float Pressure_OW = 0;
float WindSpeed_OW = 0;


// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "eefda56d305bde4aac6ad331b6ae87d1";
// Example:
//String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";

// Replace with your country code and city
String city = "Greensboro";
String countryCode = "USA";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;



void MQTT_connect();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("SETUP STAGE Controller ID: ESP32 WEATHER STATION AND AD DISPLAY");
  pinMode(LDR_Pin, INPUT);
  pinMode(Built_IN_Led, OUTPUT);

  Setup_Stage_DHT11_Sensor();
  Setup_Stage_Pressure_Sensor_180();
  Setup_Stage_LCD();




  Serial.print("Connecting to :");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID , WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("...");
    Serial.println(" Retrying Connection... WIFI NOT AVAILABLE OR PASSWORD WRONG");
    Reset_Total = Reset_Total + 1;
    Serial.print(" Client Resetting in (/100) : ");
    Serial.println(Reset_Total);

    if ( Reset_Total >= 100 ) {
      resetFunc();
    }
    waiting3();
  }

  digitalWrite(Built_IN_Led, LOW);

  Serial.println("CONNECTED TO WIFI !");
  Serial.print("Status: "); Serial.println(WiFi.status());  // some parameters from the network
  Serial.print("IP: ");     Serial.println(WiFi.localIP());
  Serial.print("Subnet: "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  Serial.print("Signal: "); Serial.println(WiFi.RSSI());
  Serial.print("Networks: "); Serial.println(WiFi.scanNetworks());

  mqtt.subscribe(&Text_Feed_Adafruit);

  eeAddress += sizeof(float);
  eeAddress_Temperature = eeAddress;
  eeAddress += sizeof(float);
  eeAddress_Humidity = eeAddress;



  EEPROM.get(eeAddress_Temperature, Temperature);
  EEPROM.get(eeAddress_Humidity, Humidity);

}

void loop() {
  // put your main code here, to run repeatedly:

  waiting();
  Loop_Stage_Get_Temperature_Humidity();
  Loop_Stage_Get_Light_Sensor();
  Loop_Stage_Pressure_Sensor_180();
  Loop_Stage_Get_Open_Weather_API();
  Loop_Stage_LCD();
  CommunicatingWithCloud ();

  Serial.print("Ad_Message = ");
  Serial.println(Ad_Message);



}


void Loop_Stage_Get_Open_Weather_API() {

  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
//      JSONVar myObject = JSON.parse(jsonBuffer);
//
//      // JSON.typeof(jsonVar) can be used to get the type of the var
//      if (JSON.typeof(myObject) == "undefined") {
//        Serial.println("Parsing input failed!");
//        return;
//      }
//
//      Serial.print("JSON object = ");
//      Serial.println(myObject);
//      Serial.print("Temperature: ");
//      Serial.println(myObject["main"]["temp"]);
//      Serial.print("Pressure: ");
//      Serial.println(myObject["main"]["pressure"]);
//      Serial.print("Humidity: ");
//      Serial.println(myObject["main"]["humidity"]);
//      Serial.print("Wind Speed: ");
//      Serial.println(myObject["wind"]["speed"]);

      String input = jsonBuffer;

      StaticJsonDocument<1024> doc;

      DeserializationError error = deserializeJson(doc, input);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      float coord_lon = doc["coord"]["lon"]; // -8.61
      float coord_lat = doc["coord"]["lat"]; // 41.15

      JsonObject weather_0 = doc["weather"][0];
      int weather_0_id = weather_0["id"]; // 801
      const char* weather_0_main = weather_0["main"]; // "Clouds"
      const char* weather_0_description = weather_0["description"]; // "few clouds"
      const char* weather_0_icon = weather_0["icon"]; // "02d"

      const char* base = doc["base"]; // "stations"

      JsonObject main = doc["main"];
      float main_temp = main["temp"]; // 294.44
      float main_feels_like = main["feels_like"]; // 292.82
      float main_temp_min = main["temp_min"]; // 292.15
      float main_temp_max = main["temp_max"]; // 297.04
      int main_pressure = main["pressure"]; // 1008
      int main_humidity = main["humidity"]; // 63

      int visibility = doc["visibility"]; // 10000

      float wind_speed = doc["wind"]["speed"]; // 4.1




      Temperature_OW = main_temp;
      Temperature_OW = ((main_temp - 273.15) * (9 / 5) + 32);


      Humidity_OW = main_humidity;
      Pressure_OW = main_pressure;
      WindSpeed_OW = wind_speed;

      int wind_deg = doc["wind"]["deg"]; // 240

      int clouds_all = doc["clouds"]["all"]; // 20

      long dt = doc["dt"]; // 1589288330

      JsonObject sys = doc["sys"];
      int sys_type = sys["type"]; // 1
      int sys_id = sys["id"]; // 6900
      const char* sys_country = sys["country"]; // "PT"
      long sys_sunrise = sys["sunrise"]; // 1589260737
      long sys_sunset = sys["sunset"]; // 1589312564

      int timezone = doc["timezone"]; // 3600
      long id = doc["id"]; // 2735943
      const char* name = doc["name"]; // "Porto"
      int cod = doc["cod"]; // 200




    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}



String httpGETRequest(const char* serverName) {
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void Setup_Stage_LCD() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();

}
void Loop_Stage_LCD() {

  Serial.println("Printing to LCD Display");

  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print("Temperature = ");
  lcd.print(Temperature);

  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  // clears the display to print new message

  // set cursor to first column, second row
  lcd.setCursor(0, 1);
  lcd.print("Humidity = ");
  lcd.print(Humidity);




  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  CommunicatingWithCloud ();

  lcd.setCursor(0, 2);
  lcd.print("Light Sensor = ");
  lcd.print(Light_Sensor_Percentage);
  lcd.print("%");


  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }


  lcd.clear();



  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print("A Pressure= ");
  lcd.print(P);


  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  // clears the display to print new message

  // set cursor to first column, second row
  lcd.setCursor(0, 1);
  lcd.print("Altitude = ");
  lcd.print(a);


  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  lcd.setCursor(0, 2);
  lcd.print("R Pressure = ");
  lcd.print(p0);


  CommunicatingWithCloud ();



  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }


  lcd.clear();


  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print("AD Message = ");
  lcd.setCursor(0, 1);
  lcd.print(Ad_Message);


  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  // clears the display to print new message

  lcd.clear();


  lcd.setCursor(0, 0);
  // print message
  lcd.print("Temperature_OW = ");
  lcd.print(Temperature_OW);

  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  // clears the display to print new message

  // set cursor to first column, second row
  lcd.setCursor(0, 1);
  lcd.print("Humidity_OW = ");
  lcd.print(Humidity_OW);




  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

  CommunicatingWithCloud ();

  lcd.setCursor(0, 2);
  lcd.print("Pressure_OW  = ");
  lcd.print(Pressure_OW );



  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }


  lcd.setCursor(0, 3);
  lcd.print("WindSpeed_OW  = ");
  lcd.print(WindSpeed_OW );



  for (int i = 0; i <= LCD_Delay ; i++) { //Delay 1 Second
    waiting(); //30ms
  }


  // clears the display to print new message

  lcd.clear();


}


void Setup_Stage_Pressure_Sensor_180() {

  // Initialize the sensor (it is important to get calibration values stored on the device).

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while (1); // Pause forever.
  }

}


void Loop_Stage_Pressure_Sensor_180() {

  char status;

  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:

  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE, 0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE * 3.28084, 0);
  Serial.println(" feet");

  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.

  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T, 2);
      Serial.print(" deg C, ");
      Serial.print((9.0 / 5.0)*T + 32.0, 2);
      Serial.println(" deg F");

      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P, 2);
          Serial.print(" mb, ");
          Serial.print(P * 0.0295333727, 2);
          Serial.println(" inHg");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P, ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0, 2);
          Serial.print(" mb, ");
          Serial.print(p0 * 0.0295333727, 2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P, p0);
          Serial.print("computed altitude: ");
          Serial.print(a, 0);
          Serial.print(" meters, ");
          Serial.print(a * 3.28084, 0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  for (int i = 0; i <= 34 ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

}






void Loop_Stage_Get_Light_Sensor() {

  LDR_ADC_Value = analogRead(LDR_Pin); // 0 - 0V and 4095 - 3.3V

  Serial.print("LDR_ADC_Value =");
  Serial.println(LDR_ADC_Value);

  Light_Sensor_Percentage = map(LDR_ADC_Value ,  LDR_ADC_Min , LDR_ADC_Max , Light_Sesnor_Min_Percentage, Light_Sensor_Max_Percentage);


  Serial.print("Light_Sensor_Percentage = ");
  Serial.print(Light_Sensor_Percentage);
  Serial.println("%");

  for (int i = 0; i <= 34 ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

}


void Setup_Stage_DHT11_Sensor() {
  dht.begin();
}

void Loop_Stage_Get_Temperature_Humidity() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  Humidity = h;
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  Temperature = f;

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  for (int i = 0; i <= 34 ; i++) { //Delay 1 Second
    waiting(); //30ms
  }

}


void CommunicatingWithCloud () {

  if ((!(WiFi.status() == WL_CONNECTED))) {

    //    WiFi.disconnect();
    Serial.print("Connecting to :");
    Serial.println(WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("...");
      Serial.println(" Retrying Connection... WIFI NOT AVAILABLE OR PASSWORD WRONG");
      Reset_Total = Reset_Total + 1;
      Serial.print("  Resetting in (/100) : ");
      Serial.println(Reset_Total);

      if ( Reset_Total >= 100 ) {
        resetFunc();
      }
      waiting3();
    }

  }
  else {


    Serial.println("CONNECTED TO WIFI !");
    Serial.print("Status: "); Serial.println(WiFi.status());  // some parameters from the network
    Serial.print("IP: ");     Serial.println(WiFi.localIP());
    Serial.print("Subnet: "); Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
    Serial.print("SSID: "); Serial.println(WiFi.SSID());
    Serial.print("Signal: "); Serial.println(WiFi.RSSI());
    Serial.print("Networks: "); Serial.println(WiFi.scanNetworks());


    MQTT_connect();

    digitalWrite(Built_IN_Led, LOW);
    Serial.println(" Waiting For INCOMING SUBSCRIPTIONS...");
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(3000))) {


      if (subscription == &Text_Feed_Adafruit) {
        Serial.print(F("Got: "));
        Serial.println((char *)Text_Feed_Adafruit.lastread);
        Ad_Message = String((char *)Text_Feed_Adafruit.lastread);

        Serial.print("Ad_Message = ");
        Serial.println(Ad_Message);


        //              Ads_Remote_Control_CMD = atoi((char *)Ads_Remote_Control_CMD_Adafruit.lastread);
        //              Serial.print("Ads_Remote_Control_CMD =  ");
        //              Serial.println(Ads_Remote_Control_CMD);
        //
        //              EEPROM.put(eeAddress_Ads_Remote_Control_CMD, Ads_Remote_Control_CMD);
        //              EEPROM.commit();
        //              Serial.print("eeAddress_Ads_Remote_Control_CMD Written at Address = ");
        //              Serial.println(eeAddress_Ads_Remote_Control_CMD);
        //
      }

    }


    if ( Uploading == 2) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Temperature Value... "));
      Serial.print(Temperature);
      Serial.print("...");
      if (! Temperature_Sensor_Adafruit.publish(Temperature)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }


    if ( Uploading == 3) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Humidity Value... "));
      Serial.print(Humidity);
      Serial.print("...");
      if (! Humidity_Sensor_Adafruit.publish(Humidity)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }


    if ( Uploading == 4) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Light Sensor Value... "));
      Serial.print(Light_Sensor_Percentage );
      Serial.print("...");
      if (! Light_Sensor_Adafruit.publish(Light_Sensor_Percentage )) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }


    if ( Uploading == 5) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Absolute_Pressure_Sensor Value... "));
      Serial.print(P);
      Serial.print("...");
      if (! Absolute_Pressure_Sensor_Adafruit.publish(P)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }

    if ( Uploading == 6) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Altitude_Sensor Value... "));
      Serial.print(a);
      Serial.print("...");
      if (! Altitude_Sensor_Adafruit.publish(a)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }

    if ( Uploading == 7) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Relative_Pressure_Sensor Value... "));
      Serial.print(p0);
      Serial.print("...");
      if (! Relative_Pressure_Sensor_Adafruit.publish(p0)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }


    if ( Uploading == 8) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Temperature_OW Value... "));
      Serial.print(Temperature_OW);
      Serial.print("...");
      if (! Temperature_OW_Adafruit.publish(Temperature_OW)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }


    if ( Uploading == 9) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Humidity_OW Value... "));
      Serial.print(Humidity_OW);
      Serial.print("...");
      if (! Humidity_OW_Adafruit.publish(Humidity_OW)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }



    if ( Uploading == 10) {

      Serial.println("**UPLOADING DATA TO ADAFRUIT CLOUD**");

      Serial.print(F("\nSending Pressure_OW Value... "));
      Serial.print(Pressure_OW);
      Serial.print("...");
      if (! Pressure_OW_Adafruit.publish(Pressure_OW)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      waiting();
    }




    if ( Uploading >= 12) {
      Uploading = 0;
    }
    else {
      Uploading =   Uploading + 1;
      Serial.print(F("Uploading in ... "));
      Serial.println(Uploading);



    }
  }
}




void waiting() { // 30 ms waiting Time
  digitalWrite(Built_IN_Led, HIGH);
  delay(10);
  digitalWrite(Built_IN_Led, LOW);
  delay(10);
  digitalWrite(Built_IN_Led, HIGH);
  delay(10);
}



void waiting2() {
  for (int i = 0; i <= 1 ; i++) {
    digitalWrite(Built_IN_Led, HIGH);
    delay(50);
    digitalWrite(Built_IN_Led, LOW);
    delay(50);
    digitalWrite(Built_IN_Led, HIGH);
    delay(50);
    digitalWrite(Built_IN_Led, LOW);
    delay(50);
  }
}

void waiting3() {
  for (int i = 0; i <= 6 ; i++) { //6*500 = 3000
    digitalWrite(Built_IN_Led, HIGH);
    delay(50);
    digitalWrite(Built_IN_Led, LOW);
    delay(400);
    digitalWrite(Built_IN_Led, HIGH);
    delay(50);
  }
}



void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {

    Serial.println("MQTT Connected!");
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 5;

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    for (int i = 0; i <= 25 ; i++) {
      waiting2();  // wait 5 seconds
      waiting();
    }

    retries--;

    Serial.println("MQTT NOT Connected!");

    if (retries == 0) {

      Serial.println("MQTT NOT Connected Restart Activated!");
      resetFunc();


    }
  }
}
