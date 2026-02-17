

void Setponts(){
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  /*
  "Seleccione con que calibracion bufer va a calibrar");
  "Primero coloque el sensor en el bufer y despues seleccione una opcion"
    "1 -- Buffer 4.0"
    "2 -- Buffer 6.86"
    "3 -- Buffer 9.18"
    "4 -- Guardar Calibracion"
  */

  if (cmd == "Calph1") {
    //LecturaPh();
    ph_1 = voltagePh;
    pasoCalibracionph = 1;
    Serial.println("Datos recopilados");
    Serial.print("voltaje mV = " );
    Serial.print(ph_1);
    Serial.print(", " );
    Serial.print("Buffer = " );
    Serial.println(bufferph_1);
  }  
    

  if (cmd == "Calph2") {
    //LecturaPh();
    ph_2 = voltagePh;
    pasoCalibracionph = 2;
    Serial.println("Datos recopilados");
    Serial.print("voltaje mV = " );
    Serial.print(ph_2);
    Serial.print(", " );
    Serial.print("Buffer = " );
    Serial.println(bufferph_2);
  }

  if (cmd == "Calph3") {
    //LecturaPh();
    ph_3 = voltagePh;
    pasoCalibracionph = 3;
    Serial.println("Datos recopilados");
    Serial.print("voltaje mV = " );
    Serial.print(ph_3);
    Serial.print(", " );
    Serial.print("Buffer = " );
    Serial.println(bufferph_3);
  }

  if (cmd == "GuardarCalibracion") {

    if (pasoCalibracionph == 3 && ph_3 != 0){
    // Hacer el calculo de la ecuacion (pendiente)
    float sumX = ph_1 + ph_2 + ph_3;  //milivolts leidos
    float sumY = bufferph_1 + bufferph_2 + bufferph_3;// ph de solucion buffer
    float sumXY = (ph_1 * bufferph_1) + (ph_2 * bufferph_2) + (ph_3 * bufferph_3); 
    float sumX2 = (ph_1 * ph_1) + (ph_2 * ph_2) + (ph_3 * ph_3);

    float m = (3*sumXY - sumX*sumY) / (3*sumX2 - sumX*sumX);
    float b = (sumY - m*sumX) / 3;
    Serial.println("Calibracion Guardada");
    Serial.print("m = ");
    Serial.println(m);
    Serial.print("b = ");
    Serial.println(b);
    offsetph = b;
    slopeph = m;
    // Guarda el valor actual de ph en la Flash
    preferences.putFloat("offsetph", offsetph);
    // Guarda el valor actual de tds en la Flash
    preferences.putInt("slopeph", slopeph);
    ph_1 = 0;
    ph_2 = 0;
    ph_3 = 0;
    } 
    else{
        Serial.println("Por favor, termine de calibrar usando las 3 soluciones para poder guardas la calibracion");
      }
    }
  }