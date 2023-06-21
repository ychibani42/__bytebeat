#include <math.h>

/*
**  Tout les 15 ans, industriel ---> diminuition des feux de forets (tout les 50 ans)
**  vu qu'on empemche les feux de forets, on a plus de forets
*/

#include "wiring_private.h"
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

float x[] = {0, 65, 160, 180, 200, 220, 255};
float y[] = {10, 7, 5, 5, 6, 5, 4};
float z[] = {2, 2, 4, 6, 4, 5, 7};
int n = 7;

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
	where = buffer[unsigned(r)];
	rear = next(r);
	return 1;
}

static inline char forest_fire1(unsigned short a)
{
	unsigned short ust = t;
	return 64 * sin(sin(t / 100) - t / (2 + (t >> a & t >> 12) % 9)) + 128;
}

static inline char forest_fire2(unsigned short a)
{
	unsigned short ust = t;
	return 64 * sin(sin(t / 100) - t / (2 + (t >> 10 & t >> a) % 9)) + 128;
}

unsigned long previousTime = 0; // Stocker le temps précédent

void generate_samples()
{
	for (;;)
	{

	}
	unsigned long currentTime = millis(); // Obtenez le temps actuel
  
	char sample = forest_fire(3);
	if (!put(sample))
			break;
		t++;
  
  // Vérifier si l'intervalle de temps s'est écoulé pour appeler l'autre fonction
  if (currentTime - previousTime >= interval[currentIntervalIndex])
  {
    previousTime = currentTime; // Mettre à jour le temps précédent
    
    // Exécuter l'autre fonction pendant la durée spécifiée
    unsigned long endTime = currentTime + duration;
    while (millis() < endTime)
    {
      otherFunction();
    }
  }
  
  // Vérifier si le temps de changement d'intervalle est écoulé
  if (currentTime - intervalChangeTime >= interval[currentIntervalIndex])
  {
    currentIntervalIndex = (currentIntervalIndex + 1) % 7;  // Passer à l'intervalle suivant dans le tableau
    changeInterval(interval[currentIntervalIndex]); // Changer l'intervalle avec la nouvelle valeur
  }
}


void setup()
{
	// audio setup
	pinMode(A0, INPUT);
	pinMode(9, OUTPUT);
	
	Serial.begin(9600);

	sbi(TCCR2A, WGM20);
	sbi(TCCR2A, WGM21);
	TCCR2B = (1 << CS20); 
}

void loop()
{
	sensorValue = analogRead(A0);
	outputValue = map(sensorValue, 0, 1023, 0, 255);

	generate_samples();
	char c;
	if (get(&c))
		OCR2A = c; // set PWM duty cycle
}