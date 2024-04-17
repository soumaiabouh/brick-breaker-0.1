#include <GL/glew.h>
#include <GL/glut.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const char* vertexShaderSource = R"(
    #version 330 core
    
    layout (location = 0) in vec2 aPos;
    
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
    }
)";

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the rectangles
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport((width - WINDOW_WIDTH) / 2, (height - WINDOW_HEIGHT) / 2, WINDOW_WIDTH, WINDOW_HEIGHT);
}

//int main(int argc, char** argv) {
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
//    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
//    glutCreateWindow("Rectangles Example");
//    glutDisplayFunc(display);
//    glutReshapeFunc(reshape);
//
//    glewInit();
//
//    // Vertex data for the rectangles (in NDC)
//    float vertices[] = {
//        // Rectangle 1
//        -0.9f,  0.6f,
//        -0.9f,  0.3f,
//        -0.6f,  0.3f,
//        -0.9f,  0.6f,
//        -0.6f,  0.3f,
//        -0.6f,  0.6f,
//
//        // Rectangle 2
//        -0.4f,  0.6f,
//        -0.4f,  0.3f,
//        -0.1f,  0.3f,
//        -0.4f,  0.6f,
//        -0.1f,  0.3f,
//        -0.1f,  0.6f,
//
//        // Rectangle 3
//         0.1f,  0.6f,
//         0.1f,  0.3f,
//         0.4f,  0.3f,
//         0.1f,  0.6f,
//         0.4f,  0.3f,
//         0.4f,  0.6f,
//
//         // Rectangle 4
//         -0.9f, -0.3f,
//         -0.9f, -0.6f,
//         -0.6f, -0.6f,
//         -0.9f, -0.3f,
//         -0.6f, -0.6f,
//         -0.6f, -0.3f,
//
//         // Rectangle 5
//         -0.4f, -0.3f,
//         -0.4f, -0.6f,
//         -0.1f, -0.6f,
//         -0.4f, -0.3f,
//         -0.1f, -0.6f,
//         -0.1f, -0.3f,
//
//         // Rectangle 6
//          0.1f, -0.3f,
//          0.1f, -0.6f,
//          0.4f, -0.6f,
//          0.1f, -0.3f,
//          0.4f, -0.6f,
//          0.4f, -0.3f
//    };
//
//    // Create and bind Vertex Array Object (VAO)
//    GLuint VAO;
//    glGenVertexArrays(1, &VAO);
//    glBindVertexArray(VAO);
//
//    // Create and bind Vertex Buffer Object (VBO)
//    GLuint VBO;
//    glGenBuffers(1, &VBO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    // Create and compile the vertex shader
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//    glCompileShader(vertexShader);
//
//    // Create and compile the fragment shader
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//    glCompileShader(fragmentShader);
//
//    // Create and link the shader program
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//
//    // Set up vertex attribute pointer
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//
//    // Use the shader program
//    glUseProgram(shaderProgram);
//
//    glutMainLoop();
//
//    // Clean up
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteProgram(shaderProgram);
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    return 0;
//}
