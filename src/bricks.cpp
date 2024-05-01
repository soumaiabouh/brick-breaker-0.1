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
const float WALL_COLOR[3] = { 0.5f, 0.5f, 0.5f };  // Color for all walls
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
const GLfloat PADDLE_LEFT_COLOR[3] = { 1.0f, 0.0f, 0.4f };  // Magenta
const GLfloat PADDLE_MIDDLE_COLOR[3] = { 0.0f, 0.0f, 128.0f / 255.0f }; // Navy
const GLfloat PADDLE_RIGHT_COLOR[3] = { 0.0f, 123.0f / 255.0f, 167.0f / 255.0f };;  // Cerulean

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

// Global variable to hold the shader program ID
GLuint shaderProgram;  

GLuint VAO = 0, VBO = 0;
// Shader sources
const char* vertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform float windowWidth;
        uniform float windowHeight;

        void main() {
            // Transform from pixel coordinates to normalized device coordinates
            float x = (aPos.x / windowWidth) * 2.0 - 1.0;
            float y = (aPos.y / windowHeight) * 2.0 - 1.0;
            gl_Position = vec4(x, -y, 0.0, 1.0); // Y is inverted as y increases downwards in pixel coords
        }
    )glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 uColor; // Base color
    uniform bool useGradient; // Control whether to use gradient coloring
    uniform float xPos; // Position-based gradient effect
    uniform float screenWidth; // Screen width for scaling gradient

    void main() {
        if (useGradient) {
            float hueX = xPos / screenWidth;
            vec3 gradientColor = vec3(0.1 + 0.95 * hueX, 0.7, 0.7); // Adjust gradient color
            FragColor = vec4(uColor * gradientColor, 1.0); // Apply gradient effect
        } else {
            FragColor = vec4(uColor, 1.0); // Use plain color
        }
    }
    )glsl";

// 0. Helper functions
// Function to draw a wall given bottom-left and top-right coordinates
void drawRectangle(float x1, float y1, float x2, float y2, const GLfloat* color, bool applyGradient) {
    float vertices[] = {
        x1, y1,
        x2, y1,
        x2, y2,
        x1, y2
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);  // Load vertex data

    glUseProgram(shaderProgram);
    glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, color); // Set color
    glUniform1f(glGetUniformLocation(shaderProgram, "windowWidth"), static_cast<float>(WINDOW_WIDTH));
    glUniform1f(glGetUniformLocation(shaderProgram, "windowHeight"), static_cast<float>(WINDOW_HEIGHT));
    glUniform1f(glGetUniformLocation(shaderProgram, "xPos"), (x1 + x2) / 2); // Middle of the rectangle for gradient effect
    glUniform1f(glGetUniformLocation(shaderProgram, "screenWidth"), static_cast<float>(WINDOW_WIDTH));
    glUniform1i(glGetUniformLocation(shaderProgram, "useGradient"), applyGradient ? 1 : 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Draw the rectangle

    glBindVertexArray(0);
    glUseProgram(0);
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

// Function to compile shaders
GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

// Function to create shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
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
    const char* game_over_text = "Game Over!";
    float text_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, game_over_text);
    float text_x = (WINDOW_WIDTH - text_width) / 2; // Center the text horizontally
    float text_y = WINDOW_HEIGHT / 2; // Position the text vertically in the middle
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for the game over text
    renderBitmapString(text_x, text_y, GLUT_BITMAP_HELVETICA_18, game_over_text);
}

void printGameOverOptions() {
    const char* game_over_options = "Press [r] to restart or [q] to quit";
    float options_width = calculateStringWidth(GLUT_BITMAP_9_BY_15, game_over_options);
    float options_x = (WINDOW_WIDTH - options_width) / 2;
    float options_y = WINDOW_HEIGHT / 2 + 50; // Below the game over text
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderBitmapString(options_x, options_y, GLUT_BITMAP_9_BY_15, game_over_options);
}

void printPressKeyToContinue() {
    const char* continue_message = "Press any key to continue";
    float message_width = calculateStringWidth(GLUT_BITMAP_9_BY_15, continue_message);
    float message_x = (WINDOW_WIDTH - message_width) / 2;
    float message_y = WINDOW_HEIGHT / 2 + 50; // Below the score or any other central message
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderBitmapString(message_x, message_y, GLUT_BITMAP_9_BY_15, continue_message);
}

void printWinMessage() {
    const char* win_text = "You Won!";
    float text_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, win_text);
    float text_x = (WINDOW_WIDTH - text_width) / 2;
    float text_y = WINDOW_HEIGHT / 2 - 50; // Position it above the game over options
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for the win text
    renderBitmapString(text_x, text_y, GLUT_BITMAP_HELVETICA_18, win_text);
}

void printPowerUpStatus() {
    if (powerUpActive) {
        std::string power_up_text = "Power-Up Active: " + std::to_string(powerUpDuration);
        float text_width = calculateStringWidth(GLUT_BITMAP_9_BY_15, power_up_text.c_str());
        float text_x = (WINDOW_WIDTH - text_width) / 2;
        float text_y = WINDOW_HEIGHT - 15; // Display at the bottom of the screen
        glColor3f(0.0f, 1.0f, 0.0f);
        renderBitmapString(text_x, text_y, GLUT_BITMAP_9_BY_15, power_up_text.c_str());
    }
}

// 2. STATIC ELEMENTS
// 2.1 WALLS
void drawWalls() {
    // Draw the left wall
    drawRectangle(0.0f, 40.0f, WALL_THICKNESS, WALL_HEIGHT, WALL_COLOR, false);

    // Draw the right wall
    drawRectangle(WINDOW_WIDTH - WALL_THICKNESS, 40.0f, WINDOW_WIDTH, WALL_HEIGHT, WALL_COLOR, false);

    // Draw the top wall
    drawRectangle(0.0f, 40.0f, WINDOW_WIDTH, 40 + TOP_WALL_HEIGHT, WALL_COLOR, false);
}

void drawRectangleContour(float x1, float y1, float x2, float y2, const GLfloat* color, float thickness) {
    // Generate vertices for the contour
    std::vector<float> contourVertices = {
        // Top horizontal line
        x1 - thickness, y1 - thickness,
        x2 + thickness, y1 - thickness,
        // Right vertical line
        x2 + thickness, y1 - thickness,
        x2 + thickness, y2 + thickness,
        // Bottom horizontal line
        x1 - thickness, y2 + thickness,
        x2 + thickness, y2 + thickness,
        // Left vertical line
        x1 - thickness, y1 - thickness,
        x1 - thickness, y2 + thickness
    };

    // Generate VAO and VBO for contour
    GLuint contourVAO, contourVBO;
    glGenVertexArrays(1, &contourVAO);
    glGenBuffers(1, &contourVBO);

    // Bind contour VAO and VBO
    glBindVertexArray(contourVAO);
    glBindBuffer(GL_ARRAY_BUFFER, contourVBO);

    // Load contour vertex data into VBO
    glBufferData(GL_ARRAY_BUFFER, contourVertices.size() * sizeof(float), contourVertices.data(), GL_STATIC_DRAW);

    // Specify contour vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set contour color uniform
    glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, color);

    // Draw the contour
    glDrawArrays(GL_LINES, 0, contourVertices.size() / 2);

    // Cleanup contour VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete VAO and VBO
    glDeleteVertexArrays(1, &contourVAO);
    glDeleteBuffers(1, &contourVBO);
}

void drawWallContours() {
    // Black color for the contour
    GLfloat contourColor[] = { 0.0f, 0.0f, 0.0f };
    // Contour thickness
    const float contourThickness = 0.1f; // Adjust this value to change the thickness

    // Draw contours for the left wall
    drawRectangleContour(0.0f, 60.0f, WALL_THICKNESS, WALL_HEIGHT, contourColor, contourThickness);

    // Draw contours for the right wall
    drawRectangleContour(WINDOW_WIDTH - WALL_THICKNESS, 60.0f, WINDOW_WIDTH, WALL_HEIGHT, contourColor, contourThickness);

    // Draw contours for the top wall
    drawRectangleContour(0.0f, 40.0f, WINDOW_WIDTH, 40 + TOP_WALL_HEIGHT, contourColor, contourThickness);
}

// 2.2 BRICKS
void initBricks() {
    int horizontal_margin = 20;
    int vertical_margin = 20;

    float wall_top = 40.0f + TOP_WALL_HEIGHT; // lower part of the top wall (y2)
    float wall_left = WALL_THICKNESS;
    float wall_right = WINDOW_WIDTH - WALL_THICKNESS;

    float available_width = wall_right - wall_left - ((BRICK_COLS - 1) * BRICK_SPACING) - (2 * horizontal_margin); // leaving some space between the bricks and both walls

    brickWidth = available_width / BRICK_COLS;

    float start_x = wall_left + horizontal_margin;
    float start_y = wall_top + vertical_margin; // Starting Y position for the bricks + added margin

    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            float x = start_x + col * (brickWidth + BRICK_SPACING);
            float y = start_y + row * (BRICK_HEIGHT + BRICK_SPACING);
            brickPositionsX.push_back(x);
            brickPositionsY.push_back(y);
            brickActive.push_back(true);
        }
    }
}

void drawBricks() {
    const GLfloat BRICK_COLOR[3] = { 1.0f, 1.0f, 1.0f }; // Red color for the bricks
    for (size_t i = 0; i < brickPositionsX.size(); ++i) {
        if (brickActive[i]) {
            drawRectangle(
                brickPositionsX[i],
                brickPositionsY[i],
                brickPositionsX[i] + brickWidth,
                brickPositionsY[i] + BRICK_HEIGHT,
                BRICK_COLOR,
                1  // Enable gradient effect for bricks
            );
        }
    }
}

void drawBrickContours() {
    // Black color for the contour
    GLfloat contourColor[] = { 0.0f, 0.0f, 0.0f };
    // Contour thickness
    const float contourThickness = 1.0f; // Adjust this value to change the thickness

    // Loop over each active brick
    for (size_t i = 0; i < brickPositionsX.size(); ++i) {
        if (brickActive[i]) {
            // Calculate brick position and size
            float x1 = brickPositionsX[i];
            float y1 = brickPositionsY[i];
            float x2 = x1 + brickWidth;
            float y2 = y1 + BRICK_HEIGHT;

            // Generate vertices for the contour
            std::vector<float> contourVertices = {
                // Top horizontal line
                x1 - contourThickness, y1 - contourThickness,
                x2 + contourThickness, y1 - contourThickness,
                // Right vertical line
                x2 + contourThickness, y1 - contourThickness,
                x2 + contourThickness, y2 + contourThickness,
                // Bottom horizontal line
                x1 - contourThickness, y2 + contourThickness,
                x2 + contourThickness, y2 + contourThickness,
                // Left vertical line
                x1 - contourThickness, y1 - contourThickness,
                x1 - contourThickness, y2 + contourThickness
            };

            // Generate VAO and VBO for contour
            GLuint contourVAO, contourVBO;
            glGenVertexArrays(1, &contourVAO);
            glGenBuffers(1, &contourVBO);

            // Bind contour VAO and VBO
            glBindVertexArray(contourVAO);
            glBindBuffer(GL_ARRAY_BUFFER, contourVBO);

            // Load contour vertex data into VBO
            glBufferData(GL_ARRAY_BUFFER, contourVertices.size() * sizeof(float), contourVertices.data(), GL_STATIC_DRAW);

            // Specify contour vertex attributes
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Use shader program
            glUseProgram(shaderProgram);

            // Set contour color uniform
            glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, contourColor);

            // Draw the contour
            glDrawArrays(GL_LINES, 0, contourVertices.size() / 2);

            // Cleanup contour VAO and VBO
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            // Delete VAO and VBO
            glDeleteVertexArrays(1, &contourVAO);
            glDeleteBuffers(1, &contourVBO);
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
    drawRectangle(
        paddleX,
        paddleY,
        paddleX + paddleLeftWidth,
        paddleY + PADDLE_HEIGHT,
        PADDLE_LEFT_COLOR,
        false  // No gradient for the paddle
    );

    // Draw middle section
    drawRectangle(
        paddleX + paddleLeftWidth,
        paddleY,
        paddleX + paddleLeftWidth + paddleMiddleWidth,
        paddleY + PADDLE_HEIGHT,
        PADDLE_MIDDLE_COLOR,
        false  // No gradient for the paddle
    );

    // Draw right section
    drawRectangle(
        paddleX + paddleLeftWidth + paddleMiddleWidth,
        paddleY,
        paddleX + paddleLeftWidth + paddleMiddleWidth + paddleRightWidth,
        paddleY + PADDLE_HEIGHT,
        PADDLE_RIGHT_COLOR,
        false  // No gradient for the paddle
    );
}

void updatePaddle(float deltaTime) {
    float move_amount = PADDLE_SPEED * deltaTime;

    if (leftKeyPressed || leftArrowPressed) {
        paddleX -= move_amount;
        if (paddleX < 0) paddleX = 0;
    }
    if (rightKeyPressed || rightArrowPressed) {
        paddleX += move_amount;
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
    float delta_time = currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;

    updatePaddle(delta_time);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // Set up the next call to timer after approx. 16 ms
}


//3.2 Ball
void drawBall() {
    // White color for the ball
    GLfloat ballColor[] = { 1.0f, 1.0f, 1.0f };
    // Black color for the contour
    GLfloat contourColor[] = { 0.0f, 0.0f, 0.0f };
    // Contour size:
    const float contourThickness = 3.0f; // Adjust this value to change the thickness

    // Generate vertices for the inner circle (ball)
    std::vector<float> ballVertices;
    for (int angle = 0; angle <= 360; angle++) {
        float rad = angle * DEG2RAD; // Convert angle to radians
        ballVertices.push_back(ballX + cos(rad) * ballRadius);
        ballVertices.push_back(ballY + sin(rad) * ballRadius);
    }

    // Generate VAO and VBO for ball
    GLuint ballVAO, ballVBO;
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);

    // Bind ball VAO and VBO
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);

    // Load ball vertex data into VBO
    glBufferData(GL_ARRAY_BUFFER, ballVertices.size() * sizeof(float), ballVertices.data(), GL_STATIC_DRAW);

    // Specify ball vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set ball color uniform
    glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, ballColor);

    // Draw the ball
    glDrawArrays(GL_TRIANGLE_FAN, 0, ballVertices.size() / 2);

    // Cleanup ball VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Generate vertices for the outer circles (contour)
    for (float thickness = 1.0f; thickness <= contourThickness; thickness++) {
        std::vector<float> contourVertices;
        float contourRadius = ballRadius + thickness;
        for (int angle = 0; angle <= 360; angle++) {
            float rad = angle * DEG2RAD; // Convert angle to radians
            contourVertices.push_back(ballX + cos(rad) * contourRadius);
            contourVertices.push_back(ballY + sin(rad) * contourRadius);
        }

        // Generate VAO and VBO for contour
        GLuint contourVAO, contourVBO;
        glGenVertexArrays(1, &contourVAO);
        glGenBuffers(1, &contourVBO);

        // Bind contour VAO and VBO
        glBindVertexArray(contourVAO);
        glBindBuffer(GL_ARRAY_BUFFER, contourVBO);

        // Load contour vertex data into VBO
        glBufferData(GL_ARRAY_BUFFER, contourVertices.size() * sizeof(float), contourVertices.data(), GL_STATIC_DRAW);

        // Specify contour vertex attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Set contour color uniform
        glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, contourColor);

        // Draw the contour
        glDrawArrays(GL_LINE_LOOP, 0, contourVertices.size() / 2);

        // Cleanup contour VAO and VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Delete VAO and VBO
        glDeleteVertexArrays(1, &contourVAO);
        glDeleteBuffers(1, &contourVBO);
    }
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
        int left_section_end = paddleX + paddleLeftWidth;
        int middle_section_end = left_section_end + paddleMiddleWidth;

        if (ballX + ballRadius >= paddleX && ballX < left_section_end) {
            return 1;  // Ball is above the left section
        }
        else if (ballX >= left_section_end && ballX < middle_section_end) {
            return 2;  // Ball is above the middle section
        }
        else if (ballX >= middle_section_end && ballX - ballRadius < paddleX + paddleLength) {
            return 3;  // Ball is above the right section
        }
    }
    return 0;  // Ball is not above the paddle
}

int checkBrickCollision(float& ball_x, float& ball_y, float& ball_dx, float& ball_dy, bool multiple_checks = true) {
    int collision_type = 0;
    for (size_t i = 0; i < brickPositionsX.size(); ++i) {
        if (brickActive[i]) {
            float brick_left = brickPositionsX[i];
            float brick_right = brick_left + brickWidth;
            float brick_top = brickPositionsY[i];
            float brick_bottom = brick_top + BRICK_HEIGHT;

            // Check collision with the ball
            if (ball_x + ballRadius > brick_left && ball_x - ballRadius < brick_right &&
                ball_y + ballRadius > brick_top && ball_y - ballRadius < brick_bottom) {
                // Determine points by row
                int row = i / BRICK_COLS;
                if (row < 2) gameScore += 5;       // Top two rows
                else if (row < 4) gameScore += 3;  // Middle two rows
                else gameScore += 10;               // Bottom two rows

                brickActive[i] = false;  // Remove the brick

                // Determine side of collision
                bool hit_vertical = ((ball_x + ballRadius) > brick_left && (ball_x - ballRadius) < brick_right);
                bool hit_horizontal = ((ball_y + ballRadius) > brick_top && (ball_y - ballRadius) < brick_bottom);

                if (hit_vertical && !hit_horizontal) {
                    collision_type = std::max(collision_type, 1); // Side
                }
                else if (!hit_vertical && hit_horizontal) {
                    collision_type = std::max(collision_type, 2); // Top/Bottom
                }
                else if (hit_vertical && hit_horizontal) {
                    collision_type = std::max(collision_type, 3); // Corner
                }

                // If multiple checks is enabled, we continue checking in case we hit multiple bricks
                if (!multiple_checks) {
                    // Otherwise, we stop after hitting a single brick
                    break;
                }
            }
        }
    }
    return collision_type;
}

void handleCollisions() {
    int collision_type = checkWallCollision(ballX, ballY);
    switch (collision_type) {
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

    int brick_collision = checkBrickCollision(ballX, ballY, ballDX, ballDY);
    switch (brick_collision) {
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
    int paddle_collision = checkPaddleCollision();
    switch (paddle_collision) {
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
    // Display the appropriate game over text
    glUseProgram(0); // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST); // Disable depth testing
    glDisable(GL_TEXTURE_2D); // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    printPressKeyToContinue();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void handleGameOver() {
    // Display the appropriate game over text
    glUseProgram(0); // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST); // Disable depth testing
    glDisable(GL_TEXTURE_2D); // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (allBricksDestroyed()) {
        printWinMessage();
    }
    else {
        printGameOverText();
    }
    printGameOverOptions();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
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

void displayConstantText() {
    // Ensure OpenGL is in a proper state to render text
    glUseProgram(0); // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST); // Disable depth testing
    glDisable(GL_TEXTURE_2D); // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Text display
    printText();
    printPowerUpStatus();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// GLUT display callback function
void display() {
    // Set dark gray background color
    glClearColor(0.075f, 0.075f, 0.075f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

    updateGameLogic();

    // Static elements
    drawWalls();
    drawWallContours();
    drawBricks();
    drawBrickContours();

    // Dynamic elements
    drawPaddle();
    drawBall();
    drawLaser();

    // Text display
    displayConstantText();

    glutSwapBuffers(); // Swap the buffers to make it visible
    glUseProgram(0);
}

// Initialize OpenGL Graphics
void initOpenGL() {
    // Initialize GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Generate and bind VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Specify attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);

    // 1. Setting the initial window position
    // Get the screen size
    int screen_width = glutGet(GLUT_SCREEN_WIDTH);
    int screen_height = glutGet(GLUT_SCREEN_HEIGHT);

    // Calculate the window's initial position to center it
    int window_pos_x = (screen_width - WINDOW_WIDTH) / 2;
    int window_pos_y = (screen_height - WINDOW_HEIGHT) / 2;
    glutInitWindowPosition(window_pos_x, window_pos_y);

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

    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    return 0;
}
