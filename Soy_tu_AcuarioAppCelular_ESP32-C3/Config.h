//################ Variables ADS1115#####################
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Pines I2C ESP32-C3 SuperMini
#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_ADS1115 ads;
//#######################################################################

//################ Sensor de PH #####################

#include <Preferences.h>

Preferences preferences;

AnalogFilter PhFilter;
int canal0Ph = 0;
float miliVoltagePh = 0;
float voltagePh = 0;
float offsetph = 22.446871;  //b
float slopeph= -9.141055;     //m
float ph = 0;
//#######################################################

//################ Sensor de temperatura #####################
#include <OneWire.h>
#include <DallasTemperature.h>

#define OneWirePin 2

OneWire oneWire(OneWirePin);
DallasTemperature sensors(&oneWire);
float temperaturaCelcius = 0;
//#######################################################

//################ Sensor de TDS #####################
AnalogFilter TdsFilter;
int canal1Tds = 1;
float miliVoltageTds = 0;
float voltageTds = 0;
float factor = 0;
int compensatedVoltageTds = 0;
float calculoTds =0;
int tds = 0;
//#######################################################

//################ Sensor de flotador #####################

#define flotador 4
int estadoFlotador =0;
DigitalFilter nivelAgua;
//#######################################################

//################ Sensor de Turbidez #####################

AnalogFilter TurbidezFilter;
int canal2Turb = 2;
int miliVoltageTurbidez = 0;
float voltageTurbidez = 0;
float calculoTurbidez = 0;
float turbidez = 0;
//#######################################################

//################ Pantall Oled #####################
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

unsigned long lastChange = 0;
const unsigned long interval = 3000;
uint8_t screenIndex = 0;

//#######################################################

//################ Swicheo de datos #####################
unsigned long lastChangeTime = 0;
const unsigned long swcheo = 5000; // 5 segundo

bool estadoSensoresAgua = true; // true = PH/TEMP/TURB, false = TDS

//#######################################################

//################ Calibracion ph ##########################
int calibracionph = 0;  // esta variable se recibe desde la app movil
String buferph = "0";
float bufferph_1 = 4.0;
float bufferph_2 = 6.81;
float bufferph_3 = 9.18;
float  ph_1 = 0;
float  ph_2 = 0;
float  ph_3 = 0;
int pasoCalibracionph = 0;

//#######################################################








