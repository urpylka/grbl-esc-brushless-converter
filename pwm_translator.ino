#include <Servo.h>
#include <FastLED.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

#define DATA_PIN 5
#define CLOCK_PIN 4
#define NUM_LEDS 9

#define PWM_INPUT_PIN 3
#define ESC_OUTPUT_PIN 2
#define INTEGRATE_STEP 5000
#define PWM_PROBE_DELAY 100
#define SMOOTH_TICK 50
#define BRIGHTNESS 5

CRGB leds[NUM_LEDS];
volatile unsigned long fall_Time = 0;                   // Placeholder for microsecond time when last falling edge occured.
volatile unsigned long rise_Time = 0;                   // Placeholder for microsecond time when last rising edge occured.
volatile byte dutyCycle = 0;                            // Duty Cycle %
volatile unsigned long lastRead = 0;

unsigned long previousMillis = 0; 
const long interval = 100;
boolean tick = false;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(16)>(leds, NUM_LEDS);
  FastLED.clear();
  leds[0].green = BRIGHTNESS;
  myservo.attach(ESC_OUTPUT_PIN);  // attaches the servo on pin 9 to the servo object
  pinMode(13, OUTPUT); 
  attachInterrupt(digitalPinToInterrupt(PWM_INPUT_PIN),PinChangeISR0,CHANGE);
  dutyCycle = 10;
}

void PinChangeISR0(){                                   // Pin 2 (Interrupt 0) service routine
  lastRead = micros();                                  // Get current time
  if (digitalRead(PWM_INPUT_PIN) == LOW) {
    // Falling edge
    fall_Time = lastRead;                               // Just store falling edge and calculate on rising edge
  }
  else {
    // Rising edge
    unsigned long total_Time = rise_Time - lastRead;    // Get total cycle time
    unsigned long on_Time = fall_Time - rise_Time;      // Get on time during this cycle
    total_Time = total_Time / on_Time;                  // Divide it down
    int newDutyCycle = 100 / total_Time;                       // Convert to a percentage
    if(total_Time < 100){
      if(newDutyCycle < dutyCycle){
        dutyCycle--;
      }
      if(newDutyCycle > dutyCycle){
        dutyCycle++;
      }
    }
    rise_Time = lastRead;                               // Store rise time
  }
}

void redFull(){
  if(tick){
    leds[0].red = BRIGHTNESS;
    leds[1].red = BRIGHTNESS;
    leds[2].red = BRIGHTNESS;
    leds[3].red = BRIGHTNESS;
    leds[4].red = BRIGHTNESS;
    leds[5].red = BRIGHTNESS;
    leds[6].red = BRIGHTNESS;
    leds[7].red = BRIGHTNESS;
    leds[8].red = BRIGHTNESS;
  }
}

void blueFull(){
  if(tick){
    leds[0].blue = BRIGHTNESS;
    leds[1].blue = BRIGHTNESS;
    leds[2].blue = BRIGHTNESS;
    leds[3].blue = BRIGHTNESS;
    leds[4].blue = BRIGHTNESS;
    leds[5].blue = BRIGHTNESS;
    leds[6].blue = BRIGHTNESS;
    leds[7].blue = BRIGHTNESS;
    leds[8].blue = BRIGHTNESS;
  }
}


void redFirstHalf(){
  if(tick){
    leds[0].red = BRIGHTNESS;
    leds[1].red = BRIGHTNESS;
    leds[2].red = BRIGHTNESS;
    leds[3].red = BRIGHTNESS;
    leds[4].red = BRIGHTNESS;
  }
}

void redSecondHalf(){
  if(tick){
    leds[4].red = BRIGHTNESS;
    leds[5].red = BRIGHTNESS;
    leds[6].red = BRIGHTNESS;
    leds[7].red = BRIGHTNESS;
    leds[8].red = BRIGHTNESS;
  }
}

void updateState(){
  FastLED.clear();
  int fastPwm = 1000+dutyCycle*10;
  long timeSinceLast = millis()-lastRead/1000;
  Serial.print("timelast ");
  Serial.println(timeSinceLast);
  if(timeSinceLast > 1000 || timeSinceLast < 0){    
     dutyCycle = 0;
     fastPwm = -1;
  }

  Serial.print("ESC: ");
  Serial.println(dutyCycle);
  tick = !tick;
  digitalWrite(13, tick?HIGH:LOW); 
  if(dutyCycle < 0 || dutyCycle > 100){
    blueFull();
    return;
  }

  if(fastPwm < 0){
    redFull();
    return;
  }
  if(fastPwm < 1000){
    redFirstHalf();
    myservo.writeMicroseconds(1000);
    return;
  }
  if(fastPwm > 2000){
    redSecondHalf();
    myservo.writeMicroseconds(1000);
    return;
  }
  myservo.writeMicroseconds(fastPwm);
  //v is 0-1000
  int promille = fastPwm-1000;
  boolean high = false;
  if(promille > 500){
    high = true;
    promille = promille-500;
  }
  int ledIncrement = 500/NUM_LEDS;
  int amountToLight = promille / ledIncrement;
  if(amountToLight  > NUM_LEDS){
    amountToLight = NUM_LEDS;
  }
  while(amountToLight > 0){
    if(high){
      leds[amountToLight-1].blue = BRIGHTNESS;        
    }
    leds[amountToLight-1].green = BRIGHTNESS;        
    amountToLight--;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateState();
    FastLED.show();
  } 
}


