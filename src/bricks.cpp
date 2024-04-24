#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <vector>


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
int paddle_length = 96;  // Total length of the paddle
int paddle_x = (WIDTH - paddle_length)/2;  // Starting x position
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


// 1. TEXT
// Function to render text using GLUT's bitmap fonts
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
    renderBitmapString(scoreX, 30.0f, GLUT_BITMAP_HELVETICA_18, score_to_print);
    renderBitmapString(livesX, 30.0f, GLUT_BITMAP_HELVETICA_18, lives_to_print);
    renderBitmapString(studentIDX, 30.0f, GLUT_BITMAP_HELVETICA_18, studentID);
}

// 2. STATIC ELEMENTS
// Function to draw a wall given bottom-left and top-right coordinates
void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS); // Begin drawing a quad
    glVertex2f(x1, y1); // Bottom left
    glVertex2f(x2, y1); // Bottom right
    glVertex2f(x2, y2); // Top right
    glVertex2f(x1, y2); // Top left
    glEnd(); // End drawing the quad
}

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


// 3. DYNAMIC ELEMENTS

// 3.1 Paddle
void drawPaddle() {
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

// GLUT display callback function
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen
    
    // Static elements
    drawWalls();
    drawBricks();

    // Dynamic elements
    drawPaddle();

    // Set the color for the text
    glColor3f(1.0f, 1.0f, 1.0f); // White 
    printText();

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

// Keyboard handling
void keyboardHandler(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        paddle_x -= 10; // Move paddle left
        if (paddle_x < 0) paddle_x = 0; // Keep paddle within the screen bounds
        break;
    case 'd':
    case 'D':
        paddle_x += 10; // Move paddle right
        if (paddle_x > WIDTH - paddle_length) paddle_x = WIDTH - paddle_length;
        break;
    }
    glutPostRedisplay(); // Request display update
}

void specialInput(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        paddle_x -= 10;
        if (paddle_x < 0) paddle_x = 0;
        break;
    case GLUT_KEY_RIGHT:
        paddle_x += 10;
        if (paddle_x > WIDTH - paddle_length) paddle_x = WIDTH - paddle_length;
        break;
    }
    glutPostRedisplay();
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
    glutSpecialFunc(specialInput);  // Register special key handler (arrow keys)

    glutMainLoop();
    return 0;
}
