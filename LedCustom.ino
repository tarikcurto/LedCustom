#include <Arduino.h>
#include <math.h>

#define COLS 128
#define ROWS 32
#define COLOR_BITS 4

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

    Serial.begin(115200);

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

    resetColorsByStage();

    sendOutputEnable(true);
}

// Brightness
int brightnessPercentage = 30;
int brightnessCycleMax = 50;
int brightnessFraction = brightnessCycleMax / 100.0 * brightnessPercentage;
int brightnesPortion = round(brightnessCycleMax / brightnessFraction);
int brightnessCycle = 0;

// Color wheel
int colorStage = 5;
int colorStageAmplitude = 0;
const int colorStageAmplitudeMax = 10;

// Color
const int colorMaxValue = (1 << COLOR_BITS) - 1;
int currentColorLoop = 0;
const int totalColorLoops = colorMaxValue;

// Rotate
int boxWidth = 12;
int boxPosition = 0;
bool boxDirection = true;
int currentBoxLoop = 0;
const int totalBoxLoops = 8;

// Row animation
int currentRowLoop = 0;
int currentRow = 0;
int rowState = 0;
const int totalRowLoops = 100;
int boxHeight = boxWidth;

int red = 0;
int green = 0;
int blue = 0;

void loop()
{

    if (currentColorLoop == totalColorLoops)
    {
        currentColorLoop = 0;

        if (colorStageAmplitude == colorStageAmplitudeMax)
        {

            if (colorStage == 5)
            {
                colorStage = 0;
                resetColorsByStage();
            }
            else
                colorStage++;
            colorStageAmplitude = 0;
        }

        colorStageAmplitude++;
        setColorsByStageAmplitude(colorStage, colorStageAmplitude);

        switch (colorStage)
        {
        case 0:
            break;

        default:
            break;
        }
    }
    else
    {
        currentColorLoop++;
    }

    int redInterval = calculateColorInterval(totalColorLoops, red);
    int greenInterval = calculateColorInterval(totalColorLoops, green);
    int blueInterval = calculateColorInterval(totalColorLoops, blue);
    int redValue = calculateColorValue(currentColorLoop, redInterval);
    int greenValue = calculateColorValue(currentColorLoop, greenInterval);
    int blueValue = calculateColorValue(currentColorLoop, blueInterval);

    // Update box position based on direction
    currentBoxLoop = (currentBoxLoop + 1) % totalBoxLoops;
    if (currentBoxLoop == 0)
    {
        boxPosition += (boxDirection ? 1 : -1);

        // Check bounds and update direction if needed
        if (boxPosition + boxWidth >= COLS || boxPosition <= 0)
        {
            boxDirection = !boxDirection;
            boxPosition = boxDirection ? 0 : COLS - boxWidth;
        }
    }

    for (int row = 0; row < ROWS / 2; row++)
    {
        selectRow(row);

        // Update row based on current row loop
        currentRowLoop = (currentRowLoop + 1) % totalRowLoops;
        if (currentRowLoop == 0)
        {
            if (rowState == 0 && currentRow + boxHeight == ROWS)
            {
                rowState = 1;
            }
            else if (rowState == 1 && currentRow == 0)
            {
                rowState = 0;
            }

            switch (rowState)
            {
            case 0:
                currentRow++;
                break;
            case 1:
                currentRow--;
                break;
            default:
                break;
            }
        }

        for (int col = 0; col < COLS; col++)
        {
            brightnessCycle = (brightnessCycle + 1) % brightnessCycleMax;
            if (brightnessCycle != 0)
                sendOutputEnable(brightnessCycle % brightnesPortion == 0);
            else
                sendOutputEnable(true);

            if (col >= boxPosition && col <= boxPosition + boxWidth)
            {

                int upperRow = row;
                int lowerRow = row + ROWS / 2;

                if (upperRow >= currentRow && upperRow < currentRow + boxHeight)
                    sendColorDataUpper(redValue, greenValue, blueValue);
                else
                    sendColorDataUpper(0, 0, 0);

                if (lowerRow >= currentRow && lowerRow < currentRow + boxHeight)
                    sendColorDataLower(redValue, greenValue, blueValue);
                else
                    sendColorDataLower(0, 0, 0);
            }
            else
            {
                sendColorDataUpper(0, 0, 0);
                sendColorDataLower(0, 0, 0);
            }

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
    float numberOfActions = (totalLoops * actionPercentage) / 100.0;
    if (numberOfActions == 0)
        return -1;
    return round(totalLoops / numberOfActions);
}

int calculateColorInterval(int totalLoops, int color)
{
    float fraction = totalLoops / (float)colorMaxValue;
    float colorFraction = color * fraction;
    float percentage = colorFraction / totalLoops * 100;
    return calculateActionInterval(totalLoops, (int)percentage);
}

void resetColorsByStage()
{
    red = colorMaxValue;
    green = 0;
    blue = 0;
}

void setColorsByStageAmplitude(int colorStage, int colorStageAmplitude)
{
    int ascendingColor, descendingColor;
    if (colorStageAmplitude == colorStageAmplitudeMax)
    {
        ascendingColor = colorMaxValue;
        descendingColor = 0;
    }
    else
    {
        int stepSize = (int)(colorMaxValue / colorStageAmplitudeMax);
        ascendingColor = stepSize * colorStageAmplitude;
        descendingColor = colorMaxValue - (stepSize * colorStageAmplitude);
    }

    switch (colorStage)
    {
    case 0:
        // Add blue
        blue = ascendingColor;
        break;

    case 1:
        // Remove red
        red = descendingColor;
        break;

    case 2:
        // Add green
        green = ascendingColor;
        break;

    case 3:
        // Remove blue
        blue = descendingColor;
        break;

    case 4:
        // Add red
        red = ascendingColor;
        break;

    case 5:
        // Remove green
        green = descendingColor;
        break;

    default:
        break;
    }
}

void selectRow(int row)
{
    if (row == 0)
        row = ROWS / 2 - 1;
    else
        row--;

    digitalWrite(A, row & 0x01);
    digitalWrite(B, row & 0x02);
    digitalWrite(C, row & 0x04);
    digitalWrite(D, row & 0x08);
    #if ROWS > 32
    digitalWrite(E, row & 0x10);
    #endif
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
    sendOutputEnable(false);
    // This delay might need adjustment
    //delayMicroseconds(100);
    sendOutputEnable(true);
}
