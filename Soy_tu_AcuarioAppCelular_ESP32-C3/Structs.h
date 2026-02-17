//################ Variables filtros Analogico #####################
struct AnalogFilter {
  uint16_t raw;
  int16_t lecturaAdc;
  int32_t milivoltage;
  int firstRead;
  float yk;
  float yk_1;
  float alfa;
};
//#######################################################

//################ Variables filtros Digital #####################
struct DigitalFilter {
  int oldStatus = 1;
  unsigned long time = 0;
  bool flag = 0;
  int readButton = 0;
  int estadoFlotador = 0;
};

//#######################################################