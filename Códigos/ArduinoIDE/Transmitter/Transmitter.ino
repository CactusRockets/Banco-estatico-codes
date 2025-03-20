#include <HX711.h>
#include <SPI.h>
#include <SD.h>

// Biblioteca do ESP-NOW
#include <esp_now.h>
#include <WiFi.h>

//------------------------------------------------------------------------------------
// Definindo pinos I/O 
//------------------------------------------------------------------------------------
#define       pino_led     5
#define       LedBoard     12      // Led de indicação de funcionamento
#define       DT           15      // DT HX711
#define       SCK          2       // SCK HX711
#define       CS_SDPIN     4       // CS Cartão SD

#define FATOR_CALIBRACAO -8000

// #define FATOR_CALIBRACAO -50000

#define USE_STORAGE true
#define USE_WIFI true

//------------------------------------------------------------------------------------
// Declarando Variaveis
//------------------------------------------------------------------------------------
float force;
double timeMillis;
String informations;

// Declaracao do objeto ESCALA na classe HX711 da biblioteca
HX711 escala;

File myFile;

// 30:C9:22:39:10:9C
uint8_t broadcastAddress[] = { 0x30, 0xC9, 0x22, 0x39, 0x10, 0x9C };

// Estrutura para envio dos dados. Deve ser a mesma tanto no emissor como no receptor.
typedef struct struct_message
{
  String tempo;
  String forca;
} struct_message;

// Cria uma struct_message chamada myData
struct_message myData;
// Variable to store if sending data was successful.
String success;

// Função de Callback executada quando a mensagem for recebida
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  digitalWrite(pino_led, LOW);

  Serial.println();
  Serial.print("Bytes recebidos: ");
  Serial.println(len);
  Serial.print("Tempo: ");
  Serial.print(myData.tempo);
  Serial.print(",Força: ");
  Serial.println(myData.forca);

  digitalWrite(pino_led, HIGH);
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  if (status == 0){
    success = "Delivery Success!";
  }
  else{
    success = "Delivery Fail!";
  }
  Serial.println(">>>>>");
}

void setupESPNOW() {
    // Coloca o dispositivo no modo Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Inicializa o ESP-NOW
  if (esp_now_init() != ESP_OK) {
    while(1) {
      Serial.println("Erro ao inicializar o ESP-NOW");
      errorInfo();
    }
  }
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    while(1) {
      Serial.println("Failed to add peer");
      errorInfo();
    }
  }

  successInfo();
}

void successInfo() {
  digitalWrite(LedBoard, HIGH);
}

void errorInfo() {
  digitalWrite(LedBoard, HIGH);
  delay(250);
  digitalWrite(LedBoard, LOW);
  delay(250);
}

void setupSd() {
  Serial.println("Inicializando o cartão SD...");
  // verifica se o cartão SD está presente e se pode ser inicializado
  
  while(!SD.begin(CS_SDPIN)) {
    // programa encerrado 
    Serial.println("Falha, verifique se o cartão está presente.");
    errorInfo();  
  }

  // Cria arquivo data.txt e abre
  myFile = SD.open("/data.txt", FILE_APPEND);
  // Escreve dados no arquivo
  if (myFile) {
    successInfo();

    Serial.println("Gravando dados iniciais!");
    myFile.println("Tempo, Força, Fator de calibração");
    myFile.close();

  } else {
    Serial.println("Error ao abrir data.txt");
  }
}

void writeOnSD(String str) {
  myFile = SD.open("/data.txt", FILE_APPEND);

  if (myFile) {
    Serial.println("(Dados Gravados!)");
    myFile.println(str);
    myFile.close();

  } else {
    Serial.println("Error ao gravar em data.txt");
  }
}

void setup() {
  pinMode(LedBoard, OUTPUT);  
  digitalWrite(LedBoard, LOW);
            
  Serial.begin(115200);
  Serial.println("Modulo Incializado");

  Serial.println("Inicializando e calibrando Celula de carga");  
  // Inicializacao e definicao dos pinos DT e SCK dentro do objeto ESCALA
  escala.begin(DT, SCK);
  // Tara a balança
  escala.tare();
  // Ajusta a escala para o fator de calibracao
  escala.set_scale(FATOR_CALIBRACAO);

  if(USE_STORAGE) {
    setupSd();
  }
  if(USE_WIFI) {
    setupESPNOW();
  }
}


void loop() {
  
  if (escala.is_ready()) {
    force = (escala.get_units()) * 9.8;
    timeMillis = millis();
    informations = String(timeMillis) + "," + String(force, 3) + "," + String(FATOR_CALIBRACAO);

    Serial.println(informations);
    if(USE_STORAGE) {
      writeOnSD(informations);
    }
    if(USE_WIFI) {
      // Send message via ESP-NOW
      myData.tempo = String(timeMillis);
      myData.forca = String(force, 3);

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
    }

    // HX711 Requer que cada leitura seja realizado a no minimo a cada 100ms
    // 120ms é o ideal para não ocorrer travamentos durante as leituras sem cartão SD
    // No caso da utilização do cartão sd verificar a quanto tempo está realizando as leituras
    // E inserir um delay para que as leituras fiquem acima de 100ms 
    delay(120);
    
  } else {
    Serial.println("Erro de Leitura");
  }   
}
