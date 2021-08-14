#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

#define PWM_INPUT_PIN 3
#define ESC_OUTPUT_PIN 2
#define INTEGRATE_STEP 5000
#define PWM_PROBE_DELAY 100
#define SMOOTH_TICK 50

volatile unsigned long fall_Time = 0;                   // Placeholder for microsecond time when last falling edge occured.
volatile unsigned long rise_Time = 0;                   // Placeholder for microsecond time when last rising edge occured.
volatile byte dutyCycle = 0;                            // Duty Cycle %
volatile unsigned long lastRead = 0;

unsigned long previousMillis = 0;
const long interval = 100;
boolean tick = false;

void setup() {
  Serial.begin(115200);
  myservo.attach(ESC_OUTPUT_PIN);                       // attaches the servo on pin 9 to the servo object
  pinMode(13, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PWM_INPUT_PIN), PinChangeISR0, CHANGE);
  dutyCycle = 10;
}

void PinChangeISR0() {                                  // Pin 2 (Interrupt 0) service routine
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
    int newDutyCycle = 100 / total_Time;                // Convert to a percentage
    if (total_Time < 100) {
      if (newDutyCycle < dutyCycle) {
        dutyCycle--;
      }
      if (newDutyCycle > dutyCycle) {
        dutyCycle++;
      }
    }
    rise_Time = lastRead;                               // Store rise time
  }
}

void updateState() {
  int fastPwm = 1000 + dutyCycle*10;
  long timeSinceLast = millis() - lastRead/1000;
  Serial.print("timelast: ");
  Serial.println(timeSinceLast);
  if (timeSinceLast > 1000 || timeSinceLast < 0) {
    dutyCycle = 0;
    fastPwm = -1;
  }

  Serial.print("ESC: ");
  Serial.println(dutyCycle);
  tick = !tick;
  digitalWrite(13, tick?HIGH:LOW);

  myservo.writeMicroseconds(fastPwm);
  //v is 0-1000
  int promille = fastPwm - 1000;
  boolean high = false;
  if (promille > 500) {
    high = true;
    promille = promille - 500;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    updateState();
  }
}
