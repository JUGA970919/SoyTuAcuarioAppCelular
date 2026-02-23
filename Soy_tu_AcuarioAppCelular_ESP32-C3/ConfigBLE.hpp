// ============================================================
// ConfigBLE.hpp - Configuracion WiFi via BLE para SoyTuAcuario
// ============================================================

#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ====== UUIDs del Servicio BLE ======
#define SERVICE_UUID           "12345678-1234-1234-1234-123456789abc"
#define CHAR_REDES_UUID        "12345678-1234-1234-1234-123456789001"  // READ - Lista de redes
#define CHAR_CREDENCIALES_UUID "12345678-1234-1234-1234-123456789002"  // WRITE - SSID:PASSWORD
#define CHAR_ESTADO_UUID       "12345678-1234-1234-1234-123456789003"  // NOTIFY - Estado conexion

// ====== Constantes ======
#define MAX_REDES 10
#define MAX_SSID_LENGTH 33
#define BLE_TIMEOUT_MS 300000  // 5 minutos

// ====== Variables BLE ======
BLEServer* pServer = nullptr;
BLECharacteristic* pCharRedes = nullptr;
BLECharacteristic* pCharCredenciales = nullptr;
BLECharacteristic* pCharEstado = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool credencialesRecibidas = false;
unsigned long bleStartTime = 0;

// Buffer para redes escaneadas
char redesWiFi[MAX_REDES][MAX_SSID_LENGTH];
int numRedes = 0;

// Credenciales recibidas
char ssidRecibido[MAX_SSID_LENGTH] = "";
char passwordRecibido[64] = "";

// ====== Callbacks de Conexion BLE ======
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("[BLE] Dispositivo conectado");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("[BLE] Dispositivo desconectado");
  }
};

// ====== Callback para recibir credenciales ======
class CredencialesCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();

    if (value.length() > 0) {
      Serial.print("[BLE] Credenciales recibidas: ");
      Serial.println(value.c_str());

      // Formato esperado: "SSID:PASSWORD"
      int separatorIndex = value.indexOf(':');

      if (separatorIndex > 0) {
        String ssid = value.substring(0, separatorIndex);
        String pass = value.substring(separatorIndex + 1);

        ssid.toCharArray(ssidRecibido, MAX_SSID_LENGTH);
        pass.toCharArray(passwordRecibido, 64);

        credencialesRecibidas = true;

        // Notificar que estamos intentando conectar
        pCharEstado->setValue("CONECTANDO");
        pCharEstado->notify();
      } else {
        Serial.println("[BLE] Formato invalido. Use SSID:PASSWORD");
        pCharEstado->setValue("ERROR_FORMATO");
        pCharEstado->notify();
      }
    }
  }
};

// ====== Escanear redes WiFi disponibles ======
void escanearRedesWiFi() {
  Serial.println("[WiFi] Escaneando redes...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  Serial.printf("[WiFi] %d redes encontradas\n", n);

  numRedes = min(n, MAX_REDES);

  for (int i = 0; i < numRedes; i++) {
    String ssid = WiFi.SSID(i);
    ssid.toCharArray(redesWiFi[i], MAX_SSID_LENGTH);
    Serial.printf("  %d: %s (%d dBm)\n", i + 1, redesWiFi[i], WiFi.RSSI(i));
  }

  WiFi.scanDelete();
}

// ====== Construir string de redes para BLE ======
String construirListaRedes() {
  String lista = "";

  for (int i = 0; i < numRedes; i++) {
    if (i > 0) lista += "|";
    lista += redesWiFi[i];
  }

  return lista;
}

// ====== Generar nombre BLE con MAC ======
String generarNombreBLE() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char nombre[20];
  sprintf(nombre, "SoyTuAcuario_%02X%02X", mac[4], mac[5]);
  return String(nombre);
}

// ====== Mostrar pantalla modo BLE ======
void mostrarPantallaBLE(const char* estado) {
  u8g2.clearBuffer();

  // Titulo
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 15, "MODO CONFIG");

  // Icono BLE (circulo con B)
  u8g2.drawCircle(64, 38, 12);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(58, 44, "B");

  // Estado
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int width = u8g2.getStrWidth(estado);
  u8g2.drawStr((128 - width) / 2, 62, estado);

  u8g2.sendBuffer();
}

// ====== Inicializar BLE ======
void iniciarModoBLE() {
  Serial.println("\n========================================");
  Serial.println("[BLE] Iniciando modo configuracion...");
  Serial.println("========================================\n");

  // 1. Escanear redes WiFi ANTES de apagar WiFi
  mostrarPantallaBLE("Escaneando...");
  escanearRedesWiFi();

  // 2. Apagar WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  Serial.println("[WiFi] Apagado");

  // 3. Inicializar BLE
  String nombreBLE = generarNombreBLE();
  Serial.printf("[BLE] Nombre: %s\n", nombreBLE.c_str());

  BLEDevice::init(nombreBLE.c_str());

  // 4. Crear servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // 5. Crear servicio
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // 6. Caracteristica: Lista de redes (READ)
  pCharRedes = pService->createCharacteristic(
    CHAR_REDES_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  String listaRedes = construirListaRedes();
  pCharRedes->setValue(listaRedes.c_str());
  Serial.printf("[BLE] Redes expuestas: %s\n", listaRedes.c_str());

  // 7. Caracteristica: Credenciales (WRITE)
  pCharCredenciales = pService->createCharacteristic(
    CHAR_CREDENCIALES_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharCredenciales->setCallbacks(new CredencialesCallback());

  // 8. Caracteristica: Estado (READ + NOTIFY)
  pCharEstado = pService->createCharacteristic(
    CHAR_ESTADO_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharEstado->addDescriptor(new BLE2902());
  pCharEstado->setValue("ESPERANDO");

  // 9. Iniciar servicio y advertising
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  bleStartTime = millis();
  credencialesRecibidas = false;

  mostrarPantallaBLE("Esperando...");
  Serial.println("[BLE] Advertising iniciado. Esperando conexion...\n");
}

// ====== Probar conexion WiFi con credenciales recibidas ======
bool probarConexionWiFi() {
  Serial.printf("[WiFi] Probando conexion a: %s\n", ssidRecibido);
  mostrarPantallaBLE("Conectando...");

  // Apagar BLE temporalmente para probar WiFi
  BLEDevice::deinit(false);
  delay(100);

  // Intentar conectar
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidRecibido, passwordRecibido);

  unsigned long startAttempt = millis();
  const unsigned long timeout = 15000; // 15 segundos

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Conectado! IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.println("[WiFi] Fallo la conexion");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
  }
}

// ====== Guardar credenciales en Preferences ======
void guardarCredenciales() {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssidRecibido);
  preferences.putString("password", passwordRecibido);
  preferences.putBool("configurado", true);
  preferences.end();

  Serial.println("[Flash] Credenciales guardadas en Preferences");
}

// ====== Verificar si hay credenciales guardadas ======
bool hayCredencialesGuardadas() {
  preferences.begin("wifi", true);
  bool configurado = preferences.getBool("configurado", false);
  preferences.end();
  return configurado;
}

// ====== Cargar credenciales desde Preferences ======
bool cargarCredenciales(char* ssid, char* password) {
  preferences.begin("wifi", true);

  String storedSSID = preferences.getString("ssid", "");
  String storedPass = preferences.getString("password", "");

  preferences.end();

  if (storedSSID.length() > 0) {
    storedSSID.toCharArray(ssid, MAX_SSID_LENGTH);
    storedPass.toCharArray(password, 64);
    return true;
  }

  return false;
}

// ====== Loop del modo BLE ======
// Retorna: 0 = continuar, 1 = conectado exitoso, -1 = timeout
int loopModoBLE() {
  unsigned long now = millis();

  // Verificar timeout (solo si no hay dispositivo conectado)
  if (!deviceConnected && (now - bleStartTime > BLE_TIMEOUT_MS)) {
    Serial.println("[BLE] Timeout - reiniciando...");
    mostrarPantallaBLE("Timeout...");
    delay(1000);
    return -1;
  }

  // Manejar reconexion
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("[BLE] Reiniciando advertising...");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    mostrarPantallaBLE("Conectado!");
  }

  // Procesar credenciales recibidas
  if (credencialesRecibidas) {
    credencialesRecibidas = false;

    if (probarConexionWiFi()) {
      // Conexion exitosa
      guardarCredenciales();
      mostrarPantallaBLE("OK! Reiniciando");

      // Notificar exito (aunque BLE este apagado, es para el log)
      Serial.println("[BLE] Notificando CONECTADO");

      delay(2000);
      return 1; // Exito
    } else {
      // Fallo - volver a modo BLE
      Serial.println("[BLE] Conexion fallida, volviendo a modo BLE...");
      mostrarPantallaBLE("Error WiFi");
      delay(2000);

      // Reiniciar BLE
      iniciarModoBLE();

      // Notificar error
      pCharEstado->setValue("NO_CONECTADO");
      pCharEstado->notify();
    }
  }

  return 0; // Continuar en modo BLE
}

// ====== Detener BLE ======
void detenerBLE() {
  BLEDevice::deinit(true);
  Serial.println("[BLE] Detenido");
}
