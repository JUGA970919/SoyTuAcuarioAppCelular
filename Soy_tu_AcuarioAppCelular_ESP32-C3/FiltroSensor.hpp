inline void AdsIni (){
  // ADS1115
  if (!ads.begin(0x48)) {
    Serial.println("ERROR ADS1115");
    //while (1);
    modoSimulacion = 1;
  }

  ads.setGain(GAIN_TWOTHIRDS); // ±6.144V (ideal 0–5V)

  Serial.println("Sistema listo ✅");
}


// Inicialización del filtro
inline void InitAnalogFilter(AnalogFilter &filter, int canalAds, float alfa) {

  filter.alfa = alfa;
  if (modoSimulacion == 0){
    filter.raw = ads.readADC_SingleEnded(canalAds);
    filter.firstRead = ads.computeVolts(filter.raw) * 1000.0;
  } 
  else if (modoSimulacion == 1){
    filter.firstRead = random(0,3300);
  }

  filter.yk = filter.firstRead;
  filter.yk_1 = filter.firstRead;
}

// Lectura filtrada
inline int ReadFilteredAnalog(AnalogFilter &filter, int canalAds) {

  // Leer ADC
  if (modoSimulacion == 0){
    filter.lecturaAdc = ads.readADC_SingleEnded(canalAds);
    filter.milivoltage = ads.computeVolts(filter.lecturaAdc) * 1000.0;
  } 
  else if(modoSimulacion == 1){
    filter.milivoltage = random(0,3300);
  }
  // ----- FILTRO EMA -----
  filter.yk = (filter.milivoltage * filter.alfa) + ((1.0 - filter.alfa) * filter.yk_1);
  filter.yk_1 = filter.yk;
  //delay(10);
  return filter.yk;
}
