#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string>


// Global Vars
int WIDTH = 600;
int HEIGHT = 700;
std::string STUDENTID = "261053234"; //TODO: rename based on who's submitting it
int score = 0;
int lives = 3;


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

// Function to draw a wall given bottom-left and top-right coordinates
void drawWall(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS); // Begin drawing a quad
    glVertex2f(x1, y1); // Bottom left
    glVertex2f(x2, y1); // Bottom right
    glVertex2f(x2, y2); // Top right
    glVertex2f(x1, y2); // Top left
    glEnd(); // End drawing the quad
}

// GLUT display callback function
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen
    
    // Draw walls
    glColor3f(0.75f, 0.75f, 0.75f); // Red color for walls
    drawWall(0.0f, 500.0f, 20.0f, 60.0f);
    drawWall(580.0f, 500.0f, 600.0f, 60.0f);
    drawWall(0.0f, 60.0f, 600.0f, 40.0f);


    // Set the color for the text
    glColor3f(1.0, 1.0, 1.0); // White color

    // printing the text we want to see displayed
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

    initOpenGL();  // Initialize OpenGL settings
    
    glutDisplayFunc(display); // Register the display callback
    
    // Register the reshape callback
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}
