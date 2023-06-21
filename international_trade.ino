#include <avr/io.h>
#include "wiring_private.h"
#include <math.h>

// Pin 9 - PWM output
#define PWM_PIN 9

#undef round

long t = 0;
char i = 0;

const int buffer_log_size = 7;
static unsigned char buffer[1 << buffer_log_size];
typedef unsigned char bufidx;
static bufidx front = 0;
static volatile bufidx rear = 0;

int sensorValue = 0;
int outputValue = 0;

// Points pour l'interpolation polynomiale
float x[] = {0, 5, 10, 15, 20, 50, 100, 130, 155, 191, 255};
float y[] = {1, 3, 5, 12, 20, 400, 3300, 8000, 11000, 32000, 96000};

int n = 11;

static inline unsigned char next(unsigned char cur)
{
  return (cur + 1) & ((1 << buffer_log_size) - 1);
}

static inline char put(char v)
{
  if (rear == next(front))
    return 0;
  buffer[unsigned(front)] = v;
  front = next(front);
  return 1;
}

static inline char get(char *where)
{
  register bufidx r = rear;
  if (r == front)
    return 0;
  *where = buffer[unsigned(r)];
  rear = next(r);
  return 1;
}

static inline char triangle_bells()
{
  char f = (unsigned char)((unsigned char)((unsigned char)(t >> 11) % (unsigned char)63 ^ (0x15 + t >> 12)) % 102) % 134;
  unsigned short ust = t;
  return (((((ust * f & 256) == 0) - 1 ^ ust * f) & 255) >> ((ust >> 7 + (ust >> 13 & 1)) & 7));
}

static inline char international_trade()
{
  unsigned short ust = t;
  return (ust * ust) * ((ust >> 12) + 1) | ust & ust >> 10 | ust & ust >> 6 | ((unsigned int)pow(((ust >> 9) % 128 > 114 ? ust & 4095 : ust & 8191), 1.0 / 3) * 384 | (ust >> 13 & 1 ? ust * 64 : 0)) & 128;
}

void generate_samples()
{
  for (;;)
  {
    char sample = international_trade();
    if (!put(sample))
      break;
    t++;
  }
}

void setup()
{
  pinMode(A0, INPUT);
  pinMode(PWM_PIN, OUTPUT);
  Serial.begin(9600);

  // Configure Timer 2
  // Fast PWM mode, TOP=0xFF (8-bit), prescaler=1
  sbi(TCCR2A, WGM20);
  sbi(TCCR2A, WGM21);
  TCCR2B = (1 << CS20);
}

void loop()
{
  // read the analog in value:
  sensorValue = analogRead(A0);

  // map it to the range of the analog out:
  outputValue = map(sensorValue, 0, 1023, 0, 255);

  // calculate the interpolated value:
  float interpolatedValue = interpolate(outputValue);

  // change the analog out value:
  analogWrite(PWM_PIN, outputValue);

  // print the results to the serial monitor:

  Serial.print("analog = ");
  Serial.print(sensorValue);
  Serial.print("\t digital = ");
  Serial.print(outputValue);
  Serial.print("\t interpolated = ");
  Serial.println(interpolatedValue);
  // wait 2 milliseconds before the next loop for the
  // analog-to-digital converter to settle after the
  // last reading:
  delay(2); // Wait for 2 millisecond(s)

  // Generate sine wave
  for (int i = 0; i < 256; i++)
  {
    analogWrite(PWM_PIN, 127 + 127 * sin(i * 2 * PI / 256));
    delay(1);
  }
}

// Fonction d'interpolation polynomiale
float interpolate(float xValue)
{
  float result = 0;
  for (int i = 0; i < n; i++)
  {
    float term = y[i];
    for (int j = 0; j < n; j++)
    {
      if (j != i)
      {
        term = term * (xValue - x[j]) / (x[i] - x[j]);
      }
    }
    result += term;
  }
  return result;
}
