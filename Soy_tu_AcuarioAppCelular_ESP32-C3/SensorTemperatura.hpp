void InicioTemperatura(){
  sensors.begin();
  sensors.setResolution(12);  // RESOLUCION
  sensors.setWaitForConversion(false);      // SI SE ESPERA PARA LA CONVERSION
  sensors.requestTemperatures();            // inicia primera conversión
}

void LecturaTemperatura(){
  sensors.requestTemperatures();
  temperaturaCelcius = sensors.getTempCByIndex(0);
}