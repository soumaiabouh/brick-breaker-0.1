#include <GL/glew.h>
#include <GL/glut.h>
#include <windows.h>  // For sleep function

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutCreateWindow("Triangle Example");

    glewInit();

    glutMainLoop();

    return 0;
}