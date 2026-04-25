
#include <Adafruit_TSL2591.h>
#include <Wire.h>
#define SDA_PIN 6
#define SCL_PIN 7

//Creacion de objeto de clase Adafruit_TSL2591 que hace referencia al sensor 
tsl2591Gain_t ganancias[] = {
  TSL2591_GAIN_LOW,  /// low gain (1x)
  TSL2591_GAIN_MED,  /// medium gain (25x)
  TSL2591_GAIN_HIGH, /// medium gain (428x)
  TSL2591_GAIN_MAX
};

tsl2591IntegrationTime_t timings[] = {
  TSL2591_INTEGRATIONTIME_100MS, // 100 millis
  TSL2591_INTEGRATIONTIME_200MS, // 200 millis
  TSL2591_INTEGRATIONTIME_300MS, // 300 millis
  TSL2591_INTEGRATIONTIME_400MS, // 400 millis
  TSL2591_INTEGRATIONTIME_500MS, // 500 millis
  TSL2591_INTEGRATIONTIME_600MS
};

const char* etiquetasGanancia[] = {
    "LOW",
    "MED",
    "HIGH",
    "MAX"
};

const char* etiquetasTiming[] = {
    "100",
    "200",
    "300",
    "400",
    "500",
    "600"
};


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

void scanIICDevices(){
  Serial.println("Escaneando bus I2C...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("✅ Dispositivo encontrado en: 0x");
      Serial.println(addr, HEX);
    }
  }
}
void sweepMeditions(){
  
  for(int i = 0; i < 4; i++){
      tsl.setGain(ganancias[i]);
    for(int j = 0; j < 6; j++){
      Serial.print(etiquetasGanancia[i]);
      Serial.print(";");
      tsl.setTiming(timings[j]);
      delay((j + 1) * 100 + 20);
      Serial.print(etiquetasTiming[j]);
      Serial.print(";");
      printResults();
    }
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


void printResults(){
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir,full;
  ir =  lum >> 16;
  full = lum & 0xFFFF;

  //ms;Ganancia;ATIME;CH0;CH1;Visible;Lux

  Serial.print(millis());
  Serial.print(";");
  //Serial.print(tsl.getGain());
  //Serial.print(";");
  //Serial.print(tsl.getTiming());
  //Serial.print(";");
  Serial.print(ir);
  Serial.print(";");
  Serial.print(full);
  Serial.print(";");
  Serial.print(full - ir);
  Serial.print(";");
  Serial.print(tsl.calculateLux(full,ir));
  Serial.println("");
}
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN,SCL_PIN);
  Wire.setClock(400000);
  initDevice();
  //scanIICDevices();
  //tsl.registerInterrupt(uint16_t lowerThreshold, uint16_t upperThreshold, tsl2591Persist_t persist)
}

void loop() {
  // put your main code here, to run repeatedly:
  sweepMeditions();

 
  //advancedRead();
  //delay(500);
}
