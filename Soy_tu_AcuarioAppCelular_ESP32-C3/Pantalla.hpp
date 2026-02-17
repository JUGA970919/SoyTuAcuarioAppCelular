void pantallaIni(){
  // OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(10, 30, "Iniciando...");
  u8g2.sendBuffer();
}


void drawScreen(const char* label, const char* value) {
  u8g2.clearBuffer();

  // Etiqueta centrada
  u8g2.setFont(u8g2_font_ncenB18_tr);
  int labelWidth = u8g2.getStrWidth(label);
  int labelX = (128 - labelWidth) / 2;
  u8g2.drawStr(labelX, 22, label);

  // Valor grande centrado
  u8g2.setFont(u8g2_font_logisoso32_tn);
  int valueWidth = u8g2.getStrWidth(value);
  int valueX = (128 - valueWidth) / 2;
  u8g2.drawStr(valueX, 60, value);

  u8g2.sendBuffer();
}

void MostarPantalla(){
  unsigned long now = millis();

  if (now - lastChange >= interval) {
    lastChange = now;

    char valueBuffer[10];

    switch (screenIndex) {
      case 0:
        dtostrf(ph, 3, 1, valueBuffer);
        drawScreen("PH =", valueBuffer);
        break;

      case 1:
        sprintf(valueBuffer, "%d", tds);
        drawScreen("TDS =", valueBuffer);
        break;

      case 2:
        dtostrf(temperaturaCelcius, 4, 1, valueBuffer);
        drawScreen("TEMP =", valueBuffer);
        break;

      case 3:
        sprintf(valueBuffer, "%d", nivelAgua.estadoFlotador);
        drawScreen("NIVEL =", valueBuffer);
        break;

      case 4:
        dtostrf(turbidez, 4, 2, valueBuffer);
        drawScreen("TURB =", valueBuffer);
        break;
    }

    screenIndex++;
    if (screenIndex > 4) screenIndex = 0;
  }
}