#include <WiFi.h>

// ====== CREDENCIALES (cargadas desde Preferences) ======
char wifiSSID[33] = "";
char wifiPassword[64] = "";

// ====== CONTROL DE TIEMPO ======
unsigned long lastWifiCheck = 0;
const unsigned long wifiInterval = 5000; // 5 segundos

// ====== Verificar si hay credenciales guardadas ======
bool hayCredencialesGuardadas() {
  preferences.begin("wifi", true);
  bool configurado = preferences.getBool("configurado", false);
  preferences.end();
  return configurado;
}

// ====== Cargar credenciales desde Preferences ======
bool cargarCredenciales() {
  preferences.begin("wifi", true);
  String storedSSID = preferences.getString("ssid", "");
  String storedPass = preferences.getString("password", "");
  preferences.end();

  if (storedSSID.length() > 0) {
    storedSSID.toCharArray(wifiSSID, 33);
    storedPass.toCharArray(wifiPassword, 64);
    return true;
  }
  return false;
}

// ====== INICIALIZACION WIFI ======
// Retorna true si conecta, false si falla
bool wifiInit() {
  if (!cargarCredenciales()) {
    Serial.println("[WiFi] No hay credenciales guardadas");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPassword);

  Serial.print("[WiFi] Conectando a: ");
  Serial.println(wifiSSID);

  // Esperar conexion (maximo 10 segundos)
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WiFi] Conectado | IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("[WiFi] No se pudo conectar");
    WiFi.disconnect(true);
    return false;
  }
}

// ====== LOOP WIFI (RECONEXION AUTOMATICA) ======
void wifiLoop() {
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
