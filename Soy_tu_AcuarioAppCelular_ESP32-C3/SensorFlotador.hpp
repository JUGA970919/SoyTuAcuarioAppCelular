void FlotadorIni (){
  pinMode(flotador,INPUT_PULLUP);
  ph  = preferences.getFloat("ph", 7.00);   // 7.00 si no existe
  tds = preferences.getInt("tds", 100);     // 100 si no existe
}

bool timmer(unsigned long &ActualTime, unsigned long Preset){
  if ((millis() - ActualTime) > Preset){
    ActualTime = millis();
    return true;
  }else{
    return false;
  }  
}

void StatusButton(DigitalFilter &filter, int pinButton,unsigned long debounce) {

  filter.readButton = digitalRead(pinButton);
  if (modoSimulacion == 0) {

    if (filter.oldStatus != filter.readButton) {
      filter.time = millis();
      filter.flag = 1;
    }

    if (filter.flag == 1 && timmer(filter.time, debounce)) {
      filter.estadoFlotador = (filter.readButton == LOW) ? 1 : 0;
      filter.flag = 0;
      filter.oldStatus = filter.readButton;
      //return true;   // hubo cambio válido
    }
    filter.oldStatus = filter.readButton;
  }
  else if(modoSimulacion == 1){
    filter.estadoFlotador = random(0,2);
  }
  //return false;    // no hubo cambio
}
