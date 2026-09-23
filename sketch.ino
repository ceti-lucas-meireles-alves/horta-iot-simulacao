#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

constexpr uint8_t RFID_SS = 5;
constexpr uint8_t RFID_RST = 22;
constexpr uint8_t SERVO_PIN = 13;
constexpr uint8_t MOISTURE_PIN = 34;
constexpr uint8_t GREEN_LED = 26;
constexpr uint8_t RED_LED = 27;
constexpr uint8_t BUTTON_PIN = 25;

MFRC522 rfid(RFID_SS, RFID_RST);
Servo valve;
WebServer server(80);

bool authorized = false;
bool irrigation = false;
String lastUser = "nenhum";
unsigned long lastEvent = 0;

int moisturePercent() {
  int raw = analogRead(MOISTURE_PIN);
  return map(raw, 0, 4095, 0, 100);
}

void setIrrigation(bool enabled) {
  irrigation = enabled && authorized;
  valve.write(irrigation ? 90 : 0);
  digitalWrite(GREEN_LED, irrigation ? HIGH : LOW);
  digitalWrite(RED_LED, irrigation ? LOW : HIGH);
  lastEvent = millis();
}

String html() {
  int moisture = moisturePercent();
  String state = irrigation ? "IRRIGANDO" : "AGUARDANDO";
  String access = authorized ? "autorizado" : "não autorizado";
  String page = "<!doctype html><html lang='pt-BR'><meta charset='utf-8'>";
  page += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  page += "<title>Horta IoT</title><style>body{font-family:Arial;max-width:720px;margin:2rem auto;padding:0 1rem;color:#163020} .card{padding:1rem;margin:1rem 0;border-radius:12px;background:#e8f5e9}button{padding:.7rem 1rem;margin:.3rem;border:0;border-radius:8px;background:#2e7d32;color:#fff}</style>";
  page += "<h1>Horta IoT Escolar</h1><div class='card'><h2>Status</h2>";
  page += "<p><b>Umidade simulada:</b> " + String(moisture) + "%</p>";
  page += "<p><b>Acesso RFID:</b> " + access + "</p>";
  page += "<p><b>Último usuário:</b> " + lastUser + "</p>";
  page += "<p><b>Sistema:</b> " + state + "</p></div>";
  page += "<p><a href='/irrigar'><button>Iniciar irrigação</button></a><a href='/parar'><button>Parar</button></a></p>";
  page += "<p>O potenciômetro representa o sensor de umidade. O servo representa uma válvula ou bomba.</p></html>";
  return page;
}

void setupRoutes() {
  server.on("/", []() { server.send(200, "text/html", html()); });
  server.on("/irrigar", []() { setIrrigation(true); server.sendHeader("Location", "/"); server.send(302); });
  server.on("/parar", []() { setIrrigation(false); server.sendHeader("Location", "/"); server.send(302); });
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  valve.attach(SERVO_PIN);
  valve.write(0);
  digitalWrite(RED_LED, HIGH);

  SPI.begin();
  rfid.PCD_Init();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  while (WiFi.status() != WL_CONNECTED) delay(100);
  Serial.print("Horta IoT online em http://");
  Serial.println(WiFi.localIP());
  setupRoutes();
  server.begin();
}

void loop() {
  server.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) {
    setIrrigation(!irrigation);
    delay(250);
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  String uid;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  authorized = true;
  lastUser = "cartão " + uid;
  Serial.println("RFID autorizado: " + uid);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
