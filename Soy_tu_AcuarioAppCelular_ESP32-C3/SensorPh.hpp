void Ph_Inputs(){
  preferences.begin("misDatos", false);
  offsetph  = preferences.getFloat("offsetph", 22.446871);   // 22.446871 si no existe
  slopeph = preferences.getInt("slopeph", -9.141055);     // -9.141055 si no existe
  //b
  //m
}

// ===== FUNCIÓN PRINCIPAL DE PH =====
void LecturaPh() {

  miliVoltagePh = ReadFilteredAnalog(PhFilter, canal0Ph);
  voltagePh = miliVoltagePh / 1000.0;

  //  Convertir a pH 
  //ph = slopeph * miliVoltagePh + offsetph;
  ph = slopeph * voltagePh + offsetph;

  if (ph < 0.0){
    ph = 0;
  } 
  else if (ph > 14.0){
    ph = 14;
  }
}