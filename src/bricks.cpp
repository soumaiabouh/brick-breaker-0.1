#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define DEG2RAD 3.14159/180.0

// Global Vars
int WINDOW_WIDTH = 600;
int WINDOW_HEIGHT = 700;
std::string STUDENT_ID = "261053234"; //TODO: rename based on who's submitting it
int gameScore = 0;
int livesLeft = 3;

// Wall dimensions
const int WALL_THICKNESS = 20;  // Thickness of the side walls
const int TOP_WALL_HEIGHT = 20;  // Height of the top wall
const float WALL_COLOR[3] = { 0.75f, 0.75f, 0.75f };  // Color for all walls
const float WALL_HEIGHT = WINDOW_HEIGHT - 60.0;

const int TOP_WALL_BOUNDARY = 40 + TOP_WALL_HEIGHT; // 40 is the starting point (0, 40)
const int LEFT_WALL_BOUNDARY = WALL_THICKNESS;
const int RIGHT_WALL_BOUNDARY = WINDOW_WIDTH - WALL_THICKNESS;

// Brick dimensions and spacing
const int BRICK_ROWS = 6;
const int BRICK_COLS = 18;
const float BRICK_HEIGHT = 20.0f;
const float BRICK_SPACING = 2.0f;
float brickWidth;  // To be calculated dynamically


// Vectors to store brick properties
std::vector<float> brickPositionsX;
std::vector<float> brickPositionsY;
std::vector<bool> brickActive;

// Paddle dimensions and position
const int PADDLE_LENGTH = 64;
const int PADDLE_HEIGHT = 20;  
const float PADDLE_SPEED = 250.0f;  // Adjust this speed based on testing

int paddleLength = PADDLE_LENGTH;  // Total length of the paddle
int paddleX = (WINDOW_WIDTH - paddleLength) / 2;  // Starting x position
int paddleY = 650;  // Vertical position


// Colors for the paddle sections
const GLfloat PADDLE_LEFT_COLOR[3] = { 0.7f, 0.2f, 0.2f };  // Color 2Ch approx
const GLfloat PADDLE_MIDDLE_COLOR[3] = { 0.2f, 0.3f, 0.8f };  // Color 2Dh approx
const GLfloat PADDLE_RIGHT_COLOR[3] = { 0.8f, 0.7f, 0.2f };  // Color 2Eh approx

// Segment widths
int paddleMiddleWidth = 12;
int paddleLeftWidth = (paddleLength - paddleMiddleWidth) / 2;
int paddleRightWidth = paddleLeftWidth;

// Ball properties
const float DX = 3.5f;      // X component of the velocity of the ball for the rest of the game when certain conditions are met
const float DY = -3.5f;     // Y component of the velocity of the ball for the rest of the game when certain conditions are met
float ballRadius = 5.0f;   // Visible size
float ballX = WINDOW_WIDTH / 2;   // Start in the middle of the screen horizontally
float ballY = WINDOW_HEIGHT / 2;  // Start in the middle of the screen vertically
float ballDX = 0.0f;       // Initial horizontal velocity
float ballDY = DY;         // Initial vertical velocity

// Flags
bool lifeLost = false;
bool gamePaused = false;
bool gameOver = false;
bool restartGame = false;
// Key pressed flags
bool leftKeyPressed = false;      // tracks paddle movement to the left
bool rightKeyPressed = false;     // tracks paddle movement to the right
bool leftArrowPressed = false;
bool rightArrowPressed = false;

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
float laserX = 0.0f;
float laserY = 0.0f;
float laserDX = 0.0f;
float laserDY = 1.0f;

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
    livesLeft--;
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
    if (width < WINDOW_WIDTH || height < WINDOW_HEIGHT) {
        // Reset the window size to the minimum dimensions
        width = std::max(width, WINDOW_WIDTH);
        height = std::max(height, WINDOW_HEIGHT);

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
    std::string score_str = "SCORE: " + std::to_string(gameScore);
    std::string lives_str = "LIVES: " + std::to_string(livesLeft);

    const char* score_to_print = score_str.c_str();
    const char* lives_to_print = lives_str.c_str();
    const char* student_id = "261053234";

    // Dividing up the width into 3 subcells
    float third_width = WINDOW_WIDTH / 3.0f;

    // Calculate x positions to center text
    int score_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, score_to_print);
    int lives_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, lives_to_print);
    int student_id_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, student_id);

    // Centering within each subcell
    float score_x = (third_width - score_width) / 2.0f;
    float lives_x = third_width + (third_width - lives_width) / 2.0f;
    float student_id_x = 2 * third_width + (third_width - student_id_width) / 2.0f;

    // Calculate the position based on the width of the screen. Center them
    glColor3f(1.0f, 1.0f, 1.0f);
    renderBitmapString(score_x, 30.0f, GLUT_BITMAP_HELVETICA_18, score_to_print);
    renderBitmapString(lives_x, 30.0f, GLUT_BITMAP_HELVETICA_18, lives_to_print);
    renderBitmapString(student_id_x, 30.0f, GLUT_BITMAP_HELVETICA_18, student_id);
}

void printGameOverText() {
    gameOver = true; // Set game over flag
    const char* gameOverText = "Game Over!";
    float textWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, gameOverText);
    float textX = (WINDOW_WIDTH - textWidth) / 2; // Center the text horizontally
    float textY = WINDOW_HEIGHT / 2; // Position the text vertically in the middle
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the game over text
    renderBitmapString(textX, textY, GLUT_BITMAP_HELVETICA_18, gameOverText);
}

void printGameOverOptions() {
    const char* gameOverOptions = "Press [r] to restart or [q] to quit";
    float optionsWidth = calculateStringWidth(GLUT_BITMAP_9_BY_15, gameOverOptions);
    float optionsX = (WINDOW_WIDTH - optionsWidth) / 2;
    float optionsY = WINDOW_HEIGHT / 2 + 50; // Below the game over text
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderBitmapString(optionsX, optionsY, GLUT_BITMAP_9_BY_15, gameOverOptions);
}

void printPressKeyToContinue() {
    const char* continueMessage = "Press any key to continue";
    float messageWidth = calculateStringWidth(GLUT_BITMAP_9_BY_15, continueMessage);
    float messageX = (WINDOW_WIDTH - messageWidth) / 2;
    float messageY = WINDOW_HEIGHT / 2 + 50; // Below the score or any other central message
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderBitmapString(messageX, messageY, GLUT_BITMAP_9_BY_15, continueMessage);
}

void printWinMessage() {
    const char* winText = "You Won!";
    float textWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, winText);
    float textX = (WINDOW_WIDTH - textWidth) / 2;
    float textY = WINDOW_HEIGHT / 2 - 50; // Position it above the game over options
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for the win text
    renderBitmapString(textX, textY, GLUT_BITMAP_HELVETICA_18, winText);
}

void printPowerUpStatus() {
    if (powerUpActive) {
        std::string powerUpText = "Power-Up Active: " + std::to_string(powerUpDuration);
        float textWidth = calculateStringWidth(GLUT_BITMAP_9_BY_15, powerUpText.c_str());
        float textX = (WINDOW_WIDTH - textWidth) / 2;
        float textY = WINDOW_HEIGHT - 15; // Display at the bottom of the screen
        glColor3f(0.0f, 1.0f, 0.0f);
        renderBitmapString(textX, textY, GLUT_BITMAP_9_BY_15, powerUpText.c_str());
    }
}

// 2. STATIC ELEMENTS
// 2.1 WALLS
void drawWalls() {
    glColor3fv(WALL_COLOR);  // Set the color for walls
    drawRectangle(0.0f, 40.0f, WALL_THICKNESS, WALL_HEIGHT);            // left wall
    drawRectangle(WINDOW_WIDTH - WALL_THICKNESS, 40.0f, WINDOW_WIDTH, WALL_HEIGHT);   // right wall
    drawRectangle(0.0f, 40.0f, WINDOW_WIDTH, 40 + TOP_WALL_HEIGHT);            // top wall
}

// 2.2 BRICKS
void initBricks() {
    int horizontal_margin = 20;
    int vertical_margin = 20;

    float wallTop = 40.0f + TOP_WALL_HEIGHT; // lower part of the top wall (y2)
    float wallLeft = WALL_THICKNESS;
    float wallRight = WINDOW_WIDTH - WALL_THICKNESS;

    float availableWidth = wallRight - wallLeft - ((BRICK_COLS - 1) * BRICK_SPACING) - (2 * horizontal_margin); // leaving some space between the bricks and both walls

    brickWidth = availableWidth / BRICK_COLS;

    float startX = wallLeft + horizontal_margin;
    float startY = wallTop + vertical_margin; // Starting Y position for the bricks + added margin

    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            float x = startX + col * (brickWidth + BRICK_SPACING);
            float y = startY + row * (BRICK_HEIGHT + BRICK_SPACING);
            brickPositionsX.push_back(x);
            brickPositionsY.push_back(y);
            brickActive.push_back(true);
        }
    }
}

void drawBricks() {
    // Draw bricks
    glColor3f(1.0f, 0.3f, 0.3f); // Set brick color
    for (size_t i = 0; i < brickPositionsX.size(); ++i) {
        if (brickActive[i]) {
            drawRectangle(brickPositionsX[i], brickPositionsY[i],
                brickPositionsX[i] + brickWidth,
                brickPositionsY[i] + BRICK_HEIGHT);
        }
    }
}

bool allBricksDestroyed() {
    for (bool active : brickActive) {
        if (active) return false;
    }
    return true;
}

// 3. DYNAMIC ELEMENTS
// 3.0 Restart
void resetPaddlePowerUpVariables() {
    powerUpDuration = 0;
    doublePaddleLength = false;
    paddleLength = PADDLE_LENGTH;
}

void resetLaserPowerUpVariables() {
    laserActive = false;
}

void resetPowerUpVariables() {
    powerUpActive = false;
    resetPaddlePowerUpVariables();
    resetLaserPowerUpVariables();
    if (doublePaddleLength) {
        paddleX += (PADDLE_LENGTH / 2); // Adjust paddle position
    }
}


// 3.1 Paddle
void drawPaddle() {
    // If power-up in effect, double the length
    if (doublePaddleLength) {
        paddleLength = PADDLE_LENGTH * 2;
        paddleMiddleWidth = 24;
    }
    else {
        // Resetting
        paddleLength = PADDLE_LENGTH;
        paddleMiddleWidth = 12;
    }

    // Updating the sections
    paddleLeftWidth = (paddleLength - paddleMiddleWidth) / 2;
    paddleRightWidth = paddleLeftWidth;

    // Draw left section
    glColor3fv(PADDLE_LEFT_COLOR);
    drawRectangle(paddleX, paddleY, paddleX + paddleLeftWidth, paddleY + PADDLE_HEIGHT);

    // Draw middle section
    glColor3fv(PADDLE_MIDDLE_COLOR);
    drawRectangle(paddleX + paddleLeftWidth, paddleY, paddleX + paddleLeftWidth + paddleMiddleWidth, paddleY + PADDLE_HEIGHT);

    // Draw right section
    glColor3fv(PADDLE_RIGHT_COLOR);
    drawRectangle(paddleX + paddleLeftWidth + paddleMiddleWidth, paddleY, paddleX + paddleLeftWidth + paddleMiddleWidth + paddleRightWidth, paddleY + PADDLE_HEIGHT);
}

void updatePaddle(float deltaTime) {
    float moveAmount = PADDLE_SPEED * deltaTime;

    if (leftKeyPressed || leftArrowPressed) {
        paddleX -= moveAmount;
        if (paddleX < 0) paddleX = 0;
    }
    if (rightKeyPressed || rightArrowPressed) {
        paddleX += moveAmount;
        if (paddleX > WINDOW_WIDTH - paddleLength) paddleX = WINDOW_WIDTH - paddleLength;
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

    if (gamePaused) {
        gamePaused = false; // Unpause the game on any key press
        lifeLost = false; // Reset life lost flag
    }

    switch (key) {
    case '1':
        if (gameScore >= lastPowerUpScore + 50 && !powerUpActive) {
            powerUpActive = true;
            powerUpDuration = 500;  // Set for 500 iterations
            lastPowerUpScore = gameScore;  // Update the score at activation

            doublePaddleLength = true;     // Set the flag
            paddleX -= (PADDLE_LENGTH / 2); // Allows for gracefully expending 
        }
        break;
    
    case '2':
        if (gameScore >= lastPowerUpScore + 50 && !laserActive) {
            laserActive = true;
            lastPowerUpScore = gameScore;  // Update the score at activation
            laserX = paddleX + paddleLength / 2; // Middle of the paddle
            laserY = paddleY; // Starting at the paddle's vertical position
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
        rightArrowPressed = true;
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
        rightArrowPressed = false;
        break;
    }
}

void specialInput(int key, int x, int y) {
    if (gamePaused) {
        gamePaused = false; // Unpause the game on any key press
        lifeLost = false; // Reset life lost flag
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
    glVertex2f(ballX, ballY);  // Center of circle
    for (int angle = 0; angle <= 360; angle++) {
        float rad = angle * DEG2RAD; // Convert angle to radians
        glVertex2f(ballX + cos(rad) * ballRadius, ballY + sin(rad) * ballRadius);
    }
    glEnd();
}

int checkWallCollision(int x, int y) {
    // Check for wall collisions considering the radius of the ball
    if (((x - ballRadius <= LEFT_WALL_BOUNDARY && y - ballRadius > TOP_WALL_BOUNDARY) ||
        (x + ballRadius >= RIGHT_WALL_BOUNDARY && y - ballRadius > TOP_WALL_BOUNDARY))) {
        return 1; // Ball is next to the left or right wall but not at the corner
    }
    if (y - ballRadius <= TOP_WALL_BOUNDARY && x > LEFT_WALL_BOUNDARY && x < RIGHT_WALL_BOUNDARY) {
        return 2; // Ball is next to the top wall but not at the corners
    }
    if ((x - ballRadius <= LEFT_WALL_BOUNDARY || x + ballRadius >= RIGHT_WALL_BOUNDARY) &&
        y - ballRadius <= TOP_WALL_BOUNDARY) {
        return 3; // Ball is at a corner
    }
    return 0; // No collision
}

int checkPaddleCollision() {

    if (ballY + ballRadius >= paddleY && ballY + ballRadius <= paddleY + PADDLE_HEIGHT) {
        int leftSectionEnd = paddleX + paddleLeftWidth;
        int middleSectionEnd = leftSectionEnd + paddleMiddleWidth;

        if (ballX + ballRadius >= paddleX && ballX < leftSectionEnd) {
            return 1;  // Ball is above the left section
        }
        else if (ballX >= leftSectionEnd && ballX < middleSectionEnd) {
            return 2;  // Ball is above the middle section
        }
        else if (ballX >= middleSectionEnd && ballX - ballRadius < paddleX + paddleLength) {
            return 3;  // Ball is above the right section
        }
    }
    return 0;  // Ball is not above the paddle
}

int checkBrickCollision(float& ball_x, float& ball_y, float& ball_dx, float& ball_dy, bool multipleChecks = true) {
    int collisionType = 0;
    for (size_t i = 0; i < brickPositionsX.size(); ++i) {
        if (brickActive[i]) {
            float brickLeft = brickPositionsX[i];
            float brickRight = brickLeft + brickWidth;
            float brickTop = brickPositionsY[i];
            float brickBottom = brickTop + BRICK_HEIGHT;

            // Check collision with the ball
            if (ball_x + ballRadius > brickLeft && ball_x - ballRadius < brickRight &&
                ball_y + ballRadius > brickTop && ball_y - ballRadius < brickBottom) {
                // Determine points by row
                int row = i / BRICK_COLS;
                if (row < 2) gameScore += 5;       // Top two rows
                else if (row < 4) gameScore += 3;  // Middle two rows
                else gameScore += 10;               // Bottom two rows

                brickActive[i] = false;  // Remove the brick

                // Determine side of collision
                bool hitVertical = ((ball_x + ballRadius) > brickLeft && (ball_x - ballRadius) < brickRight);
                bool hitHorizontal = ((ball_y + ballRadius) > brickTop && (ball_y - ballRadius) < brickBottom);

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
    int collisionType = checkWallCollision(ballX, ballY);
    switch (collisionType) {
    case 1:
        ballDX = -ballDX; // Invert horizontal velocity
        break;
    case 2:
        ballDY = -ballDY; // Invert vertical velocity
        break;
    case 3:
        ballDX = -ballDX; // Invert both horizontal
        ballDY = -ballDY; // and vertical velocity
        break;
    }

    int brickCollision = checkBrickCollision(ballX, ballY, ballDX, ballDY);
    switch (brickCollision) {
    case 1:
        ballDX = -ballDX; // Invert horizontal velocity
        break;
    case 2:
        ballDY = -ballDY; // Invert vertical velocity
        break;
    case 3:
        ballDX = -ballDX; // Invert both horizontal
        ballDY = -ballDY; // and vertical velocity
        break;
    }

    // Check for collision with the paddle
    int paddleCollision = checkPaddleCollision();
    switch (paddleCollision) {
    case 1:  // Collision with left section
        ballDY = -fabs(DY);
        ballDX = -fabs(DX);
        break;
    case 2:  // Collision with middle section
        ballDX = 0.0f;
        ballDY = -fabs(DY);
        break;
    case 3:  // Collision with right section
        ballDY = -fabs(DY);
        ballDX = fabs(DX);
        break;
    default:
        // No collision, proceed as normal
        break;
    }


}

int resetAfterBallLoss() {
    if (ballY + ballRadius >= WINDOW_HEIGHT) {
        decreaseLives(); // Decrement the lives
        lifeLost = true;

        // Deactivate power-ups
        resetPowerUpVariables();
        
        if (livesLeft > 0) {
            // Reset the ball position
            ballX = WINDOW_WIDTH / 2;
            ballY = WINDOW_HEIGHT / 2;
            ballDY = -fabs(ballDY); // Reset the ball's vertical direction upward
            // Reset paddle position
            paddleX = (WINDOW_WIDTH - paddleLength) / 2;
        }
        else {
            // If no lives left, signal Game Over
            gameOver = true;
            return 0;
        }
    }
    return livesLeft;
}

void updateBall() {
    if (gameOver || gamePaused) {
        return;  // Skip updating the ball if the game is paused
    }
    // Handle collisions
    handleCollisions();

    if (allBricksDestroyed()) {
        gameOver = true;
        gamePaused = true;  // Stop the game
    }

    // Update ball position based on velocity
    ballX += ballDX;
    ballY += ballDY;

    lifeLost = false; // set to false

    // Check if the ball hits the bottom of the screen
    resetAfterBallLoss();

    // Check if a life was lost and handle pausing
    if (lifeLost) {
        gamePaused = true;  // Pause the game
    }

}

// 3.3 Laser
void drawLaser() {
    if (laserActive) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red color for the laser
        glBegin(GL_QUADS);
        glVertex2f(laserX - 1, laserY);
        glVertex2f(laserX + 1, laserY);
        glVertex2f(laserX + 1, laserY - 10);
        glVertex2f(laserX - 1, laserY - 10);
        glEnd();
    }
}

void updateLaser() {
    if (laserActive) {
        laserY -= 1; // Move the laser up
        // Check collision with the top wall
        if (laserY <= TOP_WALL_BOUNDARY) {
            powerUpActive = false;
            resetLaserPowerUpVariables(); // Deactivate the laser if it hits the top wall
        }

        // Check for brick collisions
        if (checkBrickCollision(laserX, laserY, laserDX, laserDY, false) > 0) {
            // Hit a brick, but we don't need the collision type
            // We stop the laser
            powerUpActive = false;
            resetLaserPowerUpVariables();
        }

    }
}

// 4.0 Separation of concerns: putting together the functions that perform similar actions
void handleLifeLost() {
    ballDX = 0.0f;
    printPressKeyToContinue();
}

void handleGameOver() {
    // Display the appropriate game over text
    if (allBricksDestroyed()) {
        printWinMessage();
    }
    else {
        printGameOverText();
    }
    printGameOverOptions();
}

void handleGameRestart() {
    gameOver = false; // Reset game over flag
    restartGame = false;
    gamePaused = false; // Ensure the game is not paused
    livesLeft = 3; // Reset lives
    gameScore = 0; // Reset score

    // Reset ball and paddle positions
    ballX = WINDOW_WIDTH / 2;
    ballY = WINDOW_HEIGHT / 2;
    ballDX = 0.0f;
    ballDY = -fabs(ballDY); // going upwards

    // Reset paddle position
    paddleX = (WINDOW_WIDTH - paddleLength) / 2;

    // Reset bricks
    initBricks(); // Reinitialize the bricks

    // Reset variables after power-up
    lastPowerUpScore = 0;
    resetPowerUpVariables();
}

void checkGameState() {
    if (restartGame) {
        handleGameRestart();
    }
    else if (gameOver) {
        handleGameOver();
    }
    else if (lifeLost) {
        handleLifeLost();
    }
}

void updateGameLogic() {
    updateBall();
    updateLaser();

    if (powerUpActive) {
        powerUpDuration--;
        if (powerUpDuration <= 0 || lifeLost) {
            resetPowerUpVariables();
        }
    }
    checkGameState();
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

    glutSwapBuffers(); // Swap the buffers to make it visible
}

// Initialize OpenGL Graphics
void initOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Clear the background to black

    // Set up an orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0);
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
    int posX = (screenWidth - WINDOW_WIDTH) / 2;
    int posY = (screenHeight - WINDOW_HEIGHT) / 2;
    glutInitWindowPosition(posX, posY);

    // 2. Setting the size
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
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
