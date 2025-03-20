#include <esp_now.h>
#include <WiFi.h>

String tempo;
String forca;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  String tempo;
  String forca;
} struct_message;

// Create a struct_message to receive data.
struct_message receive_Data;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  memcpy(&receive_Data, incomingData, sizeof(receive_Data));

  // Serial.println();
  // Serial.println("Receive Data:");
  // Serial.print("Bytes received: ");
  // Serial.println(len);

  tempo = receive_Data.tempo;
  forca = receive_Data.forca;

  String informations = tempo + "s" + "," + forca + "N";
  Serial.println(informations);
}

void setup() {
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  
}