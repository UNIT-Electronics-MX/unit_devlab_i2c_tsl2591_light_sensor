/*
****************************************************
*ISSUE INTERRUPT
****************************************************
*/
#include <Devlab_TSL2591.h>
#include <Wire.h>
#define SDA_PIN 6
#define SCL_PIN 7
#define INT_PIN 17

Adafruit_TSL2591 tsl = Adafruit_TSL2591(1);

void initDevice(){
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║  Iniciando ID del TSL2591            ║");
  Serial.println("╚══════════════════════════════════════╝");
  
  //Inicializamos el dispositivo para verificar si hay transmision
  bool found = tsl.begin();
  if(found){
    Serial.println("Dispositivo encontrado en la dirección 0x29");
  }else{
    Serial.println("Dispositivo no encontrado");
  }

}

void advancedRead(void)
{
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
  Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);
}

uint16_t readFull(){

  uint16_t full;
  uint32_t lum = tsl.getFullLuminosity();
  full = lum & 0xFFFF;

  Serial.print(F("Full: ")); Serial.print(full); Serial.println(F("  "));
  
  return full;
}
void getStatus(void)
{
  uint8_t x = tsl.getStatus();
  // bit 4: ALS Interrupt occured
  // bit 5: No-persist Interrupt occurence
  if (x & 0x10) {
    Serial.print("[ "); Serial.print(millis()); Serial.print(" ms ] ");
    Serial.println("ALS Interrupt occured");
  }
  if (x & 0x20) {
    Serial.print("[ "); Serial.print(millis()); Serial.print(" ms ] ");
    Serial.println("No-persist Interrupt occured");
  }

  // Serial.print("[ "); Serial.print(millis()); Serial.print(" ms ] ");
  Serial.print("Status: ");
  Serial.println(x, BIN);
  tsl.clearInterrupt();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN,SCL_PIN);
  Wire.setClock(400000);
  initDevice();
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
  tsl.registerInterrupt(500,3300,TSL2591_PERSIST_10);

  pinMode(INT_PIN,INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  //advancedRead();
  uint16_t full = readFull();

Serial.print("FULL: ");
Serial.print(full);
Serial.print("  STATUS: 0x");
Serial.print(tsl.getStatus(), HEX);
Serial.print("  PIN: ");
Serial.println(digitalRead(INT_PIN));
if(digitalRead(INT_PIN) == LOW){
    if(tsl.getStatus() & 0x10){
        if(full < 100){
            Serial.println("Luz baja — sensor en oscuridad");
        } else if(full > 600){
            Serial.println("Luz alta — exceso de iluminacion");
        }
        tsl.clearInterrupt();
    }
}else{
  Serial.println("No hubo interrupcion");
}

}
