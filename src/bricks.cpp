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

void printText() {
    renderBitmapString(10.0f, 30.0f, GLUT_BITMAP_HELVETICA_18, "SCORE");
    renderBitmapString(100.0f, 30.0f, GLUT_BITMAP_HELVETICA_18, "LIVES");
    renderBitmapString(200.0f, 30.0f, GLUT_BITMAP_HELVETICA_18, "261053234");
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
