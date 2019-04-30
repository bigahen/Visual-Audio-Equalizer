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
uint32_t orange = strip.Color(255, 70, 0);
uint32_t yellow = strip.Color(255, 170, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t purple = strip.Color(255, 0, 255);
uint32_t white = strip.Color(255, 255, 255);

// MSGEQ7 Input Pins
const int audioOutput = A1;  //data out of MSGEQ7
const int strobePin = 3;    //strobe on MSGEQ7
const int resetPin = 4;      //reset on MSGEQ7

//Timer 3 Compare A Flag
volatile bool timer_comp_3_flag = 0; 

ISR (TIMER3_COMPA_vect)
{
    timer_comp_3_flag = 1; 
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
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

}

void loop() {
  
  //Update LEDs if Interrupt Flag is set
  if(timer_comp_3_flag){
    updateLEDs(); 
    timer_comp_3_flag = 0;
  }
  
}

void updateLEDs(){
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
    Serial.println(bandwidth_intensities[i]);
    //Call the setLEDs function and pass the value of the first LED in the given column along with its current intensity
    setLEDs(i*72, bandwidth_intensities[i]);
    digitalWrite(strobePin, HIGH);
  }
  strip.show();
  //delay(30);
}

void setLEDs(int start_led, int intensity){

  for(int j=0; j < intensity; j++){
    if(start_led == 0){
      strip.setPixelColor(start_led+j, colorChoose(start_led));
    }else if(start_led == 432){
      strip.setPixelColor(start_led+107-j, colorChoose(start_led));
    }
    
    strip.setPixelColor(start_led+35-j, colorChoose(start_led));
    strip.setPixelColor(start_led+36+j, colorChoose(start_led));
    strip.setPixelColor(start_led+71-j, colorChoose(start_led));
    strip.setPixelColor(start_led+72+j, colorChoose(start_led));
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

uint32_t colorChoose(int start_bit){
  switch (start_bit){
    case 0:
      return red;
      break;
    case 72:
      return orange;
      break;
    case 144:
      return yellow;
      break;
    case 216:
      return green;
      break;
    case 288:
      return blue;  
      break;
    case 360:
      return purple;
      break;
    case 432:
      return white;
      break;
  }
}

