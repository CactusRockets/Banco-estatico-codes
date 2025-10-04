#include <HX711.h>

#define DT 15
#define SCK 2
#define LedBoard 12

HX711 escala;

// parâmetros
const float massa_known_kg = 18.85;
const float g = 9.8;
const float força_known = massa_known_kg * g; // N

// número de amostras para média
const int N_AVG = 200;  // pode aumentar, porque agora temos 80 SPS

// Função para média das leituras RAW (sem delay extra)
long readAverage(int n) {
  long sum = 0;
  for (int i = 0; i < n; i++) {
    sum += escala.read(); // leitura bruta (counts)
  }
  return sum / n;
}

void twoPointCalibrate() {
  Serial.println(">> Remova toda carga da celula e aguarde...");
  delay(3000); // tempo para estabilizar
  escala.tare(); // zera (ajusta offset interno)
  delay(500);

  // Leitura no zero
  Serial.println("Lendo ZERO (media)...");
  long raw_zero = readAverage(N_AVG);
  Serial.print("raw_zero = "); Serial.println(raw_zero);

  // Leitura com peso conhecido
  Serial.println("Coloque o peso conhecido de " + String(massa_known_kg) + " kg e aguarde...");
  delay(5000);
  Serial.println("Lendo COM PESO (media)...");
  long raw_known = readAverage(N_AVG);
  Serial.print("raw_known = "); Serial.println(raw_known);

  // Cálculo do fator (slope)
  float slope = (float)(raw_known - raw_zero) / força_known; // counts por Newton
  Serial.print("slope (counts por N) = "); Serial.println(slope, 6);

  // Aplica na HX711
  escala.set_scale(slope); // agora get_units() retorna em Newtons
  escala.set_offset(raw_zero); // garante que no zero fica próximo de 0

  Serial.println("Aplicado set_scale(slope) e set_offset(raw_zero). Testando leitura...");

  // Teste rápido
  delay(500);
  float leitura = escala.get_units(20); // média de 20 amostras
  Serial.print("Leitura com escala aplicada (N): "); Serial.println(leitura, 3);

  Serial.println("\nUse no seu transmitter:");
  Serial.print("escala.set_scale("); Serial.print(slope, 6); Serial.println(");");
  Serial.print("escala.set_offset("); Serial.print(raw_zero); Serial.println(");");
}

void setup() {
  Serial.begin(115200);
  pinMode(LedBoard, OUTPUT);
  digitalWrite(LedBoard, HIGH);

  escala.begin(DT, SCK);

  twoPointCalibrate();

  digitalWrite(LedBoard, LOW);
}

void loop() {
  // Testando leituras em tempo real já calibradas
  float pesoN = escala.get_units(10); // média de 10 leituras
  Serial.print("Peso (N): ");
  Serial.println(pesoN, 3);
  delay(100); // com 80 SPS, 100 ms = 8 leituras aprox.
}
