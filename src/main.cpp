#include <Arduino.h>
#include <DHT.h>

#define PINO_DHT     4
#define TIPO_DHT     DHT22
#define PINO_LDR     34
#define PINO_RELE    26
#define LED_VERDE    32
#define LED_VERMELHO 33

#define BTN_N  12   // Nitrogênio
#define BTN_P  14   // Fósforo
#define BTN_K  27   // Potássio

DHT dht(PINO_DHT, TIPO_DHT);


bool estadoN_ok = true;
bool estadoP_ok = true;
bool estadoK_ok = true;


bool ultimoN = HIGH, ultimoP = HIGH, ultimoK = HIGH;
unsigned long ultimoDebounce = 0;
const unsigned long debounceDelay = 50; 

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(PINO_LDR, INPUT);
  pinMode(PINO_RELE, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  pinMode(BTN_N, INPUT_PULLUP);
  pinMode(BTN_P, INPUT_PULLUP);
  pinMode(BTN_K, INPUT_PULLUP);

  digitalWrite(PINO_RELE, HIGH);  // desligado
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);

  Serial.println("🌱 FarmTech - Sistema de Irrigacao Inteligente");
  Serial.println("Botões: N(12), P(14), K(27)");
  Serial.println("------------------------------------------------------");
}

void loop() {
  bool leituraN = digitalRead(BTN_N);
  bool leituraP = digitalRead(BTN_P);
  bool leituraK = digitalRead(BTN_K);
  unsigned long agora = millis();

  if (agora - ultimoDebounce > debounceDelay) {
    if (leituraN == LOW && ultimoN == HIGH) {
      estadoN_ok = !estadoN_ok;
      Serial.print("Toggle N -> "); Serial.println(estadoN_ok ? "OK" : "DEFICIENTE");
      ultimoDebounce = agora;
    }
    if (leituraP == LOW && ultimoP == HIGH) {
      estadoP_ok = !estadoP_ok;
      Serial.print("Toggle P -> "); Serial.println(estadoP_ok ? "OK" : "DEFICIENTE");
      ultimoDebounce = agora;
    }
    if (leituraK == LOW && ultimoK == HIGH) {
      estadoK_ok = !estadoK_ok;
      Serial.print("Toggle K -> "); Serial.println(estadoK_ok ? "OK" : "DEFICIENTE");
      ultimoDebounce = agora;
    }
  }
  ultimoN = leituraN; ultimoP = leituraP; ultimoK = leituraK;

  
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int ldrRaw = analogRead(PINO_LDR);

  
  float ph_simulado = map(ldrRaw, 0, 4095, 4, 9);

  bool dht_ok = !(isnan(temperatura) || isnan(umidade));
  if (!dht_ok) {
    Serial.println("⚠️ Erro lendo DHT22 (NaN). Ignorando leitura.");
    umidade = 60;
  }

  Serial.println("------------------------------------------------");
  Serial.printf("Temp: %.1f°C  |  Umid: %.1f%%  |  pH: %.2f\n", temperatura, umidade, ph_simulado);
  Serial.printf("N: %s | P: %s | K: %s\n",
                estadoN_ok ? "OK" : "DEF",
                estadoP_ok ? "OK" : "DEF",
                estadoK_ok ? "OK" : "DEF");

  
  bool umidade_baixa = (umidade < 50.0);
  bool npk_def = (!estadoN_ok) || (!estadoP_ok) || (!estadoK_ok);
  bool ph_fora = (ph_simulado < 5.5) || (ph_simulado > 7.5);

  bool acionar_irrigacao = (umidade_baixa || npk_def || ph_fora);

  if (acionar_irrigacao) {
    digitalWrite(PINO_RELE, LOW);   // Liga
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
    Serial.println("💧 Irrigacao: ATIVADA");
    if (umidade_baixa) Serial.println(" - motivo: solo seco");
    if (npk_def) {
      if (!estadoN_ok) Serial.println(" - Nitrogenio DEFICIENTE");
      if (!estadoP_ok) Serial.println(" - Fosforo DEFICIENTE");
      if (!estadoK_ok) Serial.println(" - Potassio DEFICIENTE");
    }
    if (ph_fora) Serial.println(" - pH fora da faixa ideal (5.5–7.5)");
  } else {
    digitalWrite(PINO_RELE, HIGH);  // Desliga
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    Serial.println("✅ Irrigacao: DESATIVADA - condicoes adequadas");
  }

  delay(2000);
}
