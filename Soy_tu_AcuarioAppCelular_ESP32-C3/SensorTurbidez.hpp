void LecturaTurbidez(){

  // ----- LECTURA ADC -----
  miliVoltageTurbidez = ReadFilteredAnalog(TurbidezFilter, canal2Turb);
  voltageTurbidez = (miliVoltageTurbidez / 1000.0)*1.5;

  // ----- CÁLCULO TDS -----
  //calculoTurbidez = -1120.4 * pow(voltageTurbidez,2)
  //                 +5742.3 * voltageTurbidez - 4352.9;


   // Limitar valores negativos
  //if (calculoTurbidez < 0) calculoTurbidez = 0;     

  turbidez =  voltageTurbidez;      
}

