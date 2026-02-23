void InicioTemperatura(){
  sensors.begin();
  sensors.setResolution(12);  // RESOLUCION
  sensors.setWaitForConversion(false);      // SI SE ESPERA PARA LA CONVERSION
  sensors.requestTemperatures();            // inicia primera conversión
}

void LecturaTemperatura(){
  
  if (modoSimulacion == 0){
    sensors.requestTemperatures();
    temperaturaCelcius = sensors.getTempCByIndex(0);
  }
  else if (modoSimulacion == 1){
    temperaturaCelcius = random(18.9,35.1);
  }
  
}