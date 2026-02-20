// ============================================================
// ConexionWifi.hpp - Gestion WiFi con credenciales dinamicas
// ============================================================

#include <WiFi.h>

// ====== CREDENCIALES (cargadas desde Preferences) ======
char wifiSSID[33] = "";
char wifiPassword[64] = "";

// ====== CONTROL DE TIEMPO ======
unsigned long lastWifiCheck = 0;
const unsigned long wifiInterval = 5000; // 5 segundos

// ====== ESTADO ======
bool wifiConfigurado = false;

// ====== INICIALIZACION WIFI ======
// Retorna true si logra conectar, false si no hay credenciales o falla
bool wifiInit() {
  // Cargar credenciales desde Preferences
  if (!cargarCredenciales(wifiSSID, wifiPassword)) {
    Serial.println("[WiFi] No hay credenciales guardadas");
    return false;
  }

  Serial.printf("[WiFi] Conectando a: %s\n", wifiSSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPassword);

  // Esperar conexion (maximo 10 segundos)
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Conectado! IP: %s\n", WiFi.localIP().toString().c_str());
    wifiConfigurado = true;
    return true;
  } else {
    Serial.println("[WiFi] No se pudo conectar");
    WiFi.disconnect(true);
    return false;
  }
}

// ====== LOOP WIFI (RECONEXION AUTOMATICA) ======
void wifiLoop() {
  if (!wifiConfigurado) return;

  unsigned long now = millis();

  if (now - lastWifiCheck >= wifiInterval) {
    lastWifiCheck = now;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Desconectado, reconectando...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}
