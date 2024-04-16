#include <iostream>
#include <deque>
#include <vector>
#include <raylib.h>
#include <raymath.h>

using namespace std;

Color black = {0, 0, 0, 255};
Color green = {0, 228, 48, 255};
Color red = {255, 0, 0, 255};

int cellSize = 25;
int cellCount = 25;
int offset = 30;
double lastUpdateTime = 0;
double initialUpdateInterval = 0.15;
double updateInterval = initialUpdateInterval;
double speedIncreaseFactor = 0.02;

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
            for (const auto &segment : body)
            {
                DrawRectangleRounded({offset + segment.x * cellSize, offset + segment.y * cellSize, (float)cellSize, (float)cellSize}, 0.5, 6, green);
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

        Vector2 GenerateRandomCell()
        {
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{x, y};
        }

        Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody)
    {
        Vector2 newPos;
        do
        {
            newPos = GenerateRandomCell();
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
            GenerateValidPosition(snakeBody, foodPosition);
            color = red;
        }

        void GenerateValidPosition(const deque<Vector2>& snakeBody, const Vector2& foodPosition)
        {
            int numObstacles = GetRandomValue(1, 4);
            for (int i = 0; i < numObstacles; ++i)
            {
                Vector2 newPosition;
                do
                {
                    newPosition = GenerateRandomCell();
                } while (CheckCollision(newPosition, snakeBody) || Vector2Equals(newPosition, foodPosition));
                
                position = newPosition;
            }
        }

        Vector2 GenerateRandomCell()
        {
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{x, y};
        }

        void Draw() const
        {
            DrawRectangle(offset + position.x * cellSize, offset + position.y * cellSize, cellSize, cellSize, color);
        }

        static void GenerateNewObstacles(deque<Obstacle>& obstacles, const deque<Vector2>& snakeBody, const Vector2& foodPosition)
    {
        obstacles.clear();
        int numObstacles = GetRandomValue(1, 4);
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
        bool running = true;
        int score = 0;
        int obstacleChangeScore = 10;

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

        void CheckEatFood()
        {
            if (Vector2Equals(snake.body[0], food.position))
            {
                food.position = food.GenerateRandomPos(snake.body);
                snake.addSegment = true;
                score++;
                updateInterval -= speedIncreaseFactor * initialUpdateInterval;
                if (updateInterval < 0.05) updateInterval = 0.05;
                if (score % obstacleChangeScore == 0) Obstacle::GenerateNewObstacles(obstacles, snake.body, food.position);
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
        updateInterval = initialUpdateInterval;
    }
};

int main()
{
    cout << "START GAME" << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Ouroboros");
    SetTargetFPS(120);
    Game game = Game();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        if (UpdateGameStatus(updateInterval))
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
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
