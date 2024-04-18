// #include <GL/glew.h>
// #include <GL/glut.h>

// const char* vertexShaderSource = R"(
//     #version 330 core
    
//     layout (location = 0) in vec2 aPos;
    
//     uniform vec2 uOffset;
    
//     void main() {
//         gl_Position = vec4(aPos.x + uOffset.x, aPos.y + uOffset.y, 0.0, 1.0);
//     }
// )";

// const char* fragmentShaderSource = R"(
//     #version 330 core
    
//     out vec4 FragColor;
    
//     void main() {
//         FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
//     }
// )";

// GLuint shaderProgram;
// GLint uOffsetLocation;
// GLfloat offsetX = 0.0f;
// GLfloat offsetY = 0.0f;
// const float OFFSET_INCREMENT = 0.01f;

// void display() {
//     glClear(GL_COLOR_BUFFER_BIT);

//     // Set the offset uniform in the shader
//     glUniform2f(uOffsetLocation, offsetX, offsetY);

//     // Draw the triangle
//     glDrawArrays(GL_TRIANGLES, 0, 3);

//     glutSwapBuffers();
// }

// void keyboardFunc(unsigned char key, int x, int y) {
//     switch (key) {
//     case 'w':
//         offsetY += OFFSET_INCREMENT;
//         break;
//     case 's':
//         offsetY -= OFFSET_INCREMENT;
//         break;
//     case 'a':
//         offsetX -= OFFSET_INCREMENT;
//         break;
//     case 'd':
//         offsetX += OFFSET_INCREMENT;
//         break;
//     }
//     glutPostRedisplay();
// }

// void specialFunc(int key, int x, int y) {
//     switch (key) {
//     case GLUT_KEY_UP:
//         offsetY += OFFSET_INCREMENT;
//         break;
//     case GLUT_KEY_DOWN:
//         offsetY -= OFFSET_INCREMENT;
//         break;
//     case GLUT_KEY_LEFT:
//         offsetX -= OFFSET_INCREMENT;
//         break;
//     case GLUT_KEY_RIGHT:
//         offsetX += OFFSET_INCREMENT;
//         break;
//     }
//     glutPostRedisplay();
// }

// int main(int argc, char** argv) {
//     glutInit(&argc, argv);
//     glutCreateWindow("Triangle Example");
//     glutDisplayFunc(display);
//     glutKeyboardFunc(keyboardFunc);
//     glutSpecialFunc(specialFunc);

//     glewInit();

//     // Vertex data for the triangle
//     float vertices[] = {
//         -0.5f, -0.5f,
//          0.5f, -0.5f,
//          0.0f,  0.5f
//     };

//     // Create and bind Vertex Array Object (VAO)
//     GLuint VAO;
//     glGenVertexArrays(1, &VAO);
//     glBindVertexArray(VAO);

//     // Create and bind Vertex Buffer Object (VBO)
//     GLuint VBO;
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//     // Create and compile the vertex shader
//     GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//     glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//     glCompileShader(vertexShader);

//     // Create and compile the fragment shader
//     GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//     glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//     glCompileShader(fragmentShader);

//     // Create and link the shader program
//     shaderProgram = glCreateProgram();
//     glAttachShader(shaderProgram, vertexShader);
//     glAttachShader(shaderProgram, fragmentShader);
//     glLinkProgram(shaderProgram);

//     // Set up vertex attribute pointer
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
//     glEnableVertexAttribArray(0);

//     // Get the location of the offset uniform in the shader
//     uOffsetLocation = glGetUniformLocation(shaderProgram, "uOffset");

//     // Use the shader program
//     glUseProgram(shaderProgram);

//     glutMainLoop();

//     // Clean up
//     glDeleteVertexArrays(1, &VAO);
//     glDeleteBuffers(1, &VBO);
//     glDeleteProgram(shaderProgram);
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);

//     return 0;
// }
