
void LecturaTds(){

  // ----- LECTURA ADC -----
  miliVoltageTds = ReadFilteredAnalog(TdsFilter, canal1Tds);
  voltageTds = (miliVoltageTds / 1000.0)*1.5;

  // ----- CÁLCULO TDS -----
  calculoTds = (133.42 * pow(voltageTds,3)
        -255.86 * pow(voltageTds,2)
        +857.39 * voltageTds)*0.5;

  // ----- COMPENSACIÓN POR TEMPERATURA -----
  factor = 1.0 + 0.02 * (temperaturaCelcius - 25.0);
  tds = calculoTds / factor;
              
}
