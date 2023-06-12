#include "wiring_private.h"
#undef round

long t = 0;
char i = 0;

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

/*
**      Formule de test
*/

static inline char triangle_bells()
{
    char f = (unsigned char)((unsigned char)((unsigned char)(t >> 11) % (unsigned char)63 ^ (0x15 + t >> 12)) % 102) % 134;
    unsigned short ust = t;
    return (((((ust f & 256) == 0) - 1 ^ ust * f) & 255) >> ((ust >> 7 + (ust >> 13 & 1)) & 7));
}

/*
**  Formule pour le nombre d'insectes a travers le temps
*/

static inline char triangle_bells2()
{
    unsigned short ust = t;
    return (ust * (((ust >> 1) | (ust >> 1)) & (0x80 & (ust >> 4)))) >> ((ust >> 7 + (ust >> 13 & 1)) & 7);
}

/*
**  Formule pour les echanges commerciaux
*/

static inline char triangle_bells3()
{
    unsigned short ust = t;
    return (char) ((ust * ust) & ((ust >> 12) + 1) | (ust & (ust >> 10)) | (ust & (ust >> 6)) | (cbrt(((ust >> 9) % 128 > 114 ? ust & 4095 : ust & 8191)) / 384 | (ust >> 13 & 1 ? ust / 64 : 0)) & 128);
}

void generate_samples()
{
    for (;;)
    {
        char sample = triangle_bells2();
        if (!put(sample))
            break;
        t++;
    }
}

void setup()
{
    // audio setup
    pinMode(11, OUTPUT);

    // connect pwm to pin on timer 2
    sbi(TCCR2A, COM2A1);
    TCCR2B = TCCR2B & 0xf8 | 0x01; // no prescaling on clock select
}

void loop()
{
    generate_samples();
    char c;
    if (get(&c))
        OCR2A = c; // set PWM duty cycle
}