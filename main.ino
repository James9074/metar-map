#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <FastLED.h>
#include <vector>
#include <cstring>

using namespace std;

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define NUM_AIRPORTS 23
#define SERVER "www.aviationweather.gov"
#define BASE_URI "/cgi-bin/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=true&stationString="
#define DEBUG true

boolean ledStatus = true; // used so leds only indicate connection status on first boot, or after failure
unsigned int lightningLoops;
int status = WL_IDLE_STATUS;

#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms
#define REQUEST_INTERVAL 900000 // in ms (15 min is 900000)

std::unique_ptr<BearSSL::WiFiClientSecure>SSL_CLIENT(new BearSSL::WiFiClientSecure); //Reusable

// Define the array of leds
CRGB leds[NUM_AIRPORTS];
#define DATA_PIN    4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 175

std::vector<unsigned short int> lightningLeds;
std::vector<String> airports({
  "KATL",
  "KMYR",
  "KCRE",
  "KILM",
  "KFLO",
  "KAFP",
  "KEQY",
  "KCLT",
  "KAVL",
  "KLOU",
  "KHKY",
  "KJQF",
  "KRUQ",
  "KRDU",
  "KROA",
  "KDCA",
  "KPHL",
  "KLGA",
  "KROC",
  "KBUF",
  "KPIT",
  "KCLE",
  "KDET"
});

const char* ssid = "XXXXX";
const char* password = "YYYYY";

int findAirportIndex(String airportCode){
  for (int i = 0; i < (NUM_AIRPORTS); i++) {
    if( airports[i] == airportCode){
      return i;
    }
  }
  return 0;    
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // give us control of the onboard LED
  digitalWrite(LED_BUILTIN, LOW);

  // Initialize LEDs - Full cyan while connecting to wifi
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);
  fill_solid(leds, NUM_AIRPORTS, CRGB::Cyan);
  FastLED.setBrightness(BRIGHTNESS);  
  FastLED.show();
}

void setAirport(String airportCode){    
    const char* weather = get_weather(airportCode).c_str();

    // Create a modifiable copy of the original string
    char* weatherCopy = new char[strlen(weather) + 1];
    strcpy(weatherCopy, weather);
    
    // Define the delimiter(s) for tokenization
    const char* delimiter = "\n";
    
    // Tokenize the string and loop through the tokens
    char* token = strtok(weatherCopy, delimiter);    
    while(token != nullptr) {        
        token = strtok(nullptr, delimiter);
        String flight_category = extractContent(token, "<flight_category>", "</flight_category>");
        if (flight_category != ""){
          if (DEBUG){
            Serial.print(airportCode.c_str());  
            Serial.print(": ");              
            Serial.println(flight_category.c_str());  
          }

          int airportNum = findAirportIndex(airportCode);

          if (flight_category == "VFR") leds[airportNum] = CRGB::Green;
          else if (flight_category == "WVFR") leds[airportNum] = CRGB::Yellow;
          else if (flight_category == "MVFR") leds[airportNum] = CRGB::Blue;
          else if (flight_category == "IFR") leds[airportNum] = CRGB::Red;
          else if (flight_category == "LIFR") leds[airportNum] = CRGB::Magenta;

          FastLED.show();
        }
    }
    
    // Clean up the copied string
    delete[] weatherCopy;        
}

void loop() {
  // wait for WiFi connection
  ensure_wifi();
  if ((WiFi.status() == WL_CONNECTED)) {
    fill_solid(leds, NUM_AIRPORTS, CRGB::Black);
    FastLED.show();

    // The aviationweather API allows for multiple in one go, but we don't
    // have enough RAM to pull them all at once, so loop through each.
    // TODO: One API call, parse the response stream as it comes in.
    bool firstAirport = false;
    String airportString = "";
    for (int i = 0; i < (NUM_AIRPORTS); i++) {
      setAirport(airports[i]);
      yield(); // ESP8266 employs a watchdog that will reboot the device if one loop() runs too long
    }    
    Serial.println("Waiting 15min to update");
    delay(900000);
  }
}



