int sensorValue = 0;
int outputValue = 0;

// Points pour l'interpolation polynomiale
float x[] = {0, 5, 10, 15, 20, 50, 100, 130, 155, 191, 255};
float y[] = {1, 3, 5, 12, 20, 400, 3300, 8000, 11000, 32000, 96000};
int n = 11;

void setup()
{
    pinMode(A0, INPUT);
    pinMode(9, OUTPUT);
    Serial.begin(9600);
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
    analogWrite(9, outputValue);
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