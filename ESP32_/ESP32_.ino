// Aula 11 - ESP32 DevKit, LED comum no GPIO 2, USB serial 115200.
// Periodo = ciclo completo. B inicia, P para, T:100..2000 ajusta, S consulta.
#include <Arduino.h>

const uint8_t PINO_LED = 2;
const bool LED_ATIVO_EM_HIGH = true; // Mude apenas se sua placa usar logica invertida.
unsigned long periodoMs = 1000;
unsigned long ultimaTroca = 0;
unsigned long ultimoStatus = 0;
bool piscando = true;
bool aceso = false;
char entrada[40];
size_t tamanho = 0;
bool descartando = false;

void informarEstado() {
  Serial.print("ESTADO:");
  Serial.println(aceso ? 1 : 0);
}

void aplicarLed(bool ligado) {
  aceso = ligado;
  digitalWrite(PINO_LED, (ligado == LED_ATIVO_EM_HIGH) ? HIGH : LOW);
  informarEstado();
}

void informarStatus() {
  Serial.println("PLACA:ESP32_GPIO2");
  Serial.print("INTERVALO:");
  Serial.println(periodoMs);
  Serial.println(piscando ? "LED:PISCANDO" : "LED:PARADO");
  informarEstado();
}

void processarComando(const char* comando) {
  if (strcmp(comando, "B") == 0) {
    piscando = true;
    ultimaTroca = millis();
    informarStatus();
  } else if (strcmp(comando, "P") == 0) {
    piscando = false;
    aplicarLed(false);
    informarStatus();
  } else if (strcmp(comando, "S") == 0) {
    informarStatus();
  } else if (strncmp(comando, "T:", 2) == 0) {
    // Validacao completa: nao aceitar T:500abc, valores negativos ou excessivos.
    const char* digito = comando + 2;
    unsigned long valor = 0;
    bool valido = *digito != '\0';
    while (*digito && valido) {
      if (*digito < '0' || *digito > '9') { valido = false; break; }
      valor = valor * 10 + (*digito - '0');
      if (valor > 2000) { valido = false; break; }
      ++digito;
    }
    if (valido && valor >= 100 && valor <= 2000) {
      periodoMs = valor;
      ultimaTroca = millis();
      informarStatus();
    } else Serial.println("ERRO:INTERVALO");
  } else if (*comando) Serial.println("ERRO:COMANDO");
}

void setup() {
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LED_ATIVO_EM_HIGH ? LOW : HIGH);
  Serial.begin(115200);
  ultimaTroca = millis();
  Serial.println("PRONTO");
  informarStatus();
}

void loop() {
  // Limite por iteracao para a entrada serial nao bloquear o pisca.
  for (int i = 0; i < 64 && Serial.available() > 0; ++i) {
    char c = Serial.read();
    if (c == '\n') {
      if (descartando) Serial.println("ERRO:TAMANHO");
      else { entrada[tamanho] = '\0'; processarComando(entrada); }
      tamanho = 0;
      descartando = false;
    } else if (c != '\r' && !descartando) {
      if (tamanho < sizeof(entrada) - 1) entrada[tamanho++] = c;
      else descartando = true;
    }
  }
  unsigned long agora = millis();
  if (piscando && agora - ultimaTroca >= periodoMs / 2) {
    ultimaTroca = agora;
    aplicarLed(!aceso);
  }
  // Continua informando estado mesmo parado e recupera mensagens iniciais perdidas.
  if (agora - ultimoStatus >= 1000) {
    ultimoStatus = agora;
    informarStatus();
  }
}
