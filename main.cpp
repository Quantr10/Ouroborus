#include <iostream>
#include <deque>
#include <vector>
#include <raylib.h>
#include <raymath.h>
#include <algorithm>

using namespace std;

Color black = {0, 0, 0, 255};
Color green = {0, 228, 48, 255};
Color red = {255, 0, 0, 255};

int cellSize = 25;
int cellCount = 25;
int offset = 30;
double lastUpdateTime = 0;
double initialTime = 0.2;
double updateTime = initialTime;
double speedIncrease = 0.003;

struct PowerUp 
{
    int type;
    int priority;
     bool operator<(const PowerUp& other) const {
        return priority < other.priority;
    }
};

bool CheckCollision(Vector2 element, deque<Vector2> deque)
{
    for (int i = 0; i < deque.size(); i++)
        if (Vector2Equals(deque[i], element)) return true;
    return false;
}

bool UpdateGameStatus(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
    public:
        deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        Vector2 direction = {1, 0};
        bool addSegment = false;

        void Draw()
        {
            for (int i = 0; i < body.size(); ++i)
            {
                const auto& segment = body[i];
                DrawRectangleRounded({offset + segment.x * cellSize, offset + segment.y * cellSize, (float)cellSize, (float)cellSize},0.5,6,green);
            }
        }

        void Update()
        {
            body.push_front(Vector2Add(body[0], direction));
            if (addSegment == true) addSegment = false;
            else body.pop_back();
        }

        void Reset()
        {
            body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
            direction = {1, 0};
        }
};

class Food
{
    public:
        Vector2 position = {5, 6};
        Texture2D texture;

        Food(const deque<Vector2>& snakeBody)
        {
            Image image = LoadImage("apple.png");
            ImageResizeNN(&image, cellSize, cellSize);
            texture = LoadTextureFromImage(image);
            UnloadImage(image);
            position = GenerateRandomPos(snakeBody);
        }

        ~Food() { UnloadTexture(texture); }
        
        void Draw()
        {
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }

        Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody)
        {
            Vector2 newPos;
            do {
                newPos = {(float)GetRandomValue(0, cellCount - 1), (float)GetRandomValue(0, cellCount - 1)};
            } while (CheckCollision(newPos, snakeBody));
            return newPos;
        }
};

class Obstacle
{
    public:
        Vector2 position;
        Color color;

        Obstacle(const deque<Vector2>& snakeBody, const Vector2& foodPosition)
        {
            color = red;
            GenerateValidPosition(snakeBody, foodPosition);
        }

        void GenerateValidPosition(const deque<Vector2>& snakeBody, const Vector2& foodPosition)
        {
            position = GenerateRandomValidPosition(snakeBody, foodPosition);
        }

        Vector2 GenerateRandomValidPosition(const deque<Vector2>& snakeBody, const Vector2& foodPosition) const 
        {
            Vector2 newPosition;
            do {
                newPosition = {(float)GetRandomValue(0, cellCount - 1), (float)GetRandomValue(0, cellCount - 1)};
            } while (CheckCollision(newPosition, snakeBody) && Vector2Equals(newPosition, foodPosition));
            return newPosition;
        }

        void Draw() const
        {
            DrawRectangle(offset + position.x * cellSize, offset + position.y * cellSize, cellSize, cellSize, color);
        }

        static void GenerateNewObstacles(deque<Obstacle>& obstacles, const deque<Vector2>& snakeBody, const Vector2& foodPosition)
        {
            obstacles.clear();
            int numObstacles = GetRandomValue(5, 10);
            for (int i = 0; i < numObstacles; ++i)
            {
                Obstacle obstacle(snakeBody, foodPosition);
                obstacles.push_back(obstacle);
            }
        }
};

class Game
{
    public:
        Snake snake = Snake();
        Food food = Food(snake.body);
        deque<Obstacle> obstacles;
        vector<PowerUp> powerUps;
        bool running = true;
        bool powerUpActive = false;
        int score = 0;
        int foodEaten = 0;
        int scoreToActivatePowerUp = GetRandomValue(10,15);
        int obstacleChangeScore = 5;

        void Draw()
        {
            food.Draw();
            snake.Draw();
            for (int i = 0; i < obstacles.size(); ++i) 
                obstacles[i].Draw();
        }

        void Update()
        {
            if (running)
            {
                snake.Update();
                CheckEatFood();
                CheckCollisionWithEdges();
                CheckCollisionWithTail();
                CheckCollisionWithObstacles();
            }
        }

        void AddPowerUp(int type, int priority) 
        {
            powerUps.push_back({type, priority});
            push_heap(powerUps.begin(), powerUps.end());
        }

        int UsePowerUp() 
        {
            if (!powerUps.empty()) 
            {
                int topPowerUpType = powerUps.front().type;
                powerUps.front().priority -= 1;
                make_heap(powerUps.begin(), powerUps.end());
                cout<<powerUps.size();
                for (int i = 0; i < powerUps.size(); ++i) {
                    const auto& powerUp = powerUps[i];
                    cout << "(" << powerUp.type << ", " << powerUp.priority << ") ";
                }
                cout << endl;
                return topPowerUpType;
                
            }
            return -1;
        }

        void CheckEatFood()
        {
            if (Vector2Equals(snake.body[0], food.position))
            {
                food.position = food.GenerateRandomPos(snake.body);
                snake.addSegment = true;
                foodEaten++;
                score++;
                int powerUpType = 0;
                if (foodEaten % scoreToActivatePowerUp == 0)
                {
                    scoreToActivatePowerUp = GetRandomValue(10,15);
                    powerUpType = UsePowerUp();
                    if (powerUpType == 1)
                    {
                        powerUpActive = true;
                        score += 5;
                        foodEaten = 0;
                    }
                    else if (powerUpType == 2)
                    {
                        powerUpActive = true;
                        updateTime *= 1.3;
                        foodEaten = 0;
                    }
                    else if (powerUpType == 3)
                    {
                        powerUpActive = true;
                        int currentLength = snake.body.size();
                        snake.body.erase(snake.body.begin() + currentLength/2, snake.body.end());
                        foodEaten = 0;
                    }
                }
                else
                {
                    updateTime -= speedIncrease;
                    if (updateTime < 0.05) updateTime = 0.05;
                }
                if (score % obstacleChangeScore == 0)
                    Obstacle::GenerateNewObstacles(obstacles, snake.body, food.position);
            }
        }

        void CheckCollisionWithEdges()
        {
            if (snake.body[0].x == cellCount) snake.body[0].x = 0;
            else if (snake.body[0].x == -1) snake.body[0].x = cellCount - 1;
            if (snake.body[0].y == cellCount) snake.body[0].y = 0;
            else if (snake.body[0].y == -1) snake.body[0].y = cellCount - 1;
        }

        void CheckCollisionWithTail()
        {
                deque<Vector2> headlessBody = snake.body;
                headlessBody.pop_front();
                if (CheckCollision(snake.body[0], headlessBody)) GameOver();
        }

        void CheckCollisionWithObstacles()
        {
            for (int i = 0; i < obstacles.size(); ++i) 
            {
                const auto &obstacle = obstacles[i];
                if (Vector2Equals(snake.body[0], obstacle.position))
                {
                    GameOver();
                    break;
                }
            }
        }

        void GameOver()
        {
            snake.Reset();
            food.position = food.GenerateRandomPos(snake.body);
            obstacles.clear();
            running = false;
            score = 0;
            foodEaten = 0;
            updateTime = initialTime;
            powerUps.clear();
            AddPowerUp(1, 6);
            AddPowerUp(2, 4);
            AddPowerUp(3, 2);
        }
};

int main()
{
    cout << "START GAME" << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Ouroboros");
    SetTargetFPS(120);
    Game game = Game();
    game.AddPowerUp(1, 6);
    game.AddPowerUp(2, 4);
    game.AddPowerUp(3, 2);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        if (UpdateGameStatus(updateTime))
        {
            game.Update();
        }
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
        {
            game.snake.direction = {0, -1};
            game.running = true;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
        {
            game.snake.direction = {0, 1};
            game.running = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
        {
            game.snake.direction = {-1, 0};
            game.running = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
        {
            game.snake.direction = {1, 0};
            game.running = true;
        }
        ClearBackground(black);
        DrawRectangleLinesEx({(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, red);
        DrawText("Ouroboros", offset - 5, 5, 20, red);
        DrawText("Score: ", offset - 5, offset + cellSize * cellCount + 5, 20, red);
        DrawText(TextFormat("%i", game.score), offset + 70, offset + cellSize * cellCount + 5, 20, red);
        DrawText("Speed: ", offset + 200, offset + cellSize * cellCount + 5, 20, red);
        DrawText(TextFormat("%.2f", 1.0 / updateTime), offset + 280, offset + cellSize * cellCount + 5, 20, red);
        DrawText("FoodEaten: ", offset + 450, offset + cellSize * cellCount + 5, 20, red);
        DrawText(TextFormat("%i", game.foodEaten), offset + 600, offset + cellSize * cellCount + 5, 20, red);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}