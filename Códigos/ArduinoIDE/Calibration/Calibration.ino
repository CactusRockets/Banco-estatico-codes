// Biblioteca para Leitura da Celula de Carga
#include <HX711.h>

//------------------------------------------------------------------------------------
// Definindo pinos I/O 
//------------------------------------------------------------------------------------
#define       LedBoard     12      // WIFI Module LED
#define       DT           15      // DT HX711
#define       SCK          2       // SCK HX711

#define Diferenca_maxima 50
#define Diferenca_media 10
#define Diferenca_minima 5

#define Reajuste_maximo 300
#define Reajuste_medio 100
#define Reajuste_minimo 50

#define massa_conhecida 18.5
#define fator_calibracao_inicial -50000

#define USE_STORAGE true

//------------------------------------------------------------------------------------
// Declarando Variaveis
//------------------------------------------------------------------------------------
float peso;
String informations;

// Declaracao do objeto ESCALA na classe HX711 da biblioteca
HX711 escala;

int FATOR_CALIBRACAO = fator_calibracao_inicial;

// Aqui deve ser colocado o valor do peso conhecido de algum objeto para a calibração
// Em Newtons (9.8m/s^2)
float peso_conhecido = massa_conhecida * 9.8;

int contador = 0;

/* CONFIGURAÇÕES CARTÃO MICROSD */
#include <SPI.h>
#include <SD.h>

#define CS_SDPIN 4

File myFile;

void setupSd() {
  Serial.println("Inicializando o cartão SD...");
  // verifica se o cartão SD está presente e se pode ser inicializado
  
  while(!SD.begin(CS_SDPIN)) {
    // programa encerrado 
    Serial.println("Falha, verifique se o cartão está presente.");
  }

  // Cria arquivo data.txt e abre
  myFile = SD.open("/data.txt", FILE_APPEND);
  // Escreve dados no arquivo
  if (myFile) {
    Serial.println("Gravando dados iniciais!");
    myFile.println("Peso real, Peso lido, Fator de calibração");
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
  digitalWrite(LedBoard, HIGH);
            
  Serial.begin(115200);
  Serial.println("Modulo Incializado");

  Serial.println("Inicializando e calibrando Celula de carga");  
  // Inicializacao e definicao dos pinos DT e SCK dentro do objeto ESCALA
  escala.begin(DT, SCK);
  // Tara a balança
  escala.tare();
  // Ajusta a escala para o fator de calibração
  escala.set_scale(FATOR_CALIBRACAO);

  // Coloque o objeto na balança até o final do tempo
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
}
//====================================================================================

void loop() {
  digitalWrite(LedBoard, LOW);
  
  if (escala.is_ready()) {
    peso = (escala.get_units());
    informations = 
      String(peso_conhecido) + "N" + "," + String(peso, 3) + "N" + "," + String(FATOR_CALIBRACAO);
    Serial.println(informations);
      
    if(USE_STORAGE) {
      writeOnSD(informations);
    }

    // Reajuste na calibração
    if((peso - peso_conhecido) > Diferenca_maxima) {
      FATOR_CALIBRACAO += Reajuste_maximo;
      
    } else if((peso - peso_conhecido) > Diferenca_media) {
      FATOR_CALIBRACAO += Reajuste_medio;
      
    }  else if((peso - peso_conhecido) > Diferenca_minima) {
      FATOR_CALIBRACAO += Reajuste_minimo;



    } else if((peso - peso_conhecido) < -Diferenca_maxima) {
      FATOR_CALIBRACAO -= Reajuste_maximo;
      
    } else if((peso - peso_conhecido) < -Diferenca_media) {
      FATOR_CALIBRACAO -= Reajuste_medio;
      
    } else if((peso - peso_conhecido) < -Diferenca_minima) {
      FATOR_CALIBRACAO -= Reajuste_minimo;
      
      
    } else {
      while(1) {
        Serial.println("Fator de calibração ideal:" + String(FATOR_CALIBRACAO));
        delay(500);
      }
    }

    escala.set_scale(FATOR_CALIBRACAO);
      

    // HX711 Requer que cada leitura seja realizado a no minimo a cada 100ms
    // 120ms é o ideal para não ocorrer travamentos durante as leituras sem cartão SD
    // No caso da utilização do cartão sd verificar a quanto tempo está realizando as leituras
    // E inserir um delay para que as leituras fiquem acima de 100ms 
    delay(120);
    
  } else {
    Serial.println("Erro de Leitura");
  }   
  digitalWrite(LedBoard, HIGH);       
}
