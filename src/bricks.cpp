#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>

using namespace std;

// Global Vars
int WIDTH = 600;
int HEIGHT = 700;
string STUDENTID = "261053234"; //TODO: rename based on who's submitting it


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

void printText() {
    const char* score = "SCORE";
    const char* lives = "LIVES";
    const char* studentID = "261053234";

    // Dividing up the width into 3 subcells
    float thirdWidth = WIDTH / 3.0f;

    // Calculate x positions to center text
    int scoreWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, score);
    int livesWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, lives);
    int studentIDWidth = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, studentID);

    // Centering within each subcell
    float scoreX = (thirdWidth - scoreWidth) / 2.0f;
    float livesX = thirdWidth + (thirdWidth - livesWidth) / 2.0f;
    float studentIDX = 2 * thirdWidth + (thirdWidth - studentIDWidth) / 2.0f;


    // TODO: calculate the position based on the width of the screen. Center them
    renderBitmapString(scoreX, 30.0f, GLUT_BITMAP_HELVETICA_18, score);
    renderBitmapString(livesX, 30.0f, GLUT_BITMAP_HELVETICA_18, lives);
    renderBitmapString(studentIDX, 30.0f, GLUT_BITMAP_HELVETICA_18, studentID);
}

// GLUT display callback function
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

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
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Brick Breaker – 260979679 & 261053234");

    initOpenGL();  // Initialize OpenGL settings
    glutDisplayFunc(display); // Register the display callback

    glutMainLoop();

    return 0;
}
