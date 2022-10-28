
#define POTI_PIN 15


long PotiValue ;
float giatriINra,giatriPhanTram,giatriTB;
void PIN_setup() {
  pinMode(POTI_PIN,INPUT);

}

float Energy() {
  PotiValue = analogRead(POTI_PIN);
  for( int i =0 ; i< 10; i++){
      PotiValue += analogRead(POTI_PIN);
  }
  giatriTB = PotiValue/10;
  
  if(giatriTB >= 1703 && giatriTB <= 2751){
    giatriPhanTram = float(((giatriTB - 1703)*100)/1048);
  }
  else if(giatriTB < 1703){
    giatriPhanTram = 0;
  }
  else  {
     giatriPhanTram = 100;
  }
  
  return giatriPhanTram;
}
