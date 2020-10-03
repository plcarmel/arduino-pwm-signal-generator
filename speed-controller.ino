/* pulse width */
#define PERIOD 100000L  /*  µs  */
#define OFF_WIDTH 800L  /*  µs  */
#define MIN_WIDTH 100L  /*  µs  */
#define MAX_WIDTH 5000L  /*  µs  */

/* rate of pulse width change */
#define MIN_ACCELERATION 10L  /*  µ/s  */
#define MAX_ACCELERATION 10000L  /*  µ/s  */

#define BUTTON_IN 2
#define SPEED_IN 1
#define ACCELERATION_IN 2
#define STATE_LED_OUT 12
#define PWM_OUT 13

#define BUTTON_DEBOUNCING_TIME 100L /*  ms  */

#define WIDTH_RANGE (MAX_WIDTH - MIN_WIDTH)
#define WIDTH_HALF_RANGE (WIDTH_RANGE >> 1)

#define ACCELERATION_RANGE (MAX_ACCELERATION - MIN_ACCELERATION)
#define ACCELERATION_HALF_RANGE (ACCELERATION_RANGE >> 1)

#define SECOND_TO_MICROS 1000000L

volatile unsigned long lastButtonInterrupt = 0;
volatile bool runState = false;

int readRequestedWidth() {
  return MIN_WIDTH
    + (WIDTH_RANGE * analogRead(SPEED_IN) + WIDTH_HALF_RANGE) / 1023L;
}

int readRequestedAcceleration() {
  return MIN_ACCELERATION
    + (ACCELERATION_RANGE * analogRead(ACCELERATION_IN) + ACCELERATION_HALF_RANGE) / 1023L;
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
long lastWidth = OFF_WIDTH;

void updateWidth() {
  long requestedWidth = runState ? readRequestedWidth() : OFF_WIDTH;
  long requestedAcceleration = readRequestedAcceleration();
  long diff = requestedWidth - lastWidth;
  long dir = constrain(diff, -1L, 1L);
  long delta = (dir * requestedAcceleration * PERIOD + SECOND_TO_MICROS >> 1) / SECOND_TO_MICROS;
  if (!delta) delta = dir;
  long a = dir * diff;
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
  sendPulse(lastWidth * 10L);
  waitNextPeriod();
}
