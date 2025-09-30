
// Biblioteca para Leitura da Celula de Carga
#include <HX711.h>
#include <PID_v1.h>

//------------------------------------------------------------------------------------
// Definindo pinos I/O 
//------------------------------------------------------------------------------------
#define LedBoard 12
#define DT       15
#define SCK      2

#define massa_conhecida 2
#define fator_calibracao_inicial -10000

#define USE_STORAGE true

//------------------------------------------------------------------------------------
// Variáveis globais
//------------------------------------------------------------------------------------
float peso;
String informations;

HX711 escala;

double input, output, setpoint;
// Ganhos PID (ajustar experimentalmente)
double Kp = 500, Ki = 50, Kd = 0;  

int FATOR_CALIBRACAO = fator_calibracao_inicial;

// Aqui deve ser colocado o valor do peso conhecido de algum objeto para a calibração
// Em Newtons (9.8m/s^2)
float peso_conhecido = massa_conhecida * 9.8;

PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

#include <SPI.h>
#include <SD.h>
#define CS_SDPIN 4
File myFile;

void setupSd() {
  Serial.println("Inicializando o cartão SD...");
  while(!SD.begin(CS_SDPIN)) {
    Serial.println("Falha, verifique se o cartão está presente.");
  }

  myFile = SD.open("/data.txt", FILE_APPEND);
  if (myFile) {
    Serial.println("Gravando dados iniciais!");
    myFile.println("Peso real, Peso lido, Fator de calibração");
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

void setup() {
  pinMode(LedBoard, OUTPUT);  
  digitalWrite(LedBoard, HIGH);

  Serial.begin(115200);
  Serial.println("Modulo Inicializado");

  escala.begin(DT, SCK);
  escala.tare();
  escala.set_scale(FATOR_CALIBRACAO);

  Serial.println("Coloque o objeto na balança até o final do tempo:");
  int tempo = 10;
  while(tempo > 0) {
    Serial.println(String(tempo) + "...");
    delay(1000);
    tempo -= 1;
  }
  Serial.println("\n");

  if(USE_STORAGE) {
    setupSd();
  }

  // Configuração do PID
  setpoint = peso_conhecido;
  myPID.SetMode(AUTOMATIC);
  // tempo em ms
  myPID.SetSampleTime(200);  
  // limitar correções no fator de calibração
  myPID.SetOutputLimits(-1000, 1000); 
}

void loop() {
  digitalWrite(LedBoard, LOW);

  if (escala.is_ready()) {
    peso = escala.get_units();

    // Atualizar entradas do PID
    input = peso;
    // calcula saída do PID
    myPID.Compute();   
    // aplica correção
    FATOR_CALIBRACAO += (int)output;   
    escala.set_scale(FATOR_CALIBRACAO);

    informations =
      String(peso_conhecido) + "N," + String(peso, 3) + "N," + String(FATOR_CALIBRACAO);
    Serial.println(informations);

    if(USE_STORAGE) {
      writeOnSD(informations);
    }

    // Pequena pausa entre leituras
    delay(200);
    
  } else {
    Serial.println("Erro de Leitura");
  }   

  digitalWrite(LedBoard, HIGH);
}
