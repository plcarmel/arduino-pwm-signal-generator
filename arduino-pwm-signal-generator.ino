// pulse width
const float PERIOD = 50000; // µs 
const float OFF_WIDTH = 800; // µs 
const float MIN_WIDTH = 850; // µs 
const float MAX_WIDTH = 1600; // µs 
const float WIDTH_RANGE = MAX_WIDTH - MIN_WIDTH;

// rate of pulse width change
const float MIN_ACCELERATION = 10; // µ/s 
const float MAX_ACCELERATION = 1000; // µ/s 
const int ACCELERATION_LINEAR_MAX = 1000; // 0 - 1023
const float ACCELERATION_RANGE = MAX_ACCELERATION - MIN_ACCELERATION;

const int BUTTON_IN = 2; // connected to a button that sets the pin to ground when pressed
const int SPEED_IN = 1; // pin connected to a 4.7K potentiometer through a 10 ohms series resistance
const int ACCELERATION_IN = 5; // pin connected to a 4.7K potentiometer through a 10 ohms series resistance
const int STATE_LED_OUT = 12; // connected to a led through a 10K series resistance
const int PWM_OUT = 13; // output signal

// to avoid multiple triggers when the button is pressed
const long BUTTON_DEBOUNCING_TIME = 500; // ms

volatile unsigned long lastButtonInterrupt = 0;
volatile bool runState = false;

float readRequestedWidth() {
  return MIN_WIDTH + WIDTH_RANGE * analogRead(SPEED_IN) / 1023;
}

float readRequestedAcceleration() {
  const int val = analogRead(ACCELERATION_IN);
  return val > ACCELERATION_LINEAR_MAX
    ? INFINITY
    : MIN_ACCELERATION + ACCELERATION_RANGE * val / ACCELERATION_LINEAR_MAX;
}

void onButtonDebounced() {
  const long currentTime = micros();
  if (currentTime - lastButtonInterrupt >= BUTTON_DEBOUNCING_TIME * 1000) {
    onButton();
    lastButtonInterrupt = currentTime;
  }
}

void onButton() {
  runState = !runState;
  digitalWrite(STATE_LED_OUT, runState);
}

void sendPulse(const long width) {
  digitalWrite(PWM_OUT, 1);
  delayMicroseconds(width);
  digitalWrite(PWM_OUT, 0);
}

long lastTime = micros();
float currentWidth = OFF_WIDTH;

void updateWidth() {
  const float requestedWidth = runState ? readRequestedWidth() : OFF_WIDTH;
  const float requestedAcceleration = readRequestedAcceleration();
  const float w =
    runState && currentWidth < MIN_WIDTH
    ? MIN_WIDTH
    : currentWidth;
  const float diff = requestedWidth - w;
  const float dir = diff ? diff / abs(diff) : 0;
  const float unconstrainedDelta =
    requestedAcceleration == INFINITY
    ? diff
    : (dir * requestedAcceleration * PERIOD) / 1000000;
  const float limit = dir * diff;
  const float delta = constrain(unconstrainedDelta, -limit, limit);
  const float w2 = w + delta;
  currentWidth = w2 < MIN_WIDTH ? OFF_WIDTH : w2;
}

void waitNextPeriod() {
  while (true) {
    const long newTime = micros();
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
  sendPulse(currentWidth);
  waitNextPeriod();
}
