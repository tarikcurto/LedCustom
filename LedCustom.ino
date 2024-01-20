#include <Arduino.h>
#include <math.h>

// Define pin connections
const int R1 = 25;
const int G1 = 27;
const int _B1 = 26;
const int R2 = 14;
const int G2 = 13;
const int B2 = 12;
const int A = 23;
const int B = 22;
const int C = 5;
const int D = 17;
const int E = 32;
const int OE = 15;
const int LAT = 4;
const int CLK = 16;

void setup()
{

    // Serial.begin(9600);

    // Initialize all pins as outputs
    pinMode(R1, OUTPUT);
    pinMode(G1, OUTPUT);
    pinMode(_B1, OUTPUT);
    pinMode(R2, OUTPUT);
    pinMode(G2, OUTPUT);
    pinMode(B2, OUTPUT);
    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(C, OUTPUT);
    pinMode(D, OUTPUT);
    pinMode(E, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(LAT, OUTPUT);
    pinMode(CLK, OUTPUT);

    sendOutputEnable(true);
}

// void loop(){}

bool oe = true;
int currentLoop = 0;
int totalLoops = 60;

int red = 251;
int green = 139;
int blue = 0;

int redInterval = calculateColorInterval(totalLoops, red);
int greenInterval = calculateColorInterval(totalLoops, green);
int blueInterval = calculateColorInterval(totalLoops, blue);

void loop()
{
    currentLoop = currentLoop == totalLoops ? 1 : currentLoop + 1;
    for (int row = 0; row < 16; row++)
    {
        selectRow(row);
        for (int col = 0; col < 64; col++)
        {
            oe = !oe;
            sendOutputEnable(oe);

            if (col > 26 && col < 38)
            {
                int redValue = calculateColorValue(currentLoop, redInterval);
                int greenValue = calculateColorValue(currentLoop, greenInterval);
                int blueValue = calculateColorValue(currentLoop, blueInterval);
                sendColorDataUpper(redValue, greenValue, blueValue);
            }
            else
            {
                sendColorDataUpper(0, 0, 0);
            }

            sendColorDataLower(0, 0, 0);

            clockPulse();
        }
        latchData();
    }
}

int calculateColorValue(int currentLoop, int colorInterval)
{
    if (colorInterval == -1)
        return 0;
    return currentLoop % colorInterval == 0 ? 1 : 0;
}

int calculateActionInterval(int totalLoops, int actionPercentage)
{
    float numberOfActions = (totalLoops * actionPercentage) / 100;
    if (numberOfActions == 0)
        return -1;
    return round(totalLoops / numberOfActions);
}

int calculateColorInterval(int totalLoops, int color)
{
    float fraction = totalLoops / 255.0;
    float colorFraction = color * fraction;
    float percentage = colorFraction / totalLoops * 100;
    return calculateActionInterval(totalLoops, (int)percentage);
}

void selectRow(int row)
{
    digitalWrite(A, row & 0x01);
    digitalWrite(B, row & 0x02);
    digitalWrite(C, row & 0x04);
    digitalWrite(D, row & 0x08);
    digitalWrite(E, row & 0x10);
}

void sendColorDataUpper(byte red, byte green, byte blue)
{
    digitalWrite(R1, red);
    digitalWrite(G1, green);
    digitalWrite(_B1, blue);
}

void sendColorDataLower(byte red, byte green, byte blue)
{
    digitalWrite(R2, red);
    digitalWrite(G2, green);
    digitalWrite(B2, blue);
}

void sendOutputEnable(bool enable)
{
    digitalWrite(OE, !enable);
}

void clockPulse()
{
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);
}

void latchData()
{
    digitalWrite(LAT, HIGH);
    digitalWrite(LAT, LOW);
    // After latching, turn off the output briefly to avoid ghosting
    digitalWrite(OE, HIGH);
    delayMicroseconds(100); // This delay might need adjustment
    digitalWrite(OE, LOW);
}
