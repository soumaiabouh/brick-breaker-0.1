#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define DEG2RAD 3.14159/180.0

// Global Vars
int WIDTH = 600;
int HEIGHT = 700;
std::string STUDENTID = "261053234"; //TODO: rename based on who's submitting it
int score = 0;
int lives = 3;

// Wall dimensions
const int WALL_THICKNESS = 20;  // Thickness of the side walls
const int TOP_WALL_HEIGHT = 20;  // Height of the top wall
const float WALL_COLOR[3] = { 0.75f, 0.75f, 0.75f };  // Color for all walls
const float WALL_HEIGHT = HEIGHT - 60.0;

const int TOP_WALL_BOUNDARY = 40 + TOP_WALL_HEIGHT; // 40 is the starting point (0, 40)
const int LEFT_WALL_BOUNDARY = WALL_THICKNESS;
const int RIGHT_WALL_BOUNDARY = WIDTH - WALL_THICKNESS;

// Brick dimensions and spacing
const int BRICK_ROWS = 6;
const int BRICK_COLS = 18;
float BRICK_WIDTH;  // To be calculated dynamically
const float BRICK_HEIGHT = 20.0f;
float BRICK_SPACING = 2.0f;

// Vectors to store brick properties
std::vector<float> brick_x_positions;
std::vector<float> brick_y_positions;
std::vector<bool> brick_active;

// Paddle dimensions and position
const int PADDLE_LENGTH = 64;
int paddle_length = PADDLE_LENGTH;  // Total length of the paddle
int paddle_x = (WIDTH - paddle_length) / 2;  // Starting x position
int paddle_y = 650;  // Vertical position
const int paddle_height = 20;  // Height of the paddle

// Colors for the paddle sections
const GLfloat leftColor[3] = { 0.7f, 0.2f, 0.2f };  // Color 2Ch approx
const GLfloat middleColor[3] = { 0.2f, 0.3f, 0.8f };  // Color 2Dh approx
const GLfloat rightColor[3] = { 0.8f, 0.7f, 0.2f };  // Color 2Eh approx

// Segment widths
int middleWidth = 12;
int leftWidth = (paddle_length - middleWidth) / 2;
int rightWidth = leftWidth;

// Ball properties
float ball_radius = 5.0f;   // Visible size
float ball_x = WIDTH / 2;   // Start in the middle of the screen horizontally
float ball_y = HEIGHT / 2;  // Start in the middle of the screen vertically
const float DX = 3.5f;      // X component of the velocity of the ball for the rest of the game when certain conditions are met
const float DY = -3.5f;     // Y component of the velocity of the ball for the rest of the game when certain conditions are met
float ball_dx = 0.0f;       // Initial horizontal velocity
float ball_dy = DY;         // Initial vertical velocity

// Flags
bool life_lost = false;
bool is_paused = false;
bool gameOver = false;
bool restartGame = false;
// Paddle flags
bool leftKeyPressed = false;      // tracks paddle movement to the left
bool rightKeyPressed = false;     // tracks paddle movement to the right
bool leftArrowPressed = false;
bool rightArrowPressed = false;
// Paddle speed
const float speed = 250.0f;  // Adjust this speed based on testing

// Global variables to track time
float lastFrameTime = 0.0;
float currentFrameTime = 0.0;

// Global variables to track power-ups
int lastPowerUpScore = 0;   // Score at last power-up activation
bool powerUpActive = false; // Is a power-up currently active?
int powerUpDuration = 0;    // Remaining duration of the power-up

// For the paddle power-up
bool doublePaddleLength = false;

// For the laser power-up
bool laserActive = false;
float laser_x = 0.0f;
float laser_y = 0.0f;
float laser_dx = 0.0f;
float laser_dy = 1.0f;

// 0. Helper functions
// Function to draw a wall given bottom-left and top-right coordinates
void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);  // Begin drawing a quad
    glVertex2f(x1, y1); // Bottom left
    glVertex2f(x2, y1); // Bottom right
    glVertex2f(x2, y2); // Top right
    glVertex2f(x1, y2); // Top left
    glEnd();            // End drawing the quad
}

void decreaseLives() {
    lives--;
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Function to calculate the width of a string with a given font
int calculateStringWidth(void* font, const char* string) {
    int width = 0;
    const char* c;
    for (c = string; *c != '\0'; c++) {
        width += glutBitmapWidth(font, *c);
    }
    return width;
}

// Function to enforce a minimum window size
void reshape(int width, int height) {
    // Check if the current size is below the minimum size
    if (width < WIDTH || height < HEIGHT) {
        // Reset the window size to the minimum dimensions
        width = std::max(width, WIDTH);
        height = std::max(height, HEIGHT);

        // Resize the window to the new dimensions
        glutReshapeWindow(width, height);
    }
    else {
        // Adjust the viewport
        glViewport(0, 0, width, height);
    }
}


// 1. TEXT
// Function to render text using GLUT's bitmap fonts
void printText() {
    std::string score_str = "SCORE: " + std::to_string(score);
    std::string lives_str = "LIVES: " + std::to_string(lives);

    const char* score_to_print = score_str.c_str();
    const char* lives_to_print = lives_str.c_str();
    const char* studentID = "261053234";

    // Dividing up the width into 3 subcells
    float thirdWidth = WIDTH / 3.0f;

    // Calculate x positions to center text
    int scoreWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, score_to_print);
    int livesWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, lives_to_print);
    int studentIDWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, studentID);

    // Centering within each subcell
    float scoreX = (thirdWidth - scoreWidth) / 2.0f;
    float livesX = thirdWidth + (thirdWidth - livesWidth) / 2.0f;
    float studentIDX = 2 * thirdWidth + (thirdWidth - studentIDWidth) / 2.0f;

    // Calculate the position based on the width of the screen. Center them
    glColor3f(1.0f, 1.0f, 1.0f);
    renderBitmapString(scoreX, 30.0f, GLUT_BITMAP_HELVETICA_18, score_to_print);
    renderBitmapString(livesX, 30.0f, GLUT_BITMAP_HELVETICA_18, lives_to_print);
    renderBitmapString(studentIDX, 30.0f, GLUT_BITMAP_HELVETICA_18, studentID);
}

void printGameOverText() {
    gameOver = true; // Set game over flag
    const char* gameOverText = "Game Over!";
    float textWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, gameOverText);
    float textX = (WIDTH - textWidth) / 2; // Center the text horizontally
    float textY = HEIGHT / 2; // Position the text vertically in the middle
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the game over text
    renderBitmapString(textX, textY, GLUT_BITMAP_HELVETICA_18, gameOverText);
}

void printGameOverOptions() {
    const char* gameOverOptions = "Press [r] to restart or [q] to quit";
    float optionsWidth = calculateStringWidth(GLUT_BITMAP_9_BY_15, gameOverOptions);
    float optionsX = (WIDTH - optionsWidth) / 2;
    float optionsY = HEIGHT / 2 + 50; // Below the game over text
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderBitmapString(optionsX, optionsY, GLUT_BITMAP_9_BY_15, gameOverOptions);
}

void printPressKeyToContinue() {
    const char* continueMessage = "Press any key to continue";
    float messageWidth = calculateStringWidth(GLUT_BITMAP_9_BY_15, continueMessage);
    float messageX = (WIDTH - messageWidth) / 2;
    float messageY = HEIGHT / 2 + 50; // Below the score or any other central message
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderBitmapString(messageX, messageY, GLUT_BITMAP_9_BY_15, continueMessage);
}

void printWinMessage() {
    const char* winText = "You Won!";
    float textWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, winText);
    float textX = (WIDTH - textWidth) / 2;
    float textY = HEIGHT / 2 - 50; // Position it above the game over options
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for the win text
    renderBitmapString(textX, textY, GLUT_BITMAP_HELVETICA_18, winText);
}

void printPowerUpStatus() {
    if (powerUpActive) {
        std::string powerUpText = "Power-Up Active: " + std::to_string(powerUpDuration);
        float textWidth = calculateStringWidth(GLUT_BITMAP_9_BY_15, powerUpText.c_str());
        float textX = (WIDTH - textWidth) / 2;
        float textY = HEIGHT - 15; // Display at the bottom of the screen
        glColor3f(0.0f, 1.0f, 0.0f);
        renderBitmapString(textX, textY, GLUT_BITMAP_9_BY_15, powerUpText.c_str());
    }
}

// 2. STATIC ELEMENTS
// 2.1 WALLS
void drawWalls() {
    glColor3fv(WALL_COLOR);  // Set the color for walls
    drawRectangle(0.0f, 40.0f, WALL_THICKNESS, WALL_HEIGHT);            // left wall
    drawRectangle(WIDTH - WALL_THICKNESS, 40.0f, WIDTH, WALL_HEIGHT);   // right wall
    drawRectangle(0.0f, 40.0f, WIDTH, 40 + TOP_WALL_HEIGHT);            // top wall
}

// 2.2 BRICKS
void initBricks() {
    int horizontal_margin = 20;
    int vertical_margin = 20;

    float wallTop = 40.0f + TOP_WALL_HEIGHT; // lower part of the top wall (y2)
    float wallLeft = WALL_THICKNESS;
    float wallRight = WIDTH - WALL_THICKNESS;

    float availableWidth = wallRight - wallLeft - ((BRICK_COLS - 1) * BRICK_SPACING) - (2 * horizontal_margin); // leaving some space between the bricks and both walls

    BRICK_WIDTH = availableWidth / BRICK_COLS;

    float startX = wallLeft + horizontal_margin;
    float startY = wallTop + vertical_margin; // Starting Y position for the bricks + added margin

    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            float x = startX + col * (BRICK_WIDTH + BRICK_SPACING);
            float y = startY + row * (BRICK_HEIGHT + BRICK_SPACING);
            brick_x_positions.push_back(x);
            brick_y_positions.push_back(y);
            brick_active.push_back(true);
        }
    }
}

void drawBricks() {
    // Draw bricks
    glColor3f(1.0f, 0.3f, 0.3f); // Set brick color
    for (size_t i = 0; i < brick_x_positions.size(); ++i) {
        if (brick_active[i]) {
            drawRectangle(brick_x_positions[i], brick_y_positions[i],
                brick_x_positions[i] + BRICK_WIDTH,
                brick_y_positions[i] + BRICK_HEIGHT);
        }
    }
}

bool allBricksDestroyed() {
    for (bool active : brick_active) {
        if (active) return false;
    }
    return true;
}

// 3. DYNAMIC ELEMENTS
// 3.0 Restart
void resetPaddlePowerUpVariables() {
    powerUpDuration = 0;
    doublePaddleLength = false;
    paddle_length = PADDLE_LENGTH;
}

void resetLaserPowerUpVariables() {
    laserActive = false;
}

void resetPowerUpVariables() {
    powerUpActive = false;
    resetPaddlePowerUpVariables();
    resetLaserPowerUpVariables();
    if (doublePaddleLength) {
        paddle_x += (PADDLE_LENGTH / 2); // Adjust paddle position
    }
}


// 3.1 Paddle
void drawPaddle() {
    // If power-up in effect, double the length
    if (doublePaddleLength) {
        paddle_length = PADDLE_LENGTH * 2;
        middleWidth = 24;
    }
    else {
        // Resetting
        paddle_length = PADDLE_LENGTH;
        middleWidth = 12;
    }

    // Updating the sections
    leftWidth = (paddle_length - middleWidth) / 2;
    rightWidth = leftWidth;

    // Draw left section
    glColor3fv(leftColor);
    drawRectangle(paddle_x, paddle_y, paddle_x + leftWidth, paddle_y + paddle_height);

    // Draw middle section
    glColor3fv(middleColor);
    drawRectangle(paddle_x + leftWidth, paddle_y, paddle_x + leftWidth + middleWidth, paddle_y + paddle_height);

    // Draw right section
    glColor3fv(rightColor);
    drawRectangle(paddle_x + leftWidth + middleWidth, paddle_y, paddle_x + leftWidth + middleWidth + rightWidth, paddle_y + paddle_height);
}

void updatePaddle(float deltaTime) {
    float moveAmount = speed * deltaTime;

    if (leftKeyPressed || leftArrowPressed) {
        paddle_x -= moveAmount;
        if (paddle_x < 0) paddle_x = 0;
    }
    if (rightKeyPressed || rightArrowPressed) {
        paddle_x += moveAmount;
        if (paddle_x > WIDTH - paddle_length) paddle_x = WIDTH - paddle_length;
    }
}

// Keyboard handling
void keyboardHandler(unsigned char key, int x, int y) {
    if (gameOver) {
        if (key == 'r' || key == 'R') {
            restartGame = true; // Reset the game state flag
        }
        else if (key == 'q' || key == 'Q') {
            std::cout << "Exiting game." << std::endl;
            exit(0); // Exit the program
        }
        return; // Skip other inputs if the game is over
    }

    if (is_paused) {
        is_paused = false; // Unpause the game on any key press
        life_lost = false; // Reset life lost flag
    }

    switch (key) {
    case '1':
        if (score >= lastPowerUpScore + 50 && !powerUpActive) {
            powerUpActive = true;
            powerUpDuration = 500;  // Set for 500 iterations
            lastPowerUpScore = score;  // Update the score at activation

            doublePaddleLength = true;     // Set the flag
            paddle_x -= (PADDLE_LENGTH / 2); // Allows for gracefully expending 
        }
        break;
    
    case '2':
        if (score >= lastPowerUpScore + 50 && !laserActive) {
            laserActive = true;
            lastPowerUpScore = score;  // Update the score at activation
            laser_x = paddle_x + paddle_length / 2; // Middle of the paddle
            laser_y = paddle_y; // Starting at the paddle's vertical position
        }
        break;

    }

    switch (key) {
    case 'a':
    case 'A':
        leftKeyPressed = true;
        break;
    case 'd':
    case 'D':
        rightKeyPressed = true;
        break;
    }
}

void keyboardUpHandler(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        leftKeyPressed = false;
        break;
    case 'd':
    case 'D':
        rightKeyPressed = false;
        break;
    }
}

void specialInput(int key, int x, int y) {
    if (is_paused) {
        is_paused = false; // Unpause the game on any key press
        life_lost = false; // Reset life lost flag
    }

    switch (key) {
    case GLUT_KEY_LEFT:
        leftArrowPressed = true;
        break;
    case GLUT_KEY_RIGHT:
        rightArrowPressed = true;
        break;
    }
}

void specialInputUp(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        leftArrowPressed = false;
        break;
    case GLUT_KEY_RIGHT:
        rightArrowPressed = false;
        break;
    }
}

void timer(int value) {
    currentFrameTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;

    updatePaddle(deltaTime);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // Set up the next call to timer after approx. 16 ms
}


//3.2 Ball
void drawBall() {
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the ball
    glBegin(GL_TRIANGLE_FAN);    // Begin drawing a circle
    glVertex2f(ball_x, ball_y);  // Center of circle
    for (int angle = 0; angle <= 360; angle++) {
        float rad = angle * DEG2RAD; // Convert angle to radians
        glVertex2f(ball_x + cos(rad) * ball_radius, ball_y + sin(rad) * ball_radius);
    }
    glEnd();
}

int checkWallCollision(int x, int y) {
    // Check for wall collisions considering the radius of the ball
    if (((x - ball_radius <= LEFT_WALL_BOUNDARY && y - ball_radius > TOP_WALL_BOUNDARY) ||
        (x + ball_radius >= RIGHT_WALL_BOUNDARY && y - ball_radius > TOP_WALL_BOUNDARY))) {
        return 1; // Ball is next to the left or right wall but not at the corner
    }
    if (y - ball_radius <= TOP_WALL_BOUNDARY && x > LEFT_WALL_BOUNDARY && x < RIGHT_WALL_BOUNDARY) {
        return 2; // Ball is next to the top wall but not at the corners
    }
    if ((x - ball_radius <= LEFT_WALL_BOUNDARY || x + ball_radius >= RIGHT_WALL_BOUNDARY) &&
        y - ball_radius <= TOP_WALL_BOUNDARY) {
        return 3; // Ball is at a corner
    }
    return 0; // No collision
}

int checkPaddleCollision() {

    if (ball_y + ball_radius >= paddle_y && ball_y + ball_radius <= paddle_y + paddle_height) {
        int leftSectionEnd = paddle_x + leftWidth;
        int middleSectionEnd = leftSectionEnd + middleWidth;

        if (ball_x + ball_radius >= paddle_x && ball_x < leftSectionEnd) {
            return 1;  // Ball is above the left section
        }
        else if (ball_x >= leftSectionEnd && ball_x < middleSectionEnd) {
            return 2;  // Ball is above the middle section
        }
        else if (ball_x >= middleSectionEnd && ball_x - ball_radius < paddle_x + paddle_length) {
            return 3;  // Ball is above the right section
        }
    }
    return 0;  // Ball is not above the paddle
}

int checkBrickCollision(float& ball_x, float& ball_y, float& ball_dx, float& ball_dy, bool multipleChecks = true) {
    int collisionType = 0;
    for (size_t i = 0; i < brick_x_positions.size(); ++i) {
        if (brick_active[i]) {
            float brickLeft = brick_x_positions[i];
            float brickRight = brickLeft + BRICK_WIDTH;
            float brickTop = brick_y_positions[i];
            float brickBottom = brickTop + BRICK_HEIGHT;

            // Check collision with the ball
            if (ball_x + ball_radius > brickLeft && ball_x - ball_radius < brickRight &&
                ball_y + ball_radius > brickTop && ball_y - ball_radius < brickBottom) {
                // Determine points by row
                int row = i / BRICK_COLS;
                if (row < 2) score += 5;       // Top two rows
                else if (row < 4) score += 3;  // Middle two rows
                else score += 10;               // Bottom two rows

                brick_active[i] = false;  // Remove the brick

                // Determine side of collision
                bool hitVertical = ((ball_x + ball_radius) > brickLeft && (ball_x - ball_radius) < brickRight);
                bool hitHorizontal = ((ball_y + ball_radius) > brickTop && (ball_y - ball_radius) < brickBottom);

                if (hitVertical && !hitHorizontal) {
                    collisionType = std::max(collisionType, 1); // Side
                }
                else if (!hitVertical && hitHorizontal) {
                    collisionType = std::max(collisionType, 2); // Top/Bottom
                }
                else if (hitVertical && hitHorizontal) {
                    collisionType = std::max(collisionType, 3); // Corner
                }

                // If multiple checks is enabled, we continue checking in case we hit multiple bricks
                if (!multipleChecks) {
                    // Otherwise, we stop after hitting a single brick
                    break;
                }
            }
        }
    }
    return collisionType;
}

void handleCollisions() {
    int collisionType = checkWallCollision(ball_x, ball_y);
    switch (collisionType) {
    case 1:
        ball_dx = -ball_dx; // Invert horizontal velocity
        break;
    case 2:
        ball_dy = -ball_dy; // Invert vertical velocity
        break;
    case 3:
        ball_dx = -ball_dx; // Invert both horizontal
        ball_dy = -ball_dy; // and vertical velocity
        break;
    }

    int brickCollision = checkBrickCollision(ball_x, ball_y, ball_dx, ball_dy);
    switch (brickCollision) {
    case 1:
        ball_dx = -ball_dx; // Invert horizontal velocity
        break;
    case 2:
        ball_dy = -ball_dy; // Invert vertical velocity
        break;
    case 3:
        ball_dx = -ball_dx; // Invert both horizontal
        ball_dy = -ball_dy; // and vertical velocity
        break;
    }

    // Check for collision with the paddle
    int paddleCollision = checkPaddleCollision();
    switch (paddleCollision) {
    case 1:  // Collision with left section
        ball_dy = -fabs(DY);
        ball_dx = -fabs(DX);
        break;
    case 2:  // Collision with middle section
        ball_dx = 0.0f;
        ball_dy = -fabs(DY);
        break;
    case 3:  // Collision with right section
        ball_dy = -fabs(DY);
        ball_dx = fabs(DX);
        break;
    default:
        // No collision, proceed as normal
        break;
    }


}

int resetAfterBallLoss() {
    if (ball_y + ball_radius >= HEIGHT) {
        decreaseLives(); // Decrement the lives
        life_lost = true;

        // Deactivate power-ups
        resetPowerUpVariables();
        
        if (lives > 0) {
            // Reset the ball position
            ball_x = WIDTH / 2;
            ball_y = HEIGHT / 2;
            ball_dy = -fabs(ball_dy); // Reset the ball's vertical direction upward
            // Reset paddle position
            paddle_x = (WIDTH - paddle_length) / 2;
        }
        else {
            // If no lives left, signal Game Over
            gameOver = true;
            return 0;
        }
    }
    return lives;
}

void updateBall() {
    if (gameOver || is_paused) {
        return;  // Skip updating the ball if the game is paused
    }
    // Handle collisions
    handleCollisions();

    if (allBricksDestroyed()) {
        gameOver = true;
        is_paused = true;  // Stop the game
    }

    // Update ball position based on velocity
    ball_x += ball_dx;
    ball_y += ball_dy;

    life_lost = false; // set to false

    // Check if the ball hits the bottom of the screen
    resetAfterBallLoss();

    // Check if a life was lost and handle pausing
    if (life_lost) {
        is_paused = true;  // Pause the game
    }

}

// 3.3 Laser
void drawLaser() {
    if (laserActive) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red color for the laser
        glBegin(GL_QUADS);
        glVertex2f(laser_x - 1, laser_y);
        glVertex2f(laser_x + 1, laser_y);
        glVertex2f(laser_x + 1, laser_y - 10);
        glVertex2f(laser_x - 1, laser_y - 10);
        glEnd();
    }
}

void updateLaser() {
    if (laserActive) {
        laser_y -= 1; // Move the laser up
        // Check collision with the top wall
        if (laser_y <= TOP_WALL_BOUNDARY) {
            powerUpActive = false;
            resetLaserPowerUpVariables(); // Deactivate the laser if it hits the top wall
        }

        // Check for brick collisions
        if (checkBrickCollision(laser_x, laser_y, laser_dx, laser_dy, false) > 0) {
            // Hit a brick, but we don't need the collision type
            // We stop the laser
            powerUpActive = false;
            resetLaserPowerUpVariables();
        }

    }
}

// 4.0 Separation of concerns: putting together the functions that perform similar actions
void updateGameLogic() {
    updateBall();
    updateLaser();

    if (powerUpActive) {
        powerUpDuration--;
        if (powerUpDuration <= 0 || life_lost) {
            resetPowerUpVariables();
        }
    }
}

void handleLifeLost() {
    if (life_lost && !gameOver && !restartGame) {
        ball_dx = 0.0f;
        printPressKeyToContinue(); // Function to print press any key to continue
    }
}

void handleGameOver() {
    // Check if the game is over and display the game over text
    if (gameOver) {
        if (allBricksDestroyed()) {
            printWinMessage();
        }
        else {
            printGameOverText();
        }
        printGameOverOptions();
    } 
}

void handleGameRestart() {
    if (restartGame) {
        gameOver = false; // Reset game over flag
        restartGame = false;
        is_paused = false; // Ensure the game is not paused
        lives = 3; // Reset lives
        score = 0; // Reset score

        // Reset ball and paddle positions
        ball_x = WIDTH / 2;
        ball_y = HEIGHT / 2;
        ball_dx = 0.0f;
        ball_dy = -fabs(ball_dy); // going upwards

        // Reset paddle position
        paddle_x = (WIDTH - paddle_length) / 2;

        // Reset bricks
        initBricks(); // Reinitialize the bricks

        // Reset variables after power-up
        lastPowerUpScore = 0;
        resetPowerUpVariables();
    }
}

// GLUT display callback function
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

    updateGameLogic();

    // Static elements
    drawWalls();
    drawBricks();

    // Dynamic elements
    drawPaddle();
    drawBall();
    drawLaser();

    // Text display
    printText();
    printPowerUpStatus();

    handleLifeLost();
    handleGameRestart();
    handleGameOver();

    glutSwapBuffers(); // Swap the buffers to make it visible
}

// Initialize OpenGL Graphics
void initOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Clear the background to black

    // Set up an orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, HEIGHT, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);

    // 1. Setting the initial window position
    // Get the screen size
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

    // Calculate the window's initial position to center it
    int posX = (screenWidth - WIDTH) / 2;
    int posY = (screenHeight - HEIGHT) / 2;
    glutInitWindowPosition(posX, posY);

    // 2. Setting the size
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Brick Breaker – 260979679 & 261053234");

    // 3. Initialization
    initOpenGL();  // Initialize OpenGL settings
    initBricks();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardHandler);  // Register ASCII key handler
    glutKeyboardUpFunc(keyboardUpHandler);
    glutSpecialFunc(specialInput);  // Register special key handler (arrow keys)
    glutSpecialUpFunc(specialInputUp);

    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
