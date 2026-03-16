#include <iostream>
#include <raylib.h>
#include <deque>
#include <vector>
#include <cmath>
#include <raymath.h>

using namespace std;

static bool allowMove = false; // Makes it so that the snake doesn't move until I move it
// Some colors for the game
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color wallGreen = {90, 110, 50, 255};
Color poisonRed = {200, 40, 40, 255};
// Screen and grid settings
int cellSize = 30;
int cellCount = 25;
int offset = 75;
// Function to check if a Vector2 element is in a deque of Vector2
bool ElementInDeque(Vector2 element, deque<Vector2> deque){
    for (unsigned int i = 0; i < deque.size(); i++){
        if (Vector2Equals(deque[i], element)){
            return true;
        }
    }
    return false;
}
// Function to check if a Vector2 element is in a vector of Vector2
bool ElementInVector(Vector2 element, const vector<Vector2>& vec){
    for (unsigned int i = 0; i < vec.size(); i++){
        if (Vector2Equals(vec[i], element)){
            return true;
        }
    }
    return false;
}
// Enum for the different screen states
enum class ScreenState{
    Menu,
    Playing
};
// Everything that has to do with the snake
class Snake{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    deque<Vector2> previousBody = body;
    Vector2 direction = {1, 0};
    bool addSegment = false;
    Texture2D headUp;
    Texture2D headDown;
    Texture2D headLeft;
    Texture2D headRight;
    Texture2D tailUp;
    Texture2D tailDown;
    Texture2D tailLeft;
    Texture2D tailRight;
    Texture2D bodyHorizontal;
    Texture2D bodyVertical;
    Texture2D bodyTopLeft;
    Texture2D bodyTopRight;
    Texture2D bodyBottomLeft;
    Texture2D bodyBottomRight;
    // Makes the snakes body segment match the correct size and draws it to the screen
    void DrawScaledTexture(Texture2D texture, int drawX, int drawY){
        Rectangle source = Rectangle{0.0f, 0.0f, (float)texture.width, (float)texture.height};
        Rectangle dest = Rectangle{(float)drawX, (float)drawY, (float)cellSize, (float)cellSize};
        Vector2 origin = Vector2{0.0f, 0.0f};
        DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
    }
    // Snake textures constructor
    Snake(){
        headUp = LoadTexture("Graphics/head_up.png");
        headDown = LoadTexture("Graphics/head_down.png");
        headLeft = LoadTexture("Graphics/head_left.png");
        headRight = LoadTexture("Graphics/head_right.png");
        tailUp = LoadTexture("Graphics/tail_up.png");
        tailDown = LoadTexture("Graphics/tail_down.png");
        tailLeft = LoadTexture("Graphics/tail_left.png");
        tailRight = LoadTexture("Graphics/tail_right.png");
        bodyHorizontal = LoadTexture("Graphics/body_horizontal.png");
        bodyVertical = LoadTexture("Graphics/body_vertical.png");
        bodyTopLeft = LoadTexture("Graphics/body_tl.png");
        bodyTopRight = LoadTexture("Graphics/body_tr.png");
        bodyBottomLeft = LoadTexture("Graphics/body_bl.png");
        bodyBottomRight = LoadTexture("Graphics/body_br.png");
    }
    // Snake textures destructor
    ~Snake(){
        UnloadTexture(headUp);
        UnloadTexture(headDown);
        UnloadTexture(headLeft);
        UnloadTexture(headRight);
        UnloadTexture(tailUp);
        UnloadTexture(tailDown);
        UnloadTexture(tailLeft);
        UnloadTexture(tailRight);
        UnloadTexture(bodyHorizontal);
        UnloadTexture(bodyVertical);
        UnloadTexture(bodyTopLeft);
        UnloadTexture(bodyTopRight);
        UnloadTexture(bodyBottomLeft);
        UnloadTexture(bodyBottomRight);
    }
    // Replaces the snake head texture to match the direction it's moving in
    Texture2D GetHeadTexture(){
        if (direction.x == 1) return headRight;
        if (direction.x == -1) return headLeft;
        if (direction.y == 1) return headDown;
        return headUp;
    }
    // Gets the direction from the difference of two positions, but only returns the dominant direction (the one with the biggest absolute value)
    Vector2 GetDominantDirection(Vector2 delta){
        if (fabs(delta.x) >= fabs(delta.y)){
            return Vector2{delta.x >= 0 ? 1.0f : -1.0f, 0.0f};
        }
        return Vector2{0.0f, delta.y >= 0 ? 1.0f : -1.0f};
    }
    // Replaces the snake tail texture to match the direction it's moving in
    Texture2D GetTailTextureForPositions(Vector2 beforeTail, Vector2 tail){
        Vector2 diff = GetDominantDirection(Vector2Subtract(tail, beforeTail));
        if (diff.x == 1) return tailRight;
        if (diff.x == -1) return tailLeft;
        if (diff.y == 1) return tailDown;
        return tailUp;
    }
    // Replaces the snake body segment texture to match the direction it's moving in
    Texture2D GetBodyTexture(Vector2 prev, Vector2 current, Vector2 next){
        Vector2 prevDir = GetDominantDirection(Vector2Subtract(prev, current));
        Vector2 nextDir = GetDominantDirection(Vector2Subtract(next, current));

        if ((prevDir.x != 0 && nextDir.x != 0) || fabs(prev.y - next.y) < 0.01f) return bodyHorizontal;
        if ((prevDir.y != 0 && nextDir.y != 0) || fabs(prev.x - next.x) < 0.01f) return bodyVertical;

        bool up = (prevDir.y < 0) || (nextDir.y < 0);
        bool down = (prevDir.y > 0) || (nextDir.y > 0);
        bool left = (prevDir.x < 0) || (nextDir.x < 0);
        bool right = (prevDir.x > 0) || (nextDir.x > 0);

        if (up && left) return bodyTopLeft;
        if (up && right) return bodyTopRight;
        if (down && left) return bodyBottomLeft;
        return bodyBottomRight;
    }
    // Makes the snake move smoothly 
    Vector2 GetInterpolatedPosition(unsigned int index, float alpha){
        Vector2 endPos = body[index];
        Vector2 startPos;

        if (previousBody.empty()){
            startPos = endPos;
        }
        else if (index < previousBody.size()){
            startPos = previousBody[index];
        }
        else{
            startPos = previousBody.back();
        }

        return Vector2Lerp(startPos, endPos, alpha);
    }
    // Draws the snake to the screen with the correct textures and interpolated positions
    void Draw(float alpha){
        for (unsigned int i = 0; i < body.size(); i++){
            Vector2 drawPos = GetInterpolatedPosition(i, alpha);
            float x = drawPos.x;
            float y = drawPos.y;
            int drawX = offset + (int)roundf(x * cellSize);
            int drawY = offset + (int)roundf(y * cellSize);

            if (i == 0){
                DrawScaledTexture(GetHeadTexture(), drawX, drawY);
            }
            else if (i == body.size() - 1){
                Vector2 tailStart = GetInterpolatedPosition(i - 1, alpha);
                DrawScaledTexture(GetTailTextureForPositions(tailStart, drawPos), drawX, drawY);
            }
            else{
                Vector2 prevPos = GetInterpolatedPosition(i - 1, alpha);
                Vector2 nextPos = GetInterpolatedPosition(i + 1, alpha);
                Texture2D segmentTexture = GetBodyTexture(prevPos, drawPos, nextPos);
                DrawScaledTexture(segmentTexture, drawX, drawY);
            }
        }
    }
    // Updates the snakes position and body segments, and adds a segment if it has just eaten food
    void Update(){
        previousBody = body;
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true){
            addSegment = false;
        }
        else{
            body.pop_back();
        }
    }
    // Resets the snake to its starting position and direction
    void Reset(){
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        previousBody = body;
        direction = {1, 0};
    }
};
// Everything that has to do with the food
class Food{

public:
    Vector2 position;
    Texture2D texture;
    // Food constructor, spawns the food in a random place except the snake body and loads the food texture
    Food(deque<Vector2> snakeBody){
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }
    // Food destructor, unloads the food texture
    ~Food(){
        UnloadTexture(texture);
    }
    // Draws the food to the screen
    void Draw(){
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }
    // Makes walls and poison fruit
    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }
    // Spawns foods in a random place avoiding the snake body and the rest
    Vector2 GenerateRandomPos(deque<Vector2> snakeBody){
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody)){
            position = GenerateRandomCell();
        }
        return position;
    }
};
// Everything that has nothing to do with the snake and the fruit
class Game{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    int stage = 1;
    int applesThisStage = 0;
    int stageGoals[3] = {5, 5, 5};
    double moveSpeed = 0.2;
    vector<Vector2> walls;
    bool poisonActive = false;
    Vector2 poisonPos = {0, 0};
    Sound eatSound;
    Sound wallSound;
    // Game constructor
    Game(){
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }
    // Game destructor
    ~Game(){
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }
    // Draws everything that needs to be drawn to the screen
    void Draw(float alpha){
        food.Draw();
        DrawWalls();
        DrawPoison();
        snake.Draw(alpha);
    }
    // Updates the game logic, checks for collisions and updates the snake position
    void Update(){
        if (running){
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithWalls();
            CheckCollisionWithTail();
        }
    }
    // Checks if a cell is blocked for the bot to move into
    bool IsCellBlockedForMove(Vector2 pos){
        if (pos.x < 0 || pos.x >= cellCount || pos.y < 0 || pos.y >= cellCount) return true;
        deque<Vector2> bodyToCheck = snake.body;
        if (!snake.addSegment && !bodyToCheck.empty()){
            bodyToCheck.pop_back();
        }
        if (ElementInDeque(pos, bodyToCheck)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }
    // Makes the bot move
    Vector2 ChooseBotDirection(){
        Vector2 options[4] = {Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}};
        Vector2 bestDir = snake.direction;
        int bestScore = 1000000;

        for (int i = 0; i < 4; i++){
            Vector2 dir = options[i];
            if (Vector2Equals(dir, Vector2Scale(snake.direction, -1))) continue;

            Vector2 nextPos = Vector2Add(snake.body[0], dir);
            if (IsCellBlockedForMove(nextPos)) continue;

            int score = (int)(abs((int)(food.position.x - nextPos.x)) + abs((int)(food.position.y - nextPos.y)));
            if (score < bestScore){
                bestScore = score;
                bestDir = dir;
            }
        }

        return bestDir;
    }
    // Gets the current stage goal, making sure to not go out of bounds of the stageGoals array
    int CurrentStageGoal() const {
        int index = stage - 1;
        if (index < 0) index = 0;
        if (index > 2) index = 2;
        return stageGoals[index];
    }
    // Generates a random cell position for the walls and poison fruit to spawn in
    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }
    // Checks if a cell is blocked by the snake body
    bool IsCellBlocked(Vector2 pos, const deque<Vector2>& snakeBody, bool checkFood, bool checkPoison){
        if (ElementInDeque(pos, snakeBody)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (checkFood && Vector2Equals(pos, food.position)) return true;
        if (checkPoison && poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }
    // Generates a random position for the walls and poison fruit to spawn in, making sure they don't spawn on the snake body, food, or each other
    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody, bool checkFood, bool checkPoison){
        Vector2 position = GenerateRandomCell();
        while (IsCellBlocked(position, snakeBody, checkFood, checkPoison)){
            position = GenerateRandomCell();
        }
        return position;
    }
    // Relocates the food to a new random position, making sure it doesn't spawn on the snake body, walls, or poison fruit
    void RelocateFood(){
        food.position = food.GenerateRandomPos(snake.body);
        while (ElementInVector(food.position, walls) || (poisonActive && Vector2Equals(food.position, poisonPos))){
            food.position = food.GenerateRandomPos(snake.body);
        }
    }
    // Builds the stage by adding walls and poison fruit based on the current stage, and relocates the food
    void BuildStage(){
        if (stage < 2){
            walls.clear();
        }
        if (stage >= 2){
            walls.clear();
            int wallCount = 8 + (stage - 2) * 4;
            for (int i = 0; i < wallCount; i++){
                Vector2 pos = GenerateRandomPos(snake.body, true, true);
                walls.push_back(pos);
            }
        }

        if (stage >= 3){
            poisonActive = true;
            poisonPos = GenerateRandomPos(snake.body, true, false);
        }
        else{
            poisonActive = false;
        }

        RelocateFood();
    }
    // Checks if the snake head is colliding with the food, and if so, relocates the food, adds a segment to the snake, increases the score and stage progress, and plays the eat sound. Also checks if the stage goal has been reached and if so, advances to the next stage
    void CheckCollisionWithFood(){
        if (Vector2Equals(snake.body[0], food.position)){
            RelocateFood();
            snake.addSegment = true;
            score++;
            applesThisStage++;
            PlaySound(eatSound);
        }
        if(applesThisStage >= CurrentStageGoal()){
            stage++;
            applesThisStage = 0;
            moveSpeed = max(0.06, moveSpeed * 0.9);
            score = 0;
            BuildStage();
        }
    }
    // Checks if the snake head is colliding with the edges of the screen
    void CheckCollisionWithEdges(){
        if (snake.body[0].x == cellCount || snake.body[0].x == -1){
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1){
            GameOver();
        }
    }
    // Checks if the snake head is colliding with the walls or poison fruit
    void CheckCollisionWithWalls(){
        if (stage >= 2 && ElementInVector(snake.body[0], walls)){
            GameOver();
        }
        if (poisonActive && Vector2Equals(snake.body[0], poisonPos)){
            GameOver();
        }
    }
    // Resets the game to the starting conditions of stage 1
    void GameOver(){
        snake.Reset();
        stage = 1;
        applesThisStage = 0;
        moveSpeed = 0.2;
        walls.clear();
        poisonActive = false;
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }
    // Checks if the snake head is colliding with its own body, ignoring the head itself
    void CheckCollisionWithTail(){
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)){
            GameOver();
        }
    }
    // Draws the walls to the screen
    void DrawWalls(){
        for (unsigned int i = 0; i < walls.size(); i++){
            int drawX = offset + (int)(walls[i].x * cellSize);
            int drawY = offset + (int)(walls[i].y * cellSize);
            DrawRectangle(drawX, drawY, cellSize, cellSize, wallGreen);
        }
    }
    // Draws the poison fruit to the screen
    void DrawPoison(){
        if (!poisonActive) return;
        int drawX = offset + (int)(poisonPos.x * cellSize);
        int drawY = offset + (int)(poisonPos.y * cellSize);
        DrawRectangle(drawX, drawY, cellSize, cellSize, poisonRed);
    }
};
// Main function, initializes the window and game, and contains the main game loop that checks for input, updates the game logic, and draws everything to the screen
int main(){
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();
    ScreenState screenState = ScreenState::Menu;
    int menuSelection = 0;
    bool botMode = false;
    float movementAccumulator = 0.0f;

    while (WindowShouldClose() == false){
        float frameTime = GetFrameTime();
        BeginDrawing();

        if (screenState == ScreenState::Menu){
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)){
                menuSelection = 1 - menuSelection;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)){
                botMode = (menuSelection == 1);
                screenState = ScreenState::Playing;
                allowMove = true;
                game.snake.Reset();
                game.stage = 1;
                game.applesThisStage = 0;
                game.moveSpeed = 0.2;
                game.walls.clear();
                game.poisonActive = false;
                game.food.position = game.food.GenerateRandomPos(game.snake.body);
                game.running = botMode;
                game.score = 0;
                movementAccumulator = 0.0f;
            }

            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
            int selectModeWidth = MeasureText("Select Mode", 30);
            DrawText("Select Mode", offset + cellSize * cellCount - selectModeWidth, offset - 40, 30, darkGreen);
            Color playColor = (menuSelection == 0) ? darkGreen : wallGreen;
            Color botColor = (menuSelection == 1) ? darkGreen : wallGreen;
            DrawText("Play", offset + 40, offset + 40, 32, playColor);
            DrawText("Watch Bot", offset + 40, offset + 90, 32, botColor);
            DrawText("Enter/Space to start", offset, offset + 150, 20, darkGreen);
        }
        else{
            if (IsKeyPressed(KEY_M)){
                screenState = ScreenState::Menu;
                allowMove = false;
                game.running = false;
                movementAccumulator = 0.0f;
            }

            movementAccumulator += frameTime;

            if (!botMode){
                if (IsKeyPressed(KEY_W) && game.snake.direction.y != 1 && allowMove){
                    game.snake.direction = {0, -1};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove){
                    game.snake.direction = {0, -1};
                    game.running = true;
                    allowMove = false;
                }
                if (IsKeyPressed(KEY_S) && game.snake.direction.y != -1 && allowMove){
                    game.snake.direction = {0, 1};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove){
                    game.snake.direction = {0, 1};
                    game.running = true;
                    allowMove = false;
                }
                if (IsKeyPressed(KEY_A) && game.snake.direction.x != 1 && allowMove){
                    game.snake.direction = {-1, 0};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove){
                    game.snake.direction = {-1, 0};
                    game.running = true;
                    allowMove = false;
                }
                if (IsKeyPressed(KEY_D) && game.snake.direction.x != -1 && allowMove){
                    game.snake.direction = {1, 0};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove){
                    game.snake.direction = {1, 0};
                    game.running = true;
                    allowMove = false;
                }
            }

            while (movementAccumulator >= game.moveSpeed){
                movementAccumulator -= (float)game.moveSpeed;
                if (botMode){
                    game.snake.direction = game.ChooseBotDirection();
                }
                game.Update();
                allowMove = true;
            }

            float interpolationAlpha = 0.0f;
            if (game.moveSpeed > 0.0){
                interpolationAlpha = movementAccumulator / (float)game.moveSpeed;
            }
            interpolationAlpha = Clamp(interpolationAlpha, 0.0f, 1.0f);

            // Drawing
            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
            DrawText(TextFormat("Stage %i", game.stage), offset + 450, 20, 40, darkGreen);
            DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
            game.Draw(interpolationAlpha);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}