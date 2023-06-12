



//imports
#include <WiFi.h>
#include "time.h"
#include <SPI.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//set up global values
const char* ssid       = "[YOUR NETWORK NAME HERE]";
const char* password   = "[YOUR NETWORK PASSWORD HERE]";
//replace the "[YOUR NETWORK NAME HERE]" and "[YOUR NETWORK PASSWORD HERE]" with the actual values corresponding to your WiFi connection

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -14400;
const int   daylightOffset_sec = 0;
//above two values may need to change depending on your time zone


DynamicJsonDocument doc(24000);
DeserializationError error;

float temp = 0;
float wind = 0;
float precip = 0;

//use the api for open-meteo.com here:
#define WEATHER_API "https://api.open-meteo.com/v1/forecast?latitude=[YOUR LATITUDE HERE]&longitude=[YOUR LONGITUDE HERE]&hourly=precipitation_probability&current_weather=true&forecast_days=1&timezone=America%2FNew_York"
//replace the "[YOUR LATITUDE HERE]" and "[YOUR LONGITUDE HERE]" with the actual values corresponding to your area
//this information can be found online

#define SCREEN_WIDTH 128 // display width
#define SCREEN_HEIGHT 64 // display height
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void getWeather(){
  display.clearDisplay();
  display.setTextSize(2);         //set text size   
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.setTextSize(2);         //set text size   
  display.print(F("Updating"));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    display.print(F("."));
    display.display();
  }
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
      display.clearDisplay();
      display.setTextSize(2);         //set text size   
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.setTextSize(2);         //set text size
      display.print("Could not get time");
      display.display();
      delay(5000);
  }else{
    HTTPClient client;
    client.begin(WEATHER_API);
    int httpCode = client.GET();
    if(httpCode > 0 ){
      String payload = client.getString();
      error = deserializeJson(doc, payload);
      temp = doc["current_weather"]["temperature"].as<float>();
      wind = doc["current_weather"]["windspeed"].as<float>();
      precip = doc["hourly"]["precipitation_probability"][timeinfo.tm_hour].as<float>();

    }else{
      display.clearDisplay();
      display.setTextSize(2);         //set text size   
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.setTextSize(2);         //set text size
      display.print("Error fetching data");
      display.display();
      delay(5000);   
    }
  }


  updateScreen();

  //WiFi doesn't need to be used yet
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}






void updateScreen(){
  display.clearDisplay();
  display.setCursor(0,0); //start printing from top left corner
  display.setTextSize(2);         //set text size   

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    display.print(F("ERROR"));
    return;
  }else{

    if (timeinfo.tm_min == 0 && timeinfo.tm_sec == 0){
      getWeather();
    }

    display.print(&timeinfo, "%H:%M:%S\n");
    display.setTextSize(1);         //set text size 
    display.print(&timeinfo, "%A\n%B %d %Y\n\n");
    display.print("Temp: ");
    display.print(temp);
    display.print(" C\nWind Spd: ");
    display.print(wind);
    display.print(" Km/h\nPrecip. rate: ");
    display.print(precip);
    display.print("%\n");
  }
  display.display();
}


void setup() {

  //set up console for error message
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  delay(1000);
  getWeather();


}

void loop() {
  updateScreen();
  delay(1000);
  

}



