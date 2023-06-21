#include "wiring_private.h"
#undef round

long t = 0;
char i = 0;

int sensorValue = 0;
int outputValue = 0;
const int buffer_log_size = 7;
static unsigned char buffer[1 << buffer_log_size];
typedef unsigned char bufidx;
static bufidx front = 0;
static volatile bufidx rear = 0;

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

static inline char simple_cigale()
{
   unsigned short ust = t;
   return (ust * (((ust >> 1) | (ust >> 1)) & (10000000 & (ust >> 4))));
}

static inline char double_cigale()
{
    unsigned short ust = t;
    return (ust * (((ust >> 1) | (ust ^ 1)) & (120000 & (ust >> 4))));
}

static inline char triple_cigale()
{
    unsigned short ust = t;
    return (ust * (((ust >> 1) | (ust ^ 3)) & (140000 & (ust >> 4))));
}

void generate_samples()
{
    for (;;)
    {
        if (sensorValue < 145)
        {
            char sample = simple_cigale();
            if (!put(sample))
                break;
            t++;
        }
        else if (sensorValue > 145 && sensorValue < 220)
        {
            char sample = double_cigale();
            if (!put(sample))
                break;
            t++;
        }
        else
        {
            char sample = triple_cigale();
            if (!put(sample))
                break;
            t++;
        }
    }
}

void setup()
{
    // audio setup
    pinMode(11, OUTPUT);
    pinMode(A0, INPUT);

    // connect pwm to pin on timer 2
    sbi(TCCR2A, COM2A1);
    TCCR2B = TCCR2B & 0xf8 | 0x01; // no prescaling on clock select
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