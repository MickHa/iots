#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define PIN        5 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 12 // Popular NeoPixel ring size
#define DIM 3        // how much do dim the LEDs by

#define WIFI_SSID "MYSSID"
#define WIFI_password "MYPASSWORD"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
HTTPClient http;
StaticJsonDocument<60000> doc;

uint32_t dimmedColor(byte r, byte g, byte b) {
  return pixels.Color(r / DIM, g / DIM, b / DIM);
}

uint32_t OFF  =   dimmedColor(  0,   0,   0);
uint32_t GREEN  = dimmedColor(  0, 228,   0);
uint32_t GREENY = dimmedColor( 80, 228,   0);
uint32_t YELLOW = dimmedColor(255, 255,   0);
uint32_t ORANGE = dimmedColor(255,  50,   0);
uint32_t RED    = dimmedColor(255,   0,   0);
uint32_t PURPLE = dimmedColor(143,  63, 151);
uint32_t MAROON = dimmedColor(126,   0,  35);

void setup() {
  Serial.begin(115200);
  pixels.begin();
  WiFi.begin(WIFI_SSID, WIFI_password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    pixels.clear();
    pixels.setPixelColor(i = (i + 1) % NUMPIXELS, GREEN);
    pixels.show();
  }
  pixels.clear();
  pixels.show();
}

uint32_t colorForP25(float p25) {
  // based on https://www3.epa.gov/airnow/aqi-technical-assistance-document-sept2018.pdf, Table 4
  return  p25 <=   6.0 ? GREEN 
        : p25 <=  12.0 ? GREENY  //     - 50
        : p25 <=  35.4 ? YELLOW  //  51 - 100
        : p25 <=  55.4 ? ORANGE  // 101 - 150
        : p25 <= 150.4 ? RED     // 151 - 200
        : p25 <= 250.4 ? PURPLE  // 201 - 300
        : MAROON;  
}

uint32_t pixelColors[NUMPIXELS][2] = {};

uint32_t updateColor(uint32_t pixel, int sensor) {
  pixelColors[pixel][0] = OFF;
  pixelColors[pixel][1] = OFF;
  String url = "https://www.purpleair.com/json?show=" + String(sensor);
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
      String payload = http.getString();
      DeserializationError error = deserializeJson(doc, payload);
      if (!error){
        String stats = doc["results"][0]["Stats"].as<String>();
        error = deserializeJson(doc, stats);
        if (!error) {
          float p25 = doc["v"].as<float>();
          float p25_10min = doc["v1"].as<float>();
          pixelColors[pixel][0] = colorForP25(p25);
          pixelColors[pixel][1] = colorForP25(p25_10min);
          // Serial.println(colorForP25(p25));
        }
      }
  } else {
    // Serial.println("Error on HTTP request");
  }
  http.end();
}

void loop() {
  pixels.clear();
  pixels.show();
  // Pixels # for NeoPixel Ring.
  updateColor( 7, 12811);
  updateColor(10, 33609);
  updateColor( 1, 20165);
  updateColor( 4, 37003);
  delay(500);

  // cycle between 0th and 1st color
  for(int j = 0; j < 10; j++) {
    for(int pixel = 0; pixel < NUMPIXELS; pixel++) {
      pixels.setPixelColor(pixel, pixelColors[pixel][0]);
    }
    pixels.show();
    delay(1000);
    for(int pixel = 0; pixel < NUMPIXELS; pixel++) {
      pixels.setPixelColor(pixel, pixelColors[pixel][1]);
    }
    pixels.show();
    delay(5000);
  }
}
