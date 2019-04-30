/*
 * LED Visual Equilizer Code for 18 x 30 Neopixal Grid 
 * 
 * Author: Austin Hendrix
 * Date: 18/6/18
 * 
 */
 
 //Variables Related to Neopixals and Audio Input
#include <Adafruit_NeoPixel.h>
#define LED_CONTROL_PIN 13
#define NUM_LEDS  540
#define MAX_BRIGHTNESS 25
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_CONTROL_PIN, NEO_GRB + NEO_KHZ800);

//Define Color Constants
uint32_t red = strip.Color(255, 0, 0);
uint32_t pink = strip.Color(255, 90, 90);
uint32_t orange = strip.Color(255, 70, 0);
uint32_t yellow = strip.Color(255, 170, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t purple = strip.Color(255, 0, 255);
uint32_t white = strip.Color(255, 255, 255);
uint32_t off = strip.Color(0,0,0);

// MSGEQ7 Input Pins
const int audioOutput = A1;  //data out of MSGEQ7
const int strobePin = 3;    //strobe on MSGEQ7
const int resetPin = 4;      //reset on MSGEQ7

// Values Avaliable for Board Colors
enum Board_Color {
    Red,
    Orange,
    Yellow,
    Green,
    Blue, 
    Purple, 
    White,
    Rainbow,
    Pink
  };

// Values Available for Board Orientation
enum Board_Orientation {
    Horizontal,
    Vertical, 
    VerticalMirror
};

// Global Variables Containing Board Oritenation and Board Color 
Board_Color board_color; 
Board_Orientation board_orientation; 

//Timer 3 Compare A Flag
volatile bool timer_comp_3_flag = 0; 

ISR (TIMER3_COMPA_vect)
{
    timer_comp_3_flag = 1; 
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.setTimeout(150); 
  while(!Serial){
    //wait
  }

  //Set up interrupts to handle updating of screen
  noInterrupts();
  TCCR3A = 0;
  TCCR3B = 0; 
  TCNT3 = 0; 

  OCR3A = 12500; // Output compare value
  TCCR3B |= (1 << CS31) | (1 << CS30) | (1 << WGM32); // Set clock precaler to 64 and turn on CTC mode
  TIMSK3 |= (1 << OCIE3A); // Active timer compare interrupt
  interrupts();

  //Initialize NeoPixal Strip
  strip.begin();
  strip.setBrightness(MAX_BRIGHTNESS);
  strip.show(); 

  //Initialize pins for MSGEQ7 board
  pinMode(audioOutput, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);
  
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  board_color = Rainbow; 
  board_orientation = VerticalMirror; 
}

void loop() {
  
  //Update LEDs if Interrupt Flag is set
  if(timer_comp_3_flag){
    updateLEDs(); 
    timer_comp_3_flag = 0;

    checkConfig(); 
  }
  
}

// Check config performs a handshake over Serial to a python script which will return an update command if available
void checkConfig(){

    while(Serial.available() > 0) {
      char t = Serial.read();
    }
    Serial.println("Board Checking In"); 
    Serial.flush(); 

    // Wait the timeout of 25 ms for a command to come from the serial port
    long start_time = millis();
    while(!Serial.available()){
        if(millis() - start_time > 25){
          break;
        }
    }
    if(Serial.available()){

        char command = Serial.read();
        Serial.println(command);
        if(command == 'C'){
            updateColor(); 
             
        }
        else if(command == 'O'){
            updateOrientation(); 
        }
        // This shouldn't occur
        else{
        }
        Serial.flush();        
    }
    
}

void updateColor(){
   if (board_color == Pink){
          board_color = Red;
   }else{
          board_color = int(board_color)+1; 
   }
}

void updateOrientation(){
    if (board_orientation == VerticalMirror){
          board_orientation = Horizontal; 
    }else{
          board_orientation = int(board_orientation)+1; 
    } 
}

void updateLEDs(){
  
    switch (board_orientation) {
      case Vertical: 
        updateLEDsVeritcal(); 
        break;
      case Horizontal: 
        updateLEDsHorizontal(); 
        break;
      case VerticalMirror:
        updateLEDsVerticalMirror();
        break;
    }
}


void updateLEDsVerticalMirror(){
    //Toggle Reset Pin so MSGEQ7 gives fresh input
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);

  int bandwidth_intensities[7];

  for(int i = 0; i < 7; i++){
    //Setting strobe pin LOW indicates we are reading a new frequency bandwidth 
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); //allow output audio to settle 
    //Map the input reading of 0-1023 to a value between 0-9 
    //This is used to determine how many LEDs will be turned on 
    bandwidth_intensities[i] = map(analogRead(audioOutput), 0, 1023, 0, 18);
    //Serial.println(bandwidth_intensities[i]);
    
    setVeritcalMirrorLEDs(i, bandwidth_intensities[i]);
    digitalWrite(strobePin, HIGH);
  }

    strip.show();
}

void updateLEDsHorizontal(){
   //Toggle Reset Pin so MSGEQ7 gives fresh input
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);

  int bandwidth_intensities[7];

  for(int i = 0; i < 7; i++){
    //Setting strobe pin LOW indicates we are reading a new frequency bandwidth 
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); //allow output audio to settle 
    //Map the input reading of 0-1023 to a value between 0-9 
    //This is used to determine how many LEDs will be turned on 
    bandwidth_intensities[i] = map(analogRead(audioOutput), 0, 1023, 0, 30);
    //Serial.println(bandwidth_intensities[i]);
    
    setHorizontalLEDs(i, bandwidth_intensities[i]);
    digitalWrite(strobePin, HIGH);
  }

  int average_intensity = 0; 

  for (int i = 0; i < 7; i++)
  {
    average_intensity += bandwidth_intensities[i]; 
  }

  average_intensity /= 7; 
  setHorizontalAverageLeds(average_intensity); 
  strip.show();
  //delay(30);
}


void updateLEDsVeritcal(){
   //Toggle Reset Pin so MSGEQ7 gives fresh input
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);

  int bandwidth_intensities[7];

  for(int i = 0; i < 7; i++){
    //Setting strobe pin LOW indicates we are reading a new frequency bandwidth 
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); //allow output audio to settle 
    //Map the input reading of 0-1023 to a value between 0-9 
    //This is used to determine how many LEDs will be turned on 
    bandwidth_intensities[i] = map(analogRead(audioOutput), 0, 1023, 0, 18);
    //Serial.println(bandwidth_intensities[i]);
    
    setVeritcalLEDs(i, bandwidth_intensities[i]);
    digitalWrite(strobePin, HIGH);
  }

  strip.show();
  //delay(30);
}

void setVeritcalMirrorLEDs(int band, int intensity){
  int start_led = band*36; 

  // Set first half of LEDS
  for(int j=0; j < intensity; j++){
    if(start_led == 0){
      strip.setPixelColor(start_led+j, colorChoose(band));
    }
    
    strip.setPixelColor(start_led+35-j, colorChoose(band));
    strip.setPixelColor(start_led+36+j, colorChoose(band));
    //strip.setPixelColor(start_led+71-j, colorChoose(band));
    //strip.setPixelColor(start_led+72+j, colorChoose(band));
  }

  for(int k=17; k > intensity; k--){
    if(start_led == 0){
      strip.setPixelColor(start_led+k, 0,0,0);
    }
    strip.setPixelColor(start_led+35-k, 0,0,0);
    strip.setPixelColor(start_led+36+k, 0,0,0);
    //strip.setPixelColor(start_led+71-k, 0,0,0);
    //strip.setPixelColor(start_led+72+k, 0,0,0);
  }

  // End first half

  // Set second half LEDS
  start_led = 539 - band*36;
  for(int j=intensity-1; j >= 0; j--){
    if(start_led == 539){
      strip.setPixelColor(start_led-j, colorChoose(band));
    }
    
    strip.setPixelColor(start_led-35+j, colorChoose(band));
    strip.setPixelColor(start_led-36-j, colorChoose(band));
    //strip.setPixelColor(start_led+71-j, colorChoose(band));
    //strip.setPixelColor(start_led+72+j, colorChoose(band));
  }

  for(int k=17; k > intensity; k--){
    if(start_led == 539){
      strip.setPixelColor(start_led-k, 0,0,0);
    }
    strip.setPixelColor(start_led-35+k, 0,0,0);
    strip.setPixelColor(start_led-36-k, 0,0,0);
    //strip.setPixelColor(start_led+71-k, 0,0,0);
    //strip.setPixelColor(start_led+72+k, 0,0,0);
  }
  
}

void setVeritcalLEDs(int band, int intensity){
  int start_led = band*72; 
  for(int j=0; j < intensity; j++){
    if(start_led == 0){
      strip.setPixelColor(start_led+j, colorChoose(band));
    }else if(start_led == 432){
      strip.setPixelColor(start_led+107-j, colorChoose(band));
    }
    
    strip.setPixelColor(start_led+35-j, colorChoose(band));
    strip.setPixelColor(start_led+36+j, colorChoose(band));
    strip.setPixelColor(start_led+71-j, colorChoose(band));
    strip.setPixelColor(start_led+72+j, colorChoose(band));
  }

  for(int k=17; k > intensity; k--){
    if(start_led == 0){
      strip.setPixelColor(start_led+k, 0,0,0);
    }else if(start_led == 432){
      strip.setPixelColor(start_led+107-k, 0,0,0);
    }
    strip.setPixelColor(start_led+35-k, 0,0,0);
    strip.setPixelColor(start_led+36+k, 0,0,0);
    strip.setPixelColor(start_led+71-k, 0,0,0);
    strip.setPixelColor(start_led+72+k, 0,0,0);
  }
  
}

void setHorizontalLEDs(int band, int intensity){

  for(int j=0; j < intensity; j++){
    if(j == 0){
      strip.setPixelColor(2*band, colorChoose(band));
      strip.setPixelColor(2*band+1, colorChoose(band));
    }else if(j % 2 == 0){
      strip.setPixelColor(36 * (j / 2) + 2*band, colorChoose(band));
      strip.setPixelColor(36 * (j / 2) + 2*band + 1, colorChoose(band));
    }else{
      strip.setPixelColor(36 * (j / 2 + 1) - 2*band - 1, colorChoose(band));
      strip.setPixelColor(36 * (j / 2 + 1) - 2*band - 2, colorChoose(band));
    }
  }
  

  for(int k=29; k > intensity; k--){
    if(k % 2 == 0){
      strip.setPixelColor(36 * (k / 2) + 2*band, 0,0,0);
      strip.setPixelColor(36 * (k / 2) + 2*band + 1, 0,0,0);
    }else{
      strip.setPixelColor(36 * (k / 2 + 1) - 2*band - 1, 0,0,0);
       strip.setPixelColor(36 * (k / 2 + 1) - 2*band - 2, 0,0,0);
    }
  }
  
}

void setHorizontalAverageLeds(int average_intensity){

    int band = 7; 
    for (int band = 7; band <= 8; band++){
      for(int j=0; j < average_intensity; j++){
        if(j == 0){
          strip.setPixelColor(2*band, colorChoose(7));
          strip.setPixelColor(2*band+1, colorChoose(7));
        }else if(j % 2 == 0){
          strip.setPixelColor(36 * (j / 2) + 2*band, colorChoose(7));
          strip.setPixelColor(36 * (j / 2) + 2*band + 1, colorChoose(7));
        }else{
          strip.setPixelColor(36 * (j / 2 + 1) - 2*band - 1, colorChoose(7));
          strip.setPixelColor(36 * (j / 2 + 1) - 2*band - 2, colorChoose(7));
        }
      }
      
    
      for(int k=29; k > average_intensity; k--){
        if(k % 2 == 0){
          strip.setPixelColor(36 * (k / 2) + 2*band, 0,0,0);
          strip.setPixelColor(36 * (k / 2) + 2*band + 1, 0,0,0);
        }else{
          strip.setPixelColor(36 * (k / 2 + 1) - 2*band - 1, 0,0,0);
           strip.setPixelColor(36 * (k / 2 + 1) - 2*band - 2, 0,0,0);
        }
      }
    }
}

// Will make refactoring very easy 
uint32_t colorChoose(int band){
  switch(board_color){
    case Rainbow:
      return colorChooseRainbow(band);
      break;
    case Red:
      return red; 
      break;
      
    case Orange:
      return orange; 
      break;

    case Yellow:
      return yellow; 
      break;

    case Green:
      return green; 
      break;

    case Blue:
      return blue; 
      break;

    case Purple:
      return purple; 
      break;

    case White:
      return white; 
      break;

    case Pink:
      return pink; 
      break;
    
  }
}

uint32_t colorChooseRainbow(int band){
  switch (band){
    case 0:
      return red;
      break;
    case 1:
      return orange;
      break;
    case 2:
      return yellow;
      break;
    case 3:
      return green;
      break;
    case 4:
      return blue;  
      break;
    case 5:
      return purple;
      break;
    case 6:
      return white;
      break;
    // This can only be used by the average 
    case 7:
      return pink;
      break; 

   default: 
      return off; 
  }
}
