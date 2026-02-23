#include "Structs.h"
#include "Config.h"
#include "ConexionWifi.hpp"
#include "Pantalla.hpp"

#include "FiltroSensor.hpp"
#include "SensorPh.hpp"
#include "SensorTemperatura.hpp"
#include "Sensortds.hpp"
#include "SensorFlotador.hpp"
#include "SensorTurbidez.hpp"
#include "SolicitudDatos.hpp"


//const int PinD5 = 5;

// pin para esp32 normal
const int PinD5 = 17;


void setup() {
  Serial.begin(115200);
  delay(1000);
  wifiInit();

  Wire.begin(SDA_PIN, SCL_PIN);

  pantallaIni();
  AdsIni ();

  InitAnalogFilter(PhFilter, canal0Ph, 0.03);
  InitAnalogFilter(TdsFilter, canal1Tds, 0.03);
  InitAnalogFilter(TurbidezFilter, canal2Turb, 0.03);

  InicioTemperatura();
  FlotadorIni ();
  pinMode(PinD5, OUTPUT);

}

void loop() {

  wifiLoop();
  Setponts();
  unsigned long now = millis();


  if (calibracionph == 0){    // si en la aplicacion no se esta haciendo la calibracion el monitoreo se realizara haciendo el swicheo 
     // Cambio de estado cada 1 segundo
    if (now - lastChangeTime >= swcheo) {
      lastChangeTime = now;
      estadoSensoresAgua = !estadoSensoresAgua;
    }

    // ===== ESTADO 1: PH, TEMPERATURA, TURBIDEZ =====
    if (estadoSensoresAgua) {
      
      digitalWrite(PinD5, LOW);
      LecturaPh();
      LecturaTurbidez();
      
    }
    // ===== ESTADO 2: TDS =====
    else {
    digitalWrite(PinD5, HIGH);  // D5 en 1 lógico desde el inicio
    LecturaTds();
    LecturaTemperatura();
    }
  }
  /* si la calibracion se realiza se apaga el pin del sensor de TDS y solo se realiza el muestreo del sensor de ph
    el monitoreo de la variable "voltagePh" se devera de mandar en tiempo real sin ningun retardo 
     al salir del modo calibracion la aplicacion debe mandar a 0 la variable "calibracionph" para qeu continue con el sensado
  */
  else if (calibracionph == 1){   
    digitalWrite(PinD5, LOW);     
    LecturaPh();     
    Serial.print(voltagePh);
    Serial.println(",");              
  }

  StatusButton(nivelAgua,flotador,3000);

  MostarPantalla();

  Serial.print(ph);
  Serial.print(",");   
  Serial.print(temperaturaCelcius);
  Serial.print(",");
  Serial.print(turbidez); 
  Serial.print(",");
  Serial.print(tds); 
  Serial.print(",");
  Serial.print(nivelAgua.estadoFlotador); 
  Serial.println(":");  
}
