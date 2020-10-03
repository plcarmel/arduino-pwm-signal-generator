#define MIN_WIDTH 800L /*  µs  */
#define MAX_WIDTH 1600L /*  µs  */

#define MIN_ACCELERATION 10L /*  µ/s  */
#define MAX_ACCELERATION 10000L /*  µ/s  */

#define BUTTON_IN 2
#define SPEED_IN 1
#define ACCELERATION_IN 2
#define STATE_LED_OUT 12

#define BUTTON_DEBOUNCING_TIME 100L /*  ms  */

#define WIDTH_RANGE (MAX_WIDTH - MIN_WIDTH)
#define WIDTH_HALF_RANGE (WIDTH_RANGE >> 2)

#define ACCELERATION_RANGE (MAX_ACCELERATION - MIN_ACCELERATION)
#define ACCELERATION_HALF_RANGE (ACCELERATION_RANGE >> 2)

volatile unsigned long lastButtonInterrupt = 0;

volatile bool runState = false;

int accelerationVal;

int readRequestedWidth() {
  return MIN_WIDTH
    + (WIDTH_RANGE * analogRead(SPEED_IN) + WIDTH_HALF_RANGE) / 1023L;
}

int readRequestedAcceleration() {

}


void onButtonDebounced() {
  unsigned long currentTime = micros();
  if(((long)currentTime - lastButtonInterrupt) >= BUTTON_DEBOUNCING_TIME * 1000) {
    onButton();
    lastButtonInterrupt = currentTime;
  }
}

void onButton() {
  runState = !runState;
  digitalWrite(STATE_LED_OUT, runState);
  Serial.println("onButton");
}


void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_IN, INPUT_PULLUP);
  pinMode(STATE_LED_OUT, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_IN), onButtonDebounced, RISING);
  interrupts();
}

void loop() {
  accelerationVal = analogRead(ACCELERATION_IN);
  Serial.print(runState);
  Serial.print(" | ");
  Serial.println(readRequestedPulseWidth());
  delay(1000);
}
