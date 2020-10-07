// pulse width
#define PERIOD 50000.0  /* µs */
#define OFF_WIDTH 800.0  /* µs */
#define MIN_WIDTH 1000.0  /* µs */
#define MAX_WIDTH 1600.0  /* µs */
#define WIDTH_RANGE (MAX_WIDTH - MIN_WIDTH)

// rate of pulse width change
#define MIN_ACCELERATION 10.0  /* µ/s */
#define MAX_ACCELERATION 1000.0  /* µ/s */
#define ACCELERATION_LINEAR_MAX 1000 /* 0 - 1023 */
#define ACCELERATION_RANGE (MAX_ACCELERATION - MIN_ACCELERATION)

#define BUTTON_IN 2 /* connected to a button that sets the pin to the ground when pressed */
#define SPEED_IN 1 /* pin connected to a 4.7K potentiometer through a small 10 ohms series resistance */
#define ACCELERATION_IN 5 /* pin connected to a 4.7K potentiometer through a small 10 ohms series resistance */
#define STATE_LED_OUT 12 /* connected to a led through a 10K series resistance */
#define PWM_OUT 13 /* output signal */

// to avoid multiple triggers when the button is pressed
#define BUTTON_DEBOUNCING_TIME 500L /* ms */

volatile unsigned long lastButtonInterrupt = 0;
volatile bool runState = false;

float readRequestedWidth() {
  return MIN_WIDTH + WIDTH_RANGE * analogRead(SPEED_IN) / 1023.0;
}

float readRequestedAcceleration() {
  int val = analogRead(ACCELERATION_IN);
  return val > ACCELERATION_LINEAR_MAX
    ? INFINITY
    : MIN_ACCELERATION + ACCELERATION_RANGE * (float)val / (float)ACCELERATION_LINEAR_MAX;
}

void onButtonDebounced() {
  unsigned long currentTime = micros();
  if ((long)currentTime - lastButtonInterrupt >= BUTTON_DEBOUNCING_TIME * 1000) {
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
  float delta = requestedAcceleration == INFINITY ? diff : (dir * requestedAcceleration * PERIOD) / 1000000.0;
  float a = dir * diff;
  delta = constrain(delta, -a, a);
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
