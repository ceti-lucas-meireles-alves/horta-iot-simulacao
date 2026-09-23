#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <DHTesp.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* LOCAL_AP_SSID = "Horta-IoT-Local";

constexpr uint8_t RFID_SS = 5;
constexpr uint8_t RFID_RST = 22;
constexpr uint8_t SERVO_PIN = 13;
constexpr uint8_t MOISTURE_PIN = 34;
constexpr uint8_t GREEN_LED = 26;
constexpr uint8_t RED_LED = 27;
constexpr uint8_t BUTTON_PIN = 25;
constexpr uint8_t DHT_PIN = 14;
constexpr uint8_t TANK_TRIG = 32;
constexpr uint8_t TANK_ECHO = 33;
constexpr uint8_t LIGHT_PIN = 35;
constexpr int DRY_LIMIT = 35;
constexpr int WET_LIMIT = 65;
constexpr unsigned long SENSOR_INTERVAL_MS = 2000;

MFRC522 rfid(RFID_SS, RFID_RST);
Servo valve;
WebServer server(80);
DHTesp climate;

enum OperationMode { MANUAL, AUTOMATIC, OBSERVATION };
OperationMode mode = MANUAL;

// UIDs dos cartões de teste disponíveis no Wokwi.
const String authorizedCards[] = { "01020304", "11223344" };
constexpr size_t authorizedCardCount = sizeof(authorizedCards) / sizeof(authorizedCards[0]);

bool authorized = false;
bool irrigation = false;
bool localNetwork = false;
String lastUser = "nenhum";
String lastEvent = "sistema iniciado";
unsigned long irrigationStartedAt = 0;
unsigned long lastButtonAt = 0;
unsigned long lastSensorReadAt = 0;
float airTemperature = NAN;
float airHumidity = NAN;
float tankDistance = NAN;
int lightLevel = 0;

String networkName() {
  return localNetwork ? String(LOCAL_AP_SSID) : String(WIFI_SSID);
}

int moisturePercent() {
  int raw = analogRead(MOISTURE_PIN);
  return constrain(map(raw, 0, 4095, 0, 100), 0, 100);
}

void readEnvironmentalSensors() {
  if (millis() - lastSensorReadAt < SENSOR_INTERVAL_MS) return;
  lastSensorReadAt = millis();

  TempAndHumidity reading = climate.getTempAndHumidity();
  if (!isnan(reading.temperature)) airTemperature = reading.temperature;
  if (!isnan(reading.humidity)) airHumidity = reading.humidity;

  digitalWrite(TANK_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TANK_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TANK_TRIG, LOW);
  unsigned long duration = pulseIn(TANK_ECHO, HIGH, 30000);
  tankDistance = duration > 0 ? duration * 0.0343f / 2.0f : NAN;
  lightLevel = map(analogRead(LIGHT_PIN), 0, 4095, 0, 100);
}

String modeName() {
  if (mode == AUTOMATIC) return "automático";
  if (mode == OBSERVATION) return "observação";
  return "manual";
}

bool isCardAuthorized(const String& uid) {
  for (size_t i = 0; i < authorizedCardCount; i++) {
    if (uid == authorizedCards[i]) return true;
  }
  return false;
}

void logEvent(const String& event) {
  lastEvent = event;
  Serial.printf("EVENTO | modo=%s | umidade=%d%% | usuario=%s | %s\n",
                modeName().c_str(), moisturePercent(), lastUser.c_str(), event.c_str());
}

void setIrrigation(bool enabled, const String& reason) {
  if (enabled && (!authorized || mode == OBSERVATION)) {
    logEvent("irrigacao bloqueada: sem autorizacao ou modo observacao");
    return;
  }

  irrigation = enabled;
  valve.write(irrigation ? 90 : 0);
  digitalWrite(GREEN_LED, irrigation ? HIGH : LOW);
  digitalWrite(RED_LED, irrigation ? LOW : HIGH);

  if (irrigation) {
    irrigationStartedAt = millis();
    logEvent("irrigacao iniciada: " + reason);
  } else {
    unsigned long seconds = irrigationStartedAt == 0 ? 0 : (millis() - irrigationStartedAt) / 1000;
    logEvent("irrigacao encerrada: " + reason + ", duracao=" + String(seconds) + "s");
    irrigationStartedAt = 0;
  }
}

void evaluateAutomaticMode() {
  int moisture = moisturePercent();
  if (mode != AUTOMATIC || !authorized) return;

  if (!irrigation && moisture <= DRY_LIMIT) {
    setIrrigation(true, "solo seco");
  } else if (irrigation && moisture >= WET_LIMIT) {
    setIrrigation(false, "umidade adequada");
  }
}

String html() {
  int moisture = moisturePercent();
  readEnvironmentalSensors();
  String recommendation = moisture <= DRY_LIMIT ? "irrigar" : moisture >= WET_LIMIT ? "não irrigar" : "observar";
  String page = "<!doctype html><html lang='pt-BR'><meta charset='utf-8'>";
  page += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  page += "<title>Horta IoT Escolar</title><style>body{font-family:Arial;max-width:760px;margin:2rem auto;padding:0 1rem;color:#163020}.card{padding:1rem;margin:1rem 0;border-radius:12px;background:#e8f5e9}button{padding:.7rem 1rem;margin:.3rem;border:0;border-radius:8px;background:#2e7d32;color:#fff}a{text-decoration:none}</style>";
  page += "<h1>Horta IoT Escolar</h1><div class='card'><h2>Monitoramento</h2>";
  page += "<p><b>Umidade simulada:</b> " + String(moisture) + "%</p>";
  page += "<p><b>Temperatura do ar:</b> " + String(airTemperature, 1) + " °C</p>";
  page += "<p><b>Umidade do ar:</b> " + String(airHumidity, 1) + "%</p>";
  page += "<p><b>Distância no reservatório:</b> " + (isnan(tankDistance) ? String("sem leitura") : String(tankDistance, 1) + " cm") + "</p>";
  page += "<p><b>Luminosidade:</b> " + String(lightLevel) + "%</p>";
  page += "<p><b>Recomendação:</b> " + recommendation + "</p>";
  page += "<p><b>Modo:</b> " + modeName() + "</p>";
  page += "<p><b>Rede:</b> " + networkName() + (localNetwork ? " (local/offline)" : " (Wi-Fi)") + "</p>";
  page += String("<p><b>RFID:</b> ") + (authorized ? "autorizado" : "aguardando cartão") + "</p>";
  page += "<p><b>Usuário:</b> " + lastUser + "</p>";
  page += "<p><b>Irrigação:</b> " + String(irrigation ? "ativa" : "parada") + "</p>";
  page += "<p><b>Último evento:</b> " + lastEvent + "</p></div>";
  page += "<p>Modo: <a href='/mode?value=manual'><button>Manual</button></a><a href='/mode?value=auto'><button>Automático</button></a><a href='/mode?value=observe'><button>Observação</button></a></p>";
  page += "<p><a href='/irrigar'><button>Iniciar irrigação</button></a><a href='/parar'><button>Parar</button></a></p>";
  page += "<p>O potenciômetro representa a umidade do solo; o servo representa a válvula. O HC-SR04 usa divisor resistivo no ECHO.</p></html>";
  return page;
}

void setupRoutes() {
  server.on("/", []() { server.send(200, "text/html; charset=utf-8", html()); });
  server.on("/irrigar", []() { setIrrigation(true, "comando web"); server.sendHeader("Location", "/"); server.send(302); });
  server.on("/parar", []() { setIrrigation(false, "comando web"); server.sendHeader("Location", "/"); server.send(302); });
  server.on("/mode", []() {
    String value = server.arg("value");
    if (value == "auto") mode = AUTOMATIC;
    else if (value == "observe") mode = OBSERVATION;
    else mode = MANUAL;
    if (mode != AUTOMATIC) setIrrigation(false, "mudanca de modo");
    logEvent("modo alterado para " + modeName());
    server.sendHeader("Location", "/");
    server.send(302);
  });
}

void readRfid() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  authorized = isCardAuthorized(uid);
  lastUser = "cartao " + uid;
  logEvent(authorized ? "acesso autorizado" : "acesso negado");
  if (!authorized) setIrrigation(false, "cartao nao autorizado");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TANK_TRIG, OUTPUT);
  pinMode(TANK_ECHO, INPUT);
  climate.setup(DHT_PIN, DHTesp::DHT22);
  valve.attach(SERVO_PIN);
  valve.write(0);
  digitalWrite(RED_LED, HIGH);

  SPI.begin();
  rfid.PCD_Init();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  unsigned long wifiStartedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStartedAt < 10000) delay(100);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(LOCAL_AP_SSID);
    localNetwork = true;
    Serial.print("Rede local ativa em http://");
    Serial.println(WiFi.softAPIP());
  }
  Serial.print("Horta IoT online em http://");
  Serial.println(localNetwork ? WiFi.softAPIP() : WiFi.localIP());
  setupRoutes();
  server.begin();
  logEvent("sistema pronto");
}

void loop() {
  server.handleClient();
  readRfid();
  readEnvironmentalSensors();
  evaluateAutomaticMode();

  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastButtonAt > 300) {
    lastButtonAt = millis();
    if (mode == MANUAL) setIrrigation(!irrigation, "botao fisico");
    else logEvent("botao ignorado fora do modo manual");
  }
  delay(10);
}
