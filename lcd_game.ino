#include <LiquidCrystal.h>

//button
const int buttonPin = 9;
int previousState = 0;
int buttonState = 0;
bool wasButtonPressed = false;

//lcd
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//player
byte player[8] = {
    B00000,
    B00000,
    B00100,
    B01110,
    B00100,
    B00100,
    B01010,
    B00000,
};

int playerCharReference = 0;
int playersPosition = 0;

//obstacles
byte block[8] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
};

int blockCharReference = 1;

struct Obstacle
{
    int row = -1;
    int col = -1;
    int length = -1;
    bool deleted = true;
};

const int maxObstaclesNumber = 10;
Obstacle obstacles[maxObstaclesNumber];

void setup()
{
    Serial.begin(9600);
    pinMode(buttonPin, INPUT);
    lcd.begin(16, 2);

    initializeCustomChars();
    setFirstObstacles();
    drawPlayerOnRow(0);
}

void initializeCustomChars()
{
    lcd.createChar(playerCharReference, player);
    lcd.createChar(blockCharReference, block);
}

void setFirstObstacles()
{
    obstacles[0].row = 0;
    obstacles[0].col = 13;
    obstacles[0].length = 3;
    obstacles[0].deleted = false;

    obstacles[1].row = 1;
    obstacles[1].col = 9;
    obstacles[1].length = 2;
    obstacles[1].deleted = false;
}

int points = 0;
bool gameOver = false;

void loop()
{
    if (!gameOver)
    {
        readButtonState();
        movePlayerIfButtonWasPressed();
        drawPresentObstacles();
        detectPotentialCollision();
    }
}

void readButtonState()
{
    buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH && previousState == LOW)
    {
        wasButtonPressed = true;
    }
    previousState = buttonState;
}

void movePlayerIfButtonWasPressed()
{
    if (wasButtonPressed)
    {
        clearPosition(playersPosition);

        if (playersPosition == 0)
        {
            drawPlayerOnRow(1);
        }
        else
        {
            drawPlayerOnRow(0);
        }

        playersPosition = !playersPosition;
    }

    wasButtonPressed = false;
}

void drawPlayerOnRow(int row)
{
    lcd.setCursor(0, row);
    lcd.write(playerCharReference);
}

void clearPosition(int row)
{
    lcd.setCursor(0, row);
    lcd.write(" ");
}

int previousBlockRefresh = 0;

void drawPresentObstacles()
{
    int currentTime = millis();
    if (isTimeToRefreshScreen(currentTime))
    {
        advanceObstacles();

        //draw every not deleted obstacle
        for (int i = 0; i < maxObstaclesNumber; i++)
        {
            if (!obstacles[i].deleted)
            {
                for (int j = 0; j < obstacles[i].length; j++)
                {
                    drawBlockOnPosition(obstacles[i].row, obstacles[i].col + j);
                }
            }
        }
        generateNextObstaclesWhenPossible();

        previousBlockRefresh = currentTime;
        points++;
    }
}

//refreshing the screen every half second
bool isTimeToRefreshScreen(int currentTime)
{
    return (currentTime - previousBlockRefresh) > 150;
}

//shifting every obstacle by one column to the left
void advanceObstacles()
{
    for (int i = 0; i < maxObstaclesNumber; i++)
    {
        if (!obstacles[i].deleted)
        {
            for (int j = 0; j < obstacles[i].length; j++)
            {
                clearBlockOnPosition(obstacles[i].row, obstacles[i].col + j);
            }

            obstacles[i].col--;
            if (obstacles[i].col + obstacles[i].length == 0)
            {
                setObstaclesAsDeleted(i);
            }
        }
    }
}

//setting obstacles as deleted when it leaves the screen
void setObstaclesAsDeleted(int i)
{
    obstacles[i].deleted = true;
}

void drawBlockOnPosition(int row, int col)
{
    lcd.setCursor(col, row);
    lcd.write(blockCharReference);
}

void clearBlockOnPosition(int row, int col)
{
    lcd.setCursor(col, row);
    lcd.write(" ");
}

void detectPotentialCollision()
{
    for (int i = 0; i < maxObstaclesNumber; i++)
    {
        if ((obstacles[i].col == 0 || obstacles[i].col + obstacles[i].length == 1) && obstacles[i].row == playersPosition)
        {
            setGameAsOver();
        }
    }
}

void setGameAsOver()
{
    gameOver = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KONIEC GRY");
    lcd.setCursor(0, 1);
    lcd.print("PUNKTY: ");
    lcd.setCursor(8, 1);
    lcd.print(points);
}

void generateNextObstaclesWhenPossible()
{
    int freeBlocksPercentage = getFreeBlocksPercentage();
    Serial.println(freeBlocksPercentage);
    if (freeBlocksPercentage > 60)
    {
        generateNewObstacle();
    }
}

void generateNewObstacle()
{
    bool found = false;
    for (int i = 0; i < maxObstaclesNumber; i++)
    {
        if (obstacles[i].deleted && !found)
        {
            found = true;
            int randomRow = random(0, 2);
            int lastFreeColumn = getLastUsedColumn();
            int randomDistanceFromLastBlock = random(2, 5);
            obstacles[i].row = randomRow;
            obstacles[i].col = lastFreeColumn + randomDistanceFromLastBlock;
            obstacles[i].length = random(1, 7);
            obstacles[i].deleted = false;
            break;
        }
    }
}

int getLastUsedColumn()
{
    int maxColumn = 0;
    for (int i = 0; i < maxObstaclesNumber; i++)
    {
        if (!obstacles[i].deleted)
        {
            if ((obstacles[i].col + obstacles[i].length) > maxColumn)
            {
                maxColumn = obstacles[i].col + obstacles[i].length;
            }
        }
    }

    return maxColumn;
}

int getFreeBlocksPercentage()
{
    int usedBlocksNumber = 0;
    for (int i = 0; i < maxObstaclesNumber; i++)
    {
        if (!obstacles[i].deleted)
        {
            usedBlocksNumber += obstacles[i].length;
        }
    }
    int freeBlocksNumber = 30 - usedBlocksNumber;

    return freeBlocksNumber / 30.0 * 100;
}
