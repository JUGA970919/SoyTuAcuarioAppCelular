// ============================================================
// SoyTuAcuario - Sistema de Monitoreo de Acuarios
// ESP32-C3 SuperMini
// ============================================================

#include "Structs.h"
#include "Config.h"
#include "Pantalla.hpp"
#include "ConfigBLE.hpp"
#include "ConexionWifi.hpp"

#include "FiltroSensor.hpp"
#include "SensorPh.hpp"
#include "SensorTemperatura.hpp"
#include "Sensortds.hpp"
#include "SensorFlotador.hpp"
#include "SensorTurbidez.hpp"
#include "SolicitudDatos.hpp"

const int PinD5 = 5;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("   SoyTuAcuario - Iniciando sistema");
  Serial.println("========================================\n");

  // Inicializar I2C y pantalla primero (necesario para mostrar estado)
  Wire.begin(SDA_PIN, SCL_PIN);
  pantallaIni();

  // Verificar si hay credenciales WiFi guardadas
  if (!hayCredencialesGuardadas()) {
    // No hay credenciales -> Modo BLE automatico
    Serial.println("[Sistema] Primera configuracion - Modo BLE");
    modoOperacion = MODO_CONFIG_BLE;
    iniciarModoBLE();
    return;  // No continuar con setup normal
  }

  // Intentar conectar WiFi
  if (!wifiInit()) {
    // Fallo conexion -> Modo BLE automatico
    Serial.println("[Sistema] Fallo WiFi - Modo BLE");
    modoOperacion = MODO_CONFIG_BLE;
    iniciarModoBLE();
    return;  // No continuar con setup normal
  }

  // WiFi conectado -> Continuar setup normal
  Serial.println("[Sistema] WiFi OK - Modo Normal");
  modoOperacion = MODO_NORMAL;
  setupModoNormal();
}

// ====== Setup del modo normal (sensores) ======
void setupModoNormal() {
  AdsIni();

  InitAnalogFilter(PhFilter, canal0Ph, 0.03);
  InitAnalogFilter(TdsFilter, canal1Tds, 0.03);
  InitAnalogFilter(TurbidezFilter, canal2Turb, 0.03);

  InicioTemperatura();
  FlotadorIni();
  pinMode(PinD5, OUTPUT);

  Serial.println("[Sistema] Sensores inicializados");
}

void loop() {
  // ====== MODO CONFIG BLE ======
  if (modoOperacion == MODO_CONFIG_BLE) {
    int resultado = loopModoBLE();

    if (resultado == 1) {
      // Conexion exitosa - reiniciar
      Serial.println("[Sistema] Configuracion exitosa - Reiniciando...");
      delay(1000);
      ESP.restart();
    }
    else if (resultado == -1) {
      // Timeout - reiniciar para intentar de nuevo
      Serial.println("[Sistema] Timeout BLE - Reiniciando...");
      delay(1000);
      ESP.restart();
    }

    // resultado == 0: continuar en modo BLE
    return;
  }

  // ====== MODO NORMAL (sensado) ======
  wifiLoop();
  Setponts();

  unsigned long now = millis();

  if (calibracionph == 0) {
    // Cambio de estado cada 5 segundos
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
      digitalWrite(PinD5, HIGH);
      LecturaTds();
      LecturaTemperatura();
    }
  }
  else if (calibracionph == 1) {
    // Modo calibracion pH
    digitalWrite(PinD5, LOW);
    LecturaPh();
    Serial.print(voltagePh);
    Serial.print(",");
  }

  StatusButton(nivelAgua, flotador, 3000);
  MostarPantalla();

  // Enviar datos por Serial
  Serial.print(ph);
  Serial.print(",");
  Serial.print(temperaturaCelcius);
  Serial.print(",");
  Serial.print(turbidez);
  Serial.print(",");
  Serial.print(tds);
  Serial.print(",");
  Serial.print(nivelAgua.estadoFlotador);
  Serial.println(",");
}
