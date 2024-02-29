#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_COUNT 240
#define PIN 2
#define STEP_SPEED 1
#define ON_MAX_TIME 600
#define MOTION_DELAY 50
#define BTN_DELAY 10
#define RAINBOW_LEN 12

#define MO_SENS_L_IO 8
#define MO_SENS_R_IO 7
#define LIGHT_SENS_IO 4
#define BTN_IO 5

Adafruit_NeoPixel pixels(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

#define MODE_NORMAL  0
#define MODE_ON_SINGULA  1
#define MODE_RAINBOW  2
#define MODECOUNT  3
int mode = MODE_NORMAL;

int s = 1;
int r = 60;
int l = LED_COUNT;
int h = 92;
int hc = h;
bool rainbow = false;

int timer = 0;
int on_timer = 0;

int sensorLOnI = 0;
int sensorROnI = 0;
int btnOnI = 0;

bool sensRMerker = false;
bool sensLMerker = false;
bool btnMerker = false;

void setup() {
  // put your setup code here, to run once:

  //Serial.begin(9600);

  pinMode(MO_SENS_L_IO, INPUT);
  pinMode(MO_SENS_R_IO, INPUT);
  pinMode(LIGHT_SENS_IO, INPUT);
  pinMode(BTN_IO, INPUT_PULLUP);

	pinMode(LED_BUILTIN, OUTPUT);

  pixels.begin();
}

double s1(int ii, int it) {
  double i = ii;
  double t = it;
  double x = i - t;
  if (x < s) {
    return 0;
  } else if (x < (s + r)) {
    return (cos( ((x-s-r)/r)/(PI/10) ) + 1) / 2;
  } else if (x < (s + r + l)) {
    return 1;
  } else if (x < (s + r + l + r)) {
    return (cos( ((x-s-l-r)/r) / (PI/10) ) + 1) / 2;
  } else {
    return 0;
  }
}

int minStart = -r -l -r;
int midStart = -r;
int maxStart = LED_COUNT;
int start = minStart;
int step = 0;

// States
const int States_OFF_L = 0;
const int States_OFF_R = 1;
const int States_TURN_ON_L = 2;
const int States_TURN_ON_R = 3;
const int States_ON = 4;
const int States_TURN_OFF_L = 5;
const int States_TURN_OFF_R = 6;
// States END

int state = States_OFF_L;
double light_val = 1.0D;

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

static rgb hsv2rgb(double h, double s, double v);

int lstate = state;
int llight = false;
int tick = 0;

void loop() {
  tick++;

  if (digitalRead(MO_SENS_L_IO)) {
    sensorLOnI = MOTION_DELAY;
  }
  if (digitalRead(MO_SENS_R_IO)) {
    sensorROnI = MOTION_DELAY;
  }
  if (!digitalRead(BTN_IO)) {
    btnOnI = BTN_DELAY;
  }
  bool sensorLOn = sensorLOnI > 0;
  bool sensorROn = sensorROnI > 0;
  bool btnOn = btnOnI > 0;
  bool lightOn = !digitalRead(LIGHT_SENS_IO);

  if (sensorLOnI > 0) sensorLOnI -= 1;
  if (sensorROnI > 0) sensorROnI -= 1;
  if (btnOnI > 0) btnOnI -= 1;

  pixels.clear();
  
  // Btn Action
  if (btnOn != btnMerker && btnOn) {
    mode += 1;
    mode %= MODECOUNT;
  }
  btnMerker = btnOn;
  
  // Flanke Sensor L
  if (sensorLOn != sensLMerker && sensorLOn) {
    if (state == States_OFF_R || state == States_OFF_L || state == States_TURN_OFF_R || state == States_TURN_OFF_L) {
      if (state == States_OFF_R) start = minStart;
      state = States_TURN_ON_L;
    } else if (state == States_ON || state == States_TURN_ON_R || state == States_TURN_ON_L) {
      state = States_TURN_OFF_L;
    }
  }
  sensLMerker = sensorLOn;
  
  // Flanke Sensor R
  if (sensorROn != sensRMerker && sensorROn) {
    if (state == States_OFF_L || state == States_OFF_R || state == States_TURN_OFF_L || state == States_TURN_OFF_R) {
      if (state == States_OFF_L) start = maxStart;
      state = States_TURN_ON_R;
    } else if (state == States_ON || state == States_TURN_ON_L || state == States_TURN_ON_R) {
      state = States_TURN_OFF_R;
    }
  }
  sensRMerker = sensorROn;

  /*if (lstate != state || llight != lightOn) {
    Serial.print(lightOn);
    Serial.print(" | ");

    switch(state) {
      case States_OFF_L:
          Serial.print("States_OFF_L");
        break;
      case States_OFF_R:
          Serial.print("States_OFF_R");
        break;
      case States_TURN_ON_L:
          Serial.print("States_TURN_ON_L");
        break;
      case States_TURN_ON_R:
          Serial.print("States_TURN_ON_R");
        break;
      case States_ON:
          Serial.print("States_ON");
        break;
      case States_TURN_OFF_L:
          Serial.print("States_TURN_OFF_L");
        break;
      case States_TURN_OFF_R:
          Serial.print("States_TURN_OFF_R");
        break;
    }

    Serial.print("\n");
  }//*/
  lstate = state;
  llight = lightOn;

  // LightOn macht heller oder dunkler
  if (lightOn) {
    if (light_val > 0.0D) {
      light_val -= 0.01D;
    } else {
      light_val = 0.0D;
    }
  } else {
    if (light_val < 1.0D) {
      light_val += 0.01D;
    } else {
      light_val = 1.0D;
    }
  }
  
  // Status Wechsel
  if (state == States_OFF_L) {
    if (start > minStart) {
      step = -STEP_SPEED;
    } else {
      step = 0;
    }
  } else if (state == States_OFF_R) {
    if (start < maxStart) {
      step = STEP_SPEED;
    } else {
      step = 0;
    }
  } else if (state == States_TURN_ON_L) {
    if (start >= midStart) {
      step = 0;
      state = States_ON;
    } else {
      step = STEP_SPEED;
    }
  } else if (state == States_TURN_ON_R) {
    if (start <= midStart) {
      step = 0;
      state = States_ON;
    } else {
      step = -STEP_SPEED;
    }
  } else if (state == States_ON) {
    if (start < midStart) {
      step = STEP_SPEED;
    } else if (start > midStart) {
      step = -STEP_SPEED;
    } else {
      step = 0;
    }
    on_timer += 1;
    if (on_timer > ON_MAX_TIME) {
      on_timer = 0;
      if (round(random(100,999)) % 2 == 0) {
        state = States_TURN_OFF_R;
      } else {
        state = States_TURN_OFF_L;
      }
    }
  } else if (state == States_TURN_OFF_L) {
    if (start <= minStart) {
      step = 0;
      state = States_OFF_L;
    } else {
      step = -STEP_SPEED;
    }
  } else if (state == States_TURN_OFF_R) {
    if (start >= maxStart) {
      step = 0;
      state = States_OFF_R;
    } else {
      step = STEP_SPEED;
    }
  }

  // Mode Action
	digitalWrite(LED_BUILTIN, (mode != MODE_NORMAL) ? HIGH : LOW);
  if (mode != MODE_NORMAL) {
    state = States_ON;
  }
  if (mode == MODE_RAINBOW) {
    rainbow = true;
  } else {
    rainbow = false;
  }
  
  if (state != States_ON) {
    on_timer = 0;
  }
  
  start += step;
  
  if (hc < h) {
    hc += 1;
  } else if (hc > h) {
    hc -= 1;
  } else {
    hc = h;
  }
  
  // Farb Berechnung
  for(uint16_t i = 0; i < LED_COUNT; i++) {
    int x = i;
    double v = s1(x, start);
    v *= 255.0D;
    v *= light_val;
    int hval = rainbow ? ((((hc - tick + i) % RAINBOW_LEN) / RAINBOW_LEN) * 255.0D) : hc;
    hval = ((double)i) * 4.0D;//128.0D;
    rgb col = hsv2rgb((double)(hval % 256), v, 0.0D);
    SetColor(i, col.r, col.g, col.b);
  }
  pixels.show();
  //delay(1);
}

double dblMod(double a, double b) {
	double rest = a;
	double times = floor(a / b);
	return rest - (times * b);
}

rgb hsv2rgb(double h, double s, double v) {
	// R -> RG
	// RG -> G
  // G -> GB
  // GB -> B
  // B -> BR
  // BR -> R

	double partL = 256.0D / 6.0D;
	int part = (int)floor(h / partL);
	double hh = dblMod(h, partL) / partL;
	double ss = s / 255.0D;
	double space = 255.0D * ss;
	double vv = v / 255.0D;
	double add = space * vv;

  double r = 0;
	double g = 0;
	double b = 0;

	switch(part) {
    case 0: // R -> RG
      r = v + space;
      g = v + (hh * space);
      b = v + add;
      break;
    case 1: // RG -> G
      r = v + space - (hh * space);
      g = v + space;
      b = v + add;
      break;
    case 2: // G -> GB
      r = v + add;
      g = v + space;
      b = v + (hh * space);
      break;
    case 3: // GB -> B
      r = v + add;
      g = v + space - (hh * space);
      b = v + space;
      break;
    case 4: // B -> BR
      r = v + (hh * space);
      g = v + add;
      b = v + space;
      break;
    case 5: // BR -> R
      r = v + space;
      g = v + add;
      b = v + space - (hh * space);
      break;
	}
  
  rgb out = {r, g, b};
  return out;     
}

void SetColor(uint16_t i, uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(i, pixels.Color(r, g, b));
}
