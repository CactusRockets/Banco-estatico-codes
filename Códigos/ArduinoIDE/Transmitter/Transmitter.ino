// ------------------------------------------------------------------------------------
// Bibliotecas
// ------------------------------------------------------------------------------------
#include <HX711.h>
#include <SPI.h>
#include <SD.h>
#include <esp_now.h>
#include <WiFi.h>

// ------------------------------------------------------------------------------------
// Definindo pinos I/O 
// ------------------------------------------------------------------------------------
#define pino_led        5
#define LedBoard        32
#define DT              15
#define SCK             2
#define CS_SDPIN        4

#define USE_STORAGE true
#define USE_WIFI    true

// ------------------------------------------------------------------------------------
// Parâmetros de calibração (obtidos pelo código de calibração)
#define FATOR_CALIBRACAO -227.847488
#define OFFSET_HX711     -12309

// ------------------------------------------------------------------------------------
// Variáveis globais
// ------------------------------------------------------------------------------------
HX711 escala;
File myFile;
float force;
double timeMillis;
String informations;

// MAC do receptor (substitua pelo seu)
uint8_t broadcastAddress[] = { 0xF4, 0x65, 0x0B, 0xE7, 0x1F, 0x80 };

// Estrutura para envio via ESP-NOW
typedef struct struct_message {
  String tempo;
  String forca;
} struct_message;

struct_message myData;
String success;

// ------------------------------------------------------------------------------------
// Funções auxiliares
// ------------------------------------------------------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  success = (status == ESP_NOW_SEND_SUCCESS) ? "Delivery Success!" : "Delivery Fail!";
}

void setupESPNOW() {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    while(true);
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_del_peer(broadcastAddress);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  } else {
    Serial.println("Peer adicionado com sucesso!");
  }
}

void successInfo() { digitalWrite(LedBoard, HIGH); }
void errorInfo()   {
  digitalWrite(LedBoard, HIGH);
  delay(250);
  digitalWrite(LedBoard, LOW);
  delay(250);
}

// ------------------------------------------------------------------------------------
// SD
// ------------------------------------------------------------------------------------
void setupSd() {
  Serial.println("Inicializando o cartão SD...");
  while(!SD.begin(CS_SDPIN)) {
    Serial.println("Falha, verifique se o cartão está presente.");
    errorInfo();
  }

  myFile = SD.open("/data.txt", FILE_APPEND);
  if (myFile) {
    successInfo();
    Serial.println("Gravando cabeçalho de dados!");
    myFile.println("Tempo(ms),Forca(N),Fator,Offset");
    myFile.close();
  } else {
    Serial.println("Erro ao abrir data.txt");
  }
}

void writeOnSD(String str) {
  myFile = SD.open("/data.txt", FILE_APPEND);
  if (myFile) {
    myFile.println(str);
    myFile.close();
  } else {
    Serial.println("Erro ao gravar em data.txt");
  }
}

// ------------------------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------------------------
void setup() {
  pinMode(LedBoard, OUTPUT);
  digitalWrite(LedBoard, HIGH);

  Serial.begin(115200);
  Serial.println("Transmitter Inicializado");

  // HX711
  escala.begin(DT, SCK);
  escala.set_scale(FATOR_CALIBRACAO);
  escala.set_offset(OFFSET_HX711);

  if (USE_STORAGE) setupSd();
  if (USE_WIFI)    setupESPNOW();
}

// ------------------------------------------------------------------------------------
// Loop principal (sem is_ready)
// ------------------------------------------------------------------------------------
void loop() {
  // leitura contínua (sem esperar is_ready)
  force = escala.get_units(1);  // 1 leitura (mais rápido que média de 10)
  timeMillis = millis();

  informations = String(timeMillis) + "," + String(force, 3) + "," + String(FATOR_CALIBRACAO,6) + "," + String(OFFSET_HX711);
  Serial.println(informations);

  if (USE_STORAGE) writeOnSD(informations);

  if (USE_WIFI) {
    myData.tempo = String(timeMillis);
    myData.forca = String(force, 3);
    esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  }

  // ajuste o delay conforme necessário
  delay(2); // delay mínimo, não sobrecarrega o loop
}
