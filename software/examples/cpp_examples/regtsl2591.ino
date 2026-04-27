/*
 * Diagnóstico a nivel de registro — Pin INT TSL2591
 * Bypassa completamente la librería Adafruit
 * Confirma si el problema es hardware o software
 */

#include <Wire.h>
#define SDA_PIN 6
#define SCL_PIN 7
#define INT_PIN 4
#define TSL2591_ADDR   0x29
#define CMD_NORMAL     0xA0
#define CMD_CLEAR_INT  0xE6

#define REG_ENABLE     0x00
#define REG_CONFIG     0x01
#define REG_AILTL      0x04
#define REG_AILTH      0x05
#define REG_AIHTL      0x06
#define REG_AIHTH      0x07
#define REG_PERSIST    0x0C
#define REG_STATUS     0x13
#define REG_C0DATAL    0x14

// ──────────────────────────────────────────
// PRIMITIVAS DE REGISTRO
// ──────────────────────────────────────────

void reg_write(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(TSL2591_ADDR);
  Wire.write(CMD_NORMAL | reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t reg_read(uint8_t reg) {
  Wire.beginTransmission(TSL2591_ADDR);
  Wire.write(CMD_NORMAL | reg);
  Wire.endTransmission(false);
  Wire.requestFrom(TSL2591_ADDR, 1);
  return Wire.available() ? Wire.read() : 0xFF;
}

uint16_t reg_read16(uint8_t reg) {
  uint8_t lo = reg_read(reg);
  uint8_t hi = reg_read(reg + 1);
  return ((uint16_t)hi << 8) | lo;
}

void reg_clear_interrupt() {
  Wire.beginTransmission(TSL2591_ADDR);
  Wire.write(CMD_CLEAR_INT);
  Wire.endTransmission();
}

// ──────────────────────────────────────────
// DIAGNÓSTICO
// ──────────────────────────────────────────

void diagnosticoRegistros() {
  Serial.println(F("══════════════════════════════════════"));
  Serial.println(F("  DIAGNÓSTICO A NIVEL DE REGISTRO"));
  Serial.println(F("══════════════════════════════════════"));

  // PASO 1 — Encender sensor con AIEN habilitado manualmente
  // ENABLE = PON(0x01) | AEN(0x02) | AIEN(0x10) = 0x13
  reg_write(REG_ENABLE, 0x13);
  delay(50);

  uint8_t enable_leido = reg_read(REG_ENABLE);
  Serial.print(F("  ENABLE escrito: 0x13  |  ENABLE leido: 0x"));
  Serial.print(enable_leido, HEX);
  Serial.println(enable_leido == 0x13 ? F("  ✅") : F("  ❌ no coincide"));

  // PASO 2 — Configurar GAIN=MED, ATIME=100ms
  // CONFIG = GAIN_MED(0x10) | ATIME_100MS(0x00) = 0x10
  reg_write(REG_CONFIG, 0x10);
  delay(50);

  uint8_t config_leido = reg_read(REG_CONFIG);
  Serial.print(F("  CONFIG escrito: 0x10  |  CONFIG leido: 0x"));
  Serial.println(config_leido, HEX);

  // PASO 3 — Escribir umbrales directamente en registros
  // Umbral bajo = 100  (0x0064)
  // Umbral alto = 4000 (0x0FA0)
  uint16_t umbral_bajo = 100;
  uint16_t umbral_alto = 500;

  reg_write(REG_AILTL, umbral_bajo & 0xFF);
  reg_write(REG_AILTH, umbral_bajo >> 8);
  reg_write(REG_AIHTL, umbral_alto & 0xFF);
  reg_write(REG_AIHTH, umbral_alto >> 8);

  // Persistencia = 1 ciclo
  reg_write(REG_PERSIST, 0x01);

  // Verificar que los umbrales se escribieron correctamente
  uint16_t bajo_leido = ((uint16_t)reg_read(REG_AILTH) << 8) | reg_read(REG_AILTL);
  uint16_t alto_leido = ((uint16_t)reg_read(REG_AIHTH) << 8) | reg_read(REG_AIHTL);

  Serial.print(F("  Umbral bajo escrito: "));
  Serial.print(umbral_bajo);
  Serial.print(F("  leido: "));
  Serial.println(bajo_leido);

  Serial.print(F("  Umbral alto escrito: "));
  Serial.print(umbral_alto);
  Serial.print(F("  leido: "));
  Serial.println(alto_leido);

  delay(150);

  // PASO 4 — Leer FULL y STATUS
  uint16_t full = reg_read16(REG_C0DATAL);
  uint8_t  status = reg_read(REG_STATUS);
  int      pin    = digitalRead(INT_PIN);

  Serial.println(F("──────────────────────────────────────"));
  Serial.print(F("  FULL (CH0):  ")); Serial.println(full);
  Serial.print(F("  STATUS:      0x")); Serial.println(status, HEX);
  Serial.print(F("  AINT bit:    ")); Serial.println((status & 0x10) ? F("1 ✅ interrupcion activa en registro") : F("0 ❌ no activa"));
  Serial.print(F("  PIN INT:     ")); Serial.println(pin == LOW ? F("LOW ✅ hardware OK") : F("HIGH ❌ problema hardware"));
  Serial.println(F("──────────────────────────────────────"));

  // PASO 5 — Conclusión
  bool aint  = (status & 0x10) != 0;
  bool pin_ok = (pin == LOW);

  if (aint && pin_ok) {
    Serial.println(F("  RESULTADO: ✅ Todo funciona — registro Y hardware OK"));
  } else if (aint && !pin_ok) {
    Serial.println(F("  RESULTADO: ⚠️  PROBLEMA HARDWARE CONFIRMADO"));
    Serial.println(F("             Registro dispara pero pin no baja"));
    Serial.println(F("             → Falta pull-up externo 4.7k en INT"));
  } else if (!aint && !pin_ok) {
    Serial.println(F("  RESULTADO: ❌ AINT no dispara — revisa umbrales vs FULL"));
  } else {
    Serial.println(F("  RESULTADO: ❌ Pin bajo sin AINT — revisar hardware"));
  }

  reg_clear_interrupt();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  pinMode(INT_PIN, INPUT_PULLUP);

  diagnosticoRegistros();
}

void loop() {
  // Corre el diagnóstico cada 3 segundos
  delay(3000);
  diagnosticoRegistros();
}