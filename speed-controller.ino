/* pulse width */
#define PERIOD 50000.0  /*  µs  */
#define OFF_WIDTH 800.0  /*  µs  */
#define MIN_WIDTH 1000.0  /*  µs  */
#define MAX_WIDTH 1600.0  /*  µs  */

/* rate of pulse width change */
#define MIN_ACCELERATION 10.0  /*  µ/s  */
#define MAX_ACCELERATION 1000.0  /*  µ/s  */
#define ACCELERATION_LINEAR_MAX 1000 /* 0 - 1023 */

#define BUTTON_IN 2
#define SPEED_IN 1
#define ACCELERATION_IN 5
#define STATE_LED_OUT 12
#define PWM_OUT 13

#define BUTTON_DEBOUNCING_TIME 500L /*  ms  */

#define WIDTH_RANGE ((MAX_WIDTH - MIN_WIDTH))
#define ACCELERATION_RANGE ((MAX_ACCELERATION - MIN_ACCELERATION))

#define SECOND_TO_MICROS 1000000.0

volatile unsigned long lastButtonInterrupt = 0;
volatile bool runState = false;

float readRequestedWidth() {
  return MIN_WIDTH + WIDTH_RANGE * analogRead(SPEED_IN) / 1023.0;
}

float readRequestedAcceleration() {
  int val = analogRead(ACCELERATION_IN);
  Serial.println(val); 
  return val > ACCELERATION_LINEAR_MAX
    ? INFINITY
    : MIN_ACCELERATION + ACCELERATION_RANGE * (float)val / (float)ACCELERATION_LINEAR_MAX;
}

void onButtonDebounced() {
  unsigned long currentTime = micros();
  if (((long)currentTime - lastButtonInterrupt) >= BUTTON_DEBOUNCING_TIME * 1000) {
    onButton();
    lastButtonInterrupt = currentTime;
  }
}

void onButton() {
  runState = !runState;
  digitalWrite(STATE_LED_OUT, runState);
}

void sendPulse(long width) {
  digitalWrite(PWM_OUT, 1);
  delayMicroseconds(width);
  digitalWrite(PWM_OUT, 0);
}

long lastTime = micros();
float lastWidth = OFF_WIDTH;

void updateWidth() {
  float requestedWidth = runState ? readRequestedWidth() : OFF_WIDTH;
  float requestedAcceleration = readRequestedAcceleration();
  float diff = requestedWidth - lastWidth;
  float dir = diff ? (diff / abs(diff)) : 0.0;
  float delta = requestedAcceleration == INFINITY ? diff : (dir * requestedAcceleration * PERIOD) / SECOND_TO_MICROS;
  float a = dir * diff;
  delta = constrain(delta, -a, a);
  // Serial.print("last: ");
  // Serial.print(lastWidth);
  // Serial.print(" | requested: ");
  // Serial.print(requestedWidth);
  // Serial.print("| requestedAcceleration: ");
  // Serial.print(requestedAcceleration);
  // Serial.print("| dir: ");
  // Serial.print(dir);
  // Serial.print("| delta: ");
  // Serial.println(delta);
  lastWidth = lastWidth + delta;
}

void waitNextPeriod() {
  long newTime;
  while (true) {
    newTime = micros();
    if (newTime - lastTime >= PERIOD) {
      lastTime = lastTime + PERIOD;
      return;
    }
  }
}

void setup() {
  // Serial.begin(9600);
  pinMode(BUTTON_IN, INPUT_PULLUP);
  pinMode(STATE_LED_OUT, OUTPUT);
  pinMode(PWM_OUT, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_IN), onButtonDebounced, RISING);
  interrupts();
}

void loop() {
  updateWidth();
  sendPulse(lastWidth);
  waitNextPeriod();
}
