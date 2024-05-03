#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <btBulletDynamicsCommon.h>
#include "design.h"

#define GL_CHECK_ERROR() checkOpenGLError(__FILE__, __LINE__)

const float POWERUP_PADDLE_LENGTH = 6.0f;



// Error checking function
void checkOpenGLError(const char* file, int line) {
    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            error = "Unknown error";
            break;
        }
        std::cerr << "OpenGL Error: " << error << " - " << file << ":" << line << std::endl;
        // You can choose to exit the program or take other appropriate actions here
    }
}
//rendeing global var
float cuboidMoveSpeed = 0.2f;
glm::vec3 cuboidMoveDirection(0.0f, 0.0f, 0.0f);
int selectedCuboidIndex = 0;
int lossCuboidInidex = 4;
float cuboidMinX = -7.0f;
float cuboidMaxX = 7.0f;

//screen global var
const  int SCREEN_WIDTH = 800;
const  int SCREEN_HEIGHT = 600;

//game global var
std::string STUDENT_ID = "261053234"; //TODO: rename based on who's submitting it
int gameScore = 100;
int livesLeft = 3;
bool gameOver = false;
int lastPowerUpScore = 0;   // Score at last power-up activation
bool powerUpActive = false; // Is a power-up currently active?
int powerUpDuration = 10;

glm::vec3 cameraPos = glm::vec3(3.0f, 15.0f, 45.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::mat4 planeModel = glm::mat4(1.0f);


std::vector<float> ballVertices;
std::vector<unsigned int> ballIndices;

unsigned int shaderProgram;
unsigned int planeVAO;
unsigned int  planeVBO;
btDiscreteDynamicsWorld* dynamicsWorld;
std::vector<btRigidBody*> cuboidRigidBodies;
// Brick configuration
const float brickWidth = 1.0f;      // Decrease the brick width
const float brickHeight = 0.3f;     // Decrease the brick height
const float brickDepth = 0.8f;      // Decrease the brick depth
const float brickSpacing = 0.05f;   // Decrease the brick spacing

// Calculate the starting position of the brick wall
const float brickStartX = -8.0f;    // Adjust the starting X position
const float brickEndX = 8.5f;       // Adjust the ending X position
const float brickStartY = 0.0f;     // Keep the starting Y position at 0
const float brickStartZ = -3.0f;    // Adjust the starting Z position

const int numBrickRows = 9;         // Decrease the number of rows
const int numBrickCols = 16;        // Decrease the number of columns

int remainingBricks = numBrickRows * numBrickCols;


const int numNonBrick = 5;
const int numCuboids = numNonBrick + numBrickRows * numBrickCols + 8;
std::vector<std::vector<unsigned int>> cuboidIndices;

float(*colors)[3] = new float[numCuboids][3];

bool isLifeLost = false;


unsigned int ballVAO;
std::vector<unsigned int> cuboidVAOs;
bool* isBrick;
btRigidBody* ballRigidBody;
std::vector<btCollisionShape*> cuboidShapes;
std::vector<unsigned int> cuboidVBOs;
std::vector<unsigned int> cuboidEBOs;


// Ball variables 
glm::vec3 ballInitialVelocity = glm::vec3(0.0f, 0.0f, -10.0f);
glm::vec3 ballInitialPosition = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 ballVelocity = ballInitialVelocity;
glm::vec3 ballPosition = ballInitialPosition; // Initial velocity
float ballRadius = 0.2f;
int sphereSegments = 40; // Increase segments and rings for a smoother sphere
int sphereRings = 40;

void timer(int value);
void display();
void resetAfterBallLoss();
void handleLifeLost();
void keyboard(unsigned char key, int x, int y);





const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

void main() {
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    float shininess = 32.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)glsl";


void renderText(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

int calculateStringWidth(void* font, const char* string) {
    int width = 0;
    const char* c;
    for (c = string; *c != '\0'; c++) {
        width += glutBitmapWidth(font, *c);
    }
    return width;
}
// 1. TEXT
// Function to render text using GLUT's bitmap fonts
void printText() {
    std::string score_str = "SCORE: " + std::to_string(gameScore);
    std::string lives_str = "LIVES: " + std::to_string(livesLeft);

    const char* score_to_print = score_str.c_str();
    const char* lives_to_print = lives_str.c_str();
    const char* student_id = STUDENT_ID.c_str();

    // Dividing up the width into 3 subcells
    float third_width = SCREEN_WIDTH / 3.0f;

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
    renderText(score_x, 30.0f, GLUT_BITMAP_HELVETICA_18, score_to_print);
    renderText(lives_x, 30.0f, GLUT_BITMAP_HELVETICA_18, lives_to_print);
    renderText(student_id_x, 30.0f, GLUT_BITMAP_HELVETICA_18, student_id);
}

void printGameOverText() {
    gameOver = true; // Set game over flag
    const char* game_over_text = "GAME OVER";
    float text_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, game_over_text);
    float text_x = (SCREEN_WIDTH - text_width) / 2; // Center the text horizontally
    float text_y = SCREEN_HEIGHT / 2; // Position the text vertically in the middle
    glColor3f(1.0f, 0.25f, 0.25f); // Red color for the game over text
    renderText(text_x, text_y, GLUT_BITMAP_HELVETICA_18, game_over_text);
}

void printGameOverOptions() {
    const char* game_over_options = "Press [r] to restart or [q] to quit";
    float options_width = calculateStringWidth(GLUT_BITMAP_9_BY_15, game_over_options);
    float options_x = (SCREEN_WIDTH - options_width) / 2;
    float options_y = SCREEN_HEIGHT / 2 + 50; // Below the game over text
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderText(options_x, options_y, GLUT_BITMAP_9_BY_15, game_over_options);
}

void printPressKeyToContinue() {
    const char* continue_message = "Press any key to continue";
    float message_width = calculateStringWidth(GLUT_BITMAP_9_BY_15, continue_message);
    float message_x = (SCREEN_WIDTH - message_width) / 2;
    float message_y = SCREEN_HEIGHT / 2 + 50; // Below the score or any other central message
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    renderText(message_x, message_y, GLUT_BITMAP_9_BY_15, continue_message);

}

void printWinMessage() {
    const char* win_text = "You Won!";
    float text_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, win_text);
    float text_x = (SCREEN_WIDTH - text_width) / 2;
    float text_y = SCREEN_HEIGHT / 2 - 50; // Position it above the game over options
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for the win text
    renderText(text_x, text_y, GLUT_BITMAP_HELVETICA_18, win_text);
}

void printPowerUpStatus() {
    if (powerUpActive) {
        std::string power_up_text = "Power-Up Active: " + std::to_string(powerUpDuration);
        float text_width = calculateStringWidth(GLUT_BITMAP_9_BY_15, power_up_text.c_str());
        float text_x = (SCREEN_WIDTH - text_width) / 2;
        float text_y = SCREEN_HEIGHT - 15; // Display at the bottom of the screen
        glColor3f(0.0f, 1.0f, 0.5f);
        renderText(text_x, text_y, GLUT_BITMAP_9_BY_15, power_up_text.c_str());
    }
}

void displayConstantText() {
    glUseProgram(0); // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST); // Disable depth testing
    glDisable(GL_TEXTURE_2D); // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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

    glUseProgram(shaderProgram); // Disable custom shaders for text rendering
    glEnable(GL_DEPTH_TEST); // Disable depth testing
    glEnable(GL_TEXTURE_2D); // Disable texturing
}

void handleStartUpKeyPress(unsigned char key, int x, int y) {
    // Restore the original keyboard and display callbacks
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);

    // Restart the game loop
    timer(0);
}

void displayStartUpText() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);  // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST);  // Disable depth testing
    glDisable(GL_TEXTURE_2D);  // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render the "Brick Breaker" text
    const char* message = "BRICK BREAKER";
    float message_width = calculateStringWidth(GLUT_BITMAP_HELVETICA_18, message);
    float message_x = (SCREEN_WIDTH - message_width) / 2;
    float message_y = SCREEN_HEIGHT / 2;
    glColor3f(0.7f, 0.7f, 1.0f);  // Light blue color for the text
    renderText(message_x, message_y, GLUT_BITMAP_HELVETICA_18, message);

    // Render the description text
    const char* description = "Press any key to start!";
    float description_width = calculateStringWidth(GLUT_BITMAP_8_BY_13, description);
    float description_x = (SCREEN_WIDTH - description_width) / 2;
    float description_y = (SCREEN_HEIGHT / 2) + 50.0f;
    glColor3f(1.0f, 1.0f, 1.0f);  // White color for the text
    renderText(description_x, description_y, GLUT_BITMAP_8_BY_13, description);

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}


void displayGameOverScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);  // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST);  // Disable depth testing
    glDisable(GL_TEXTURE_2D);  // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    printGameOverText();
    printGameOverOptions();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);  // Enable custom shaders
    glEnable(GL_DEPTH_TEST);  // Enable depth testing
    glEnable(GL_TEXTURE_2D);  // Enable texturing

    glutSwapBuffers();
}

void resetGame() {
    // Reset game state variables
    remainingBricks = numBrickRows * numBrickCols;
    gameScore = 0;
    livesLeft = 3;
    gameOver = false;

    // Call resetAfterBallLoss() to reset the game state
    resetAfterBallLoss();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // ASCII code for ESC key
        std::cout << "Exiting game." << std::endl;
        glutDestroyWindow(glutGetWindow()); // Close the window and exit GLUT main loop
        return;
    }

    if (key == '1' && gameScore >= lastPowerUpScore + 50) {
        // Activate paddle size powerup
        powerUpActive = true;
        powerUpDuration = 300; 
        lastPowerUpScore = gameScore;
        // Increase the paddle size
        btVector3 paddleHalfExtents = static_cast<btBoxShape*>(cuboidRigidBodies[selectedCuboidIndex]->getCollisionShape())->getHalfExtentsWithMargin();
        paddleHalfExtents.setX(paddleHalfExtents.x() * 1.5f);
        static_cast<btBoxShape*>(cuboidRigidBodies[selectedCuboidIndex]->getCollisionShape())->setImplicitShapeDimensions(paddleHalfExtents);
    }
    else if (key == '2' && gameScore >= lastPowerUpScore + 50) {
        // Activate straight ball powerup
        powerUpActive = true;
        powerUpDuration = 300;
        lastPowerUpScore = gameScore;
        // Set the ball's velocity to straight up along the z-axis
        btVector3 straightVelocity(0.0f, 0.0f, -50.0f);
        ballRigidBody->setLinearVelocity(straightVelocity);
    }
}

void handleGameOverKeyPress(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') {
        // Reset the game
        resetGame();

        // Restore the original keyboard and display callbacks
        glutKeyboardFunc(keyboard);
        glutDisplayFunc(display);

        // Restart the game loop
        timer(0);
    }
    else if (key == 'q' || key == 'Q') {
        // Quit the game
        exit(0);
    }
}


void resetAfterBallLoss() {
    // Reset game state variables
    gameScore = 0;


    if (livesLeft > 0) {

        handleLifeLost();
        // Remove all existing cuboid rigid bodies and shapes
        for (int i = 0; i < numCuboids; ++i) {
            if (cuboidRigidBodies[i] != nullptr) {
                dynamicsWorld->removeRigidBody(cuboidRigidBodies[i]);
                delete cuboidRigidBodies[i]->getMotionState();
                delete cuboidRigidBodies[i];
                delete cuboidShapes[i];
                cuboidRigidBodies[i] = nullptr;
            }
        }

        // Reset the ball position, velocity, and transform
        btTransform ballTransform;
        ballTransform.setIdentity();
        ballTransform.setOrigin(btVector3(ballInitialPosition.x, ballInitialPosition.y, ballInitialPosition.z));
        ballRigidBody->setWorldTransform(ballTransform);
        ballRigidBody->setLinearVelocity(btVector3(ballInitialVelocity.x, ballInitialVelocity.y, ballInitialVelocity.z));
        ballRigidBody->getMotionState()->setWorldTransform(ballTransform);

        // Re-create all cuboids
        createCuboids();
    }
    else {

        std::cout << "DEAD DEad." << std::endl;
        gameOver = true;

        // Set the display callback to display the game over screen
        glutDisplayFunc(displayGameOverScreen);

        // Set the keyboard callback to handle the game over key press
        glutKeyboardFunc(handleGameOverKeyPress);

        // Redisplay the scene
        glutPostRedisplay();
    }
    livesLeft = livesLeft - 1;
}



void handleContinueKeyPress(unsigned char key, int x, int y) {
    // Reset the game after the keypress


    // Restore the original keyboard callback
    glutKeyboardFunc(keyboard);

    // Restore the original display callback
    glutDisplayFunc(display);

    // Restart the game loop
    timer(0);
}

void displayLifeLostScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);  // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST);  // Disable depth testing
    glDisable(GL_TEXTURE_2D);  // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    printPressKeyToContinue();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);  // Enable custom shaders
    glEnable(GL_DEPTH_TEST);  // Enable depth testing
    glEnable(GL_TEXTURE_2D);  // Enable texturing

    glutSwapBuffers();
}




void handleLifeLost() {
    // Set the display callback to display the life lost screen
    glutDisplayFunc(displayLifeLostScreen);

    // Set the keyboard callback to handle the continue key press
    glutKeyboardFunc(handleContinueKeyPress);

    // Redisplay the scene
    glutPostRedisplay();
}

int getPointsForBrickRow(int cuboidIndex) {
    int row = (cuboidIndex - numNonBrick) / numBrickCols;
    if (row >= numBrickRows - 3 && row < numBrickRows) {
        return 3; // Last 3 rows
    }
    else if (row >= numBrickRows - 6 && row < numBrickRows - 3) {
        return 2; // Next 3 rows
    }
    else {
        return 1; // First 3 rows
    }
}


void specialKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        cuboidMoveDirection.x = -1.0f;
        break;
    case GLUT_KEY_RIGHT:
        cuboidMoveDirection.x = 1.0f;
        break;
    default:
        break;
    }
}

void specialKeyboardUp(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
    case GLUT_KEY_RIGHT:
        cuboidMoveDirection.x = 0.0f;
        break;
    default:
        break;
    }
}

class MyContactResultCallback : public btCollisionWorld::ContactResultCallback {
public:
    bool hasContact() const { return m_collisionDetected; }
    const btVector3& getCollisionNormal() const { return m_collisionNormal; }

    btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {
        m_collisionDetected = true;
        m_collisionNormal = cp.m_normalWorldOnB;
        return 0;
    }

private:
    bool m_collisionDetected = false;
    btVector3 m_collisionNormal;
};

// Helper function to add vertex data to the vertices array
void addVertexData(float* vertices, int& index, float x, float y, float z, float nx, float ny, float nz, float tx, float ty) {
    vertices[index++] = x;
    vertices[index++] = y;
    vertices[index++] = z;
    vertices[index++] = nx;
    vertices[index++] = ny;
    vertices[index++] = nz;
    vertices[index++] = tx;
    vertices[index++] = ty;
}

void generateCuboid(float length, float height, float width, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // Generate vertices for the cuboid
    vertices = {
        // Front face
        -length / 2, -height / 2, width / 2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        length / 2, -height / 2, width / 2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        length / 2, height / 2, width / 2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -length / 2, height / 2, width / 2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

        // Back face
        -length / 2, -height / 2, -width / 2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        length / 2, -height / 2, -width / 2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        length / 2, height / 2, -width / 2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -length / 2, height / 2, -width / 2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

        // Left face
        -length / 2, -height / 2, -width / 2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -length / 2, -height / 2, width / 2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -length / 2, height / 2, width / 2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -length / 2, height / 2, -width / 2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // Right face
        length / 2, -height / 2, -width / 2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        length / 2, -height / 2, width / 2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        length / 2, height / 2, width / 2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        length / 2, height / 2, -width / 2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // Top face
        -length / 2, height / 2, width / 2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        length / 2, height / 2, width / 2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        length / 2, height / 2, -width / 2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -length / 2, height / 2, -width / 2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        // Bottom face
        -length / 2, -height / 2, width / 2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        length / 2, -height / 2, width / 2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        length / 2, -height / 2, -width / 2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -length / 2, -height / 2, -width / 2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f
    };

    // Generate indices for the cuboid
    for (int i = 0; i < 6; ++i) {
        int baseIndex = i * 4; // Each face has four vertices
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
    }
}




void generateSphere(float radius, int segments, int rings, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    for (int i = 0; i <= rings; ++i) {
        float v = static_cast<float>(i) / static_cast<float>(rings);
        float phi = v * glm::pi<float>();

        for (int j = 0; j <= segments; ++j) {
            float u = static_cast<float>(j) / static_cast<float>(segments);
            float theta = u * 2.0f * glm::pi<float>();

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            int nextI = (i + 1) % (rings + 1);
            int nextJ = (j + 1) % (segments + 1);

            // Two triangles per quad
            indices.push_back(i * (segments + 1) + j);
            indices.push_back(nextI * (segments + 1) + j);
            indices.push_back(nextI * (segments + 1) + nextJ);

            indices.push_back(i * (segments + 1) + j);
            indices.push_back(nextI * (segments + 1) + nextJ);
            indices.push_back(i * (segments + 1) + nextJ);
        }
    }
}



void display() {
    // Render commands here
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the shader to use
    glUseProgram(shaderProgram);
    GL_CHECK_ERROR();

    planeModel = glm::translate(planeModel, glm::vec3(0.0f, 0.0f, 0.0f));

    model = glm::rotate(model, (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f, glm::vec3(0.5f, 1.0f, 0.0f));

    // Create the view matrix using glm::lookAt
    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    if (viewLoc != -1) {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    if (projectionLoc != -1) {
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set light properties
    glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.2f, 10.0f, 2.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

    // Render the plane with a specific color
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5f, 0.8f, 0.7f); // Set the plane color to a light blue-green
    glBindVertexArray(planeVAO);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(planeModel));
    glDrawArrays(GL_TRIANGLES, 0, 6);



    // Update the position of the selected cuboid based on user input
    btTransform cuboidTransform;
    cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->getWorldTransform(cuboidTransform);
    glm::vec3 cuboidPosition = glm::vec3(cuboidTransform.getOrigin().getX(), cuboidTransform.getOrigin().getY(), cuboidTransform.getOrigin().getZ());

    if (cuboidMoveDirection.x != 0.0f) {
        cuboidPosition += cuboidMoveDirection * cuboidMoveSpeed;

        // Clamp the cuboid position within the x-coordinate limits
        cuboidPosition.x = glm::clamp(cuboidPosition.x, cuboidMinX, cuboidMaxX);

        cuboidTransform.setOrigin(btVector3(cuboidPosition.x, cuboidPosition.y, cuboidPosition.z));
        cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->setWorldTransform(cuboidTransform);

        // Synchronize the Bullet Physics world with the updated cuboid position
        cuboidRigidBodies[selectedCuboidIndex]->setWorldTransform(cuboidTransform);
        cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->setWorldTransform(cuboidTransform);
    }

    // Render the cuboids
    for (int i = 0; i < numCuboids; ++i) {
        // Skip rendering deleted cuboids and non-bricks
        if (cuboidRigidBodies[i] == nullptr || (i >= numNonBrick && !isBrick[i])) {
            continue;
        }

        cuboidRigidBodies[i]->getMotionState()->getWorldTransform(cuboidTransform);
        cuboidPosition = glm::vec3(cuboidTransform.getOrigin().getX(), cuboidTransform.getOrigin().getY(), cuboidTransform.getOrigin().getZ());

        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), colors[i][0], colors[i][1], colors[i][2]);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cuboidPosition);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(cuboidVAOs[i]);
        glDrawElements(GL_TRIANGLES, cuboidIndices[i].size(), GL_UNSIGNED_INT, 0);
    }



    // Render the ball using indexed rendering
    glBindVertexArray(ballVAO);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0f), ballPosition)));
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.0f, 0.0f); // Set ball color to red

    displayConstantText();

    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(ballIndices.size()), GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void handleWinKeyPress(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') {
        // Reset the game
        resetGame();

        // Restore the original keyboard and display callbacks
        glutKeyboardFunc(keyboard);
        glutDisplayFunc(display);

        // Restart the game loop
        timer(0);
    }
    else if (key == 'q' || key == 'Q') {
        // Quit the game
        exit(0);
    }
}
void displayWinScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);  // Disable custom shaders for text rendering
    glDisable(GL_DEPTH_TEST);  // Disable depth testing
    glDisable(GL_TEXTURE_2D);  // Disable texturing

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    printWinMessage();
    printGameOverOptions();

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glUseProgram(shaderProgram);  // Enable custom shaders
    glEnable(GL_DEPTH_TEST);  // Enable depth testing
    glEnable(GL_TEXTURE_2D);  // Enable texturing

    glutSwapBuffers();
}

void timer(int value) {
    // Update ball position and velocity
    float deltaTime = 0.01f;  // Assuming a constant time step for simplicity

    dynamicsWorld->stepSimulation(deltaTime, 10);

    // Check for collisions between the ball and bricks
    for (int i = 0; i < numCuboids; ++i) {
        if (isBrick[i] && cuboidRigidBodies[i] != nullptr) {
            MyContactResultCallback callback;
            dynamicsWorld->contactPairTest(ballRigidBody, cuboidRigidBodies[i], callback);
            GL_CHECK_ERROR();
            if (callback.hasContact()) {

                if (isBrick[i]) {
                    remainingBricks--;
                }
                // Get the collision normal from the callback
                btVector3 collisionNormal = callback.getCollisionNormal();

                // Get the ball's current velocity
                btVector3 ballVelocity = ballRigidBody->getLinearVelocity();

                // Calculate the reflection direction using the formula: R = V - 2(V · N)N
                btVector3 reflectedVelocity = ballVelocity - 2.0f * ballVelocity.dot(collisionNormal) * collisionNormal;

                // Update the ball's velocity with the reflected direction
                ballRigidBody->setLinearVelocity(reflectedVelocity);

                // Remove the cuboid from the dynamics world
                dynamicsWorld->removeRigidBody(cuboidRigidBodies[i]);

                // Update the game score based on the brick row
                gameScore += getPointsForBrickRow(i);

                // Mark the cuboid as deleted
                isBrick[i] = false;
                cuboidRigidBodies[i] = nullptr;
            }
        }
        if (remainingBricks == 0) {
            gameOver = true;

            // Set the display callback to display the win screen
            glutDisplayFunc(displayWinScreen);

            // Set the keyboard callback to handle the win screen key press
            glutKeyboardFunc(handleWinKeyPress);

            // Redisplay the scene
            glutPostRedisplay();

            return;
        }

        if (i == lossCuboidInidex) {
            MyContactResultCallback callback;
            dynamicsWorld->contactPairTest(ballRigidBody, cuboidRigidBodies[i], callback);
            if (callback.hasContact()) {
                // Call the handleLifeLost() function
                resetAfterBallLoss();
                return; // Exit the timer function to pause the game
            }
        }
        if (cuboidRigidBodies[selectedCuboidIndex] != nullptr) {
            MyContactResultCallback callback;
            dynamicsWorld->contactPairTest(ballRigidBody, cuboidRigidBodies[selectedCuboidIndex], callback);
            if (callback.hasContact()) {
                // Get the paddle's current position and dimensions
                btTransform paddleTransform;
                cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->getWorldTransform(paddleTransform);
                btVector3 paddlePosition = paddleTransform.getOrigin();
                btVector3 paddleHalfExtents = static_cast<btBoxShape*>(cuboidRigidBodies[selectedCuboidIndex]->getCollisionShape())->getHalfExtentsWithMargin();

                // Calculate the paddle's zone boundaries
                float paddleLength = paddleHalfExtents.getX() * 2.0f;
                float paddleLeftZone = paddlePosition.getX() - paddleLength / 4.0f;
                float paddleRightZone = paddlePosition.getX() + paddleLength /4.0f;

                // Get the ball's current position
                btTransform ballTransform;
                ballRigidBody->getMotionState()->getWorldTransform(ballTransform);
                btVector3 ballPosition = ballTransform.getOrigin();

                if (ballPosition.getX() < paddleLeftZone) {
                    // Left zone collision
                    std::cout << "Ball collided with the left zone of the paddle." << std::endl;
                    // Modify the ball's velocity to bounce to the left
                    btVector3 newVelocity = ballRigidBody->getLinearVelocity();
                    newVelocity.setX(-std::abs(newVelocity.getX()) - 0.08f);
                    newVelocity.setZ(-std::abs(newVelocity.getZ()));
                    ballRigidBody->setLinearVelocity(newVelocity);
                }
                else if (ballPosition.getX() > paddleRightZone) {
                    // Right zone collision
                    std::cout << "Ball collided with the right zone of the paddle." << std::endl;
                    // Modify the ball's velocity to bounce to the right
                    btVector3 newVelocity = ballRigidBody->getLinearVelocity();
                    newVelocity.setX(std::abs(newVelocity.getX()) + 0.08f);
                    newVelocity.setZ(-std::abs(newVelocity.getZ()));
                    ballRigidBody->setLinearVelocity(newVelocity);
                }
                else {
                    // Middle zone collision
                    std::cout << "Ball collided with the center zone of the paddle." << std::endl;
                    // Modify the ball's velocity to bounce straight up
                    btVector3 newVelocity = ballRigidBody->getLinearVelocity();
                    newVelocity.setX(0.0f);
                    newVelocity.setZ(-std::abs(newVelocity.getZ()));
                    ballRigidBody->setLinearVelocity(newVelocity);
                }
            }
        }

        if (powerUpActive) {
            powerUpDuration--;
            if (powerUpDuration <= 0) {
                powerUpActive = false;
                // Reset the paddle size if paddle size powerup was active
                btCollisionShape* paddleShape = cuboidRigidBodies[selectedCuboidIndex]->getCollisionShape();
                if (paddleShape->getLocalScaling().getX() > 1.0f) {
                    btVector3 paddleHalfExtents = static_cast<btBoxShape*>(paddleShape)->getHalfExtentsWithMargin();
                    paddleHalfExtents.setX(paddleHalfExtents.x() / 1.5f);
                    static_cast<btBoxShape*>(paddleShape)->setImplicitShapeDimensions(paddleHalfExtents);
                }
            }
        }


    }


    // Get the ball's current velocity
    btVector3 currentVelocity = ballRigidBody->getLinearVelocity();

    // Calculate the speed of the ball
    btScalar speed = currentVelocity.length();

    // Define the desired constant speed
    btScalar constantSpeed = 20.0f; // Adjust the value as needed

    // Check if the current speed is not zero to avoid division by zero
    if (speed != 0.0f) {
        // Calculate the velocity direction
        btVector3 velocityDirection = currentVelocity.normalized();

        // Set the new velocity with the constant speed and the current direction
        btVector3 newVelocity = velocityDirection * constantSpeed;

        // Update the ball's velocity
        ballRigidBody->setLinearVelocity(newVelocity);
    }

    btTransform ballTransform;
    ballRigidBody->getMotionState()->getWorldTransform(ballTransform);
    ballPosition = glm::vec3(ballTransform.getOrigin().getX(), ballTransform.getOrigin().getY(), ballTransform.getOrigin().getZ());
    ballVelocity = glm::vec3(ballRigidBody->getLinearVelocity().getX(), ballRigidBody->getLinearVelocity().getY(), ballRigidBody->getLinearVelocity().getZ());
    btVector3 ballVelocity = ballRigidBody->getLinearVelocity();
    ballVelocity.setY(std::min(ballVelocity.getY(), 0.0f));
    ballRigidBody->setLinearVelocity(ballVelocity);

    glutPostRedisplay();
    glutTimerFunc(10, timer, 0);  // Call the timer function every 10 milliseconds
}

bool compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource, unsigned int& shaderProgram) {
    // Shader compilation
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n";
        std::cerr << infoLog << std::endl;

        // Clean up and return false on failure
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return false;
    }

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true; // Return true on success
}

void generatePlane(unsigned int& planeVAO, unsigned int& planeVBO) {
    float planeVertices[] = {
        // Positions          // Normals           // Texture Coords
        -10.0f, 0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,   0.0f,
         10.0f, 0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  10.0f,  0.0f,
         10.0f, 0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  10.0f,  15.0f,
         10.0f, 0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  10.0f,  15.0f,
        -10.0f, 0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  0.0f,   15.0f,
        -10.0f, 0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,   0.0f
    };

    // Generate and bind the VAO and VBO for the plane
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Set vertex attribute pointers for the plane
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void createPlaneRigidBody(btDiscreteDynamicsWorld* dynamicsWorld) {
    // Create the Bullet Physics plane
    btCollisionShape* planeShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btTransform planeTransform;
    planeTransform.setIdentity();
    planeTransform.setOrigin(btVector3(0, 0, 0));
    btScalar planeMass(0.0f); // Set mass to 0 for static objects
    btVector3 planeLocalInertia(0, 0, 0);
    btDefaultMotionState* planeMotionState = new btDefaultMotionState(planeTransform);
    btRigidBody::btRigidBodyConstructionInfo planeRigidBodyCI(planeMass, planeMotionState, planeShape, planeLocalInertia);
    planeRigidBodyCI.m_restitution = 1.0f; // Set restitution to 1 for perfectly elastic collisions
    planeRigidBodyCI.m_friction = 0.0f; // Set friction to 0 to avoid slowing down
    btRigidBody* planeRigidBody = new btRigidBody(planeRigidBodyCI);
    dynamicsWorld->addRigidBody(planeRigidBody);
}

btRigidBody* createCuboidRigidBody(btDiscreteDynamicsWorld* dynamicsWorld, float length, float height, float width, const glm::vec3& position, bool isKinematic) {
    btCollisionShape* cuboidShape = new btBoxShape(btVector3(length / 2.0f, height / 2.0f, width / 2.0f));
    btTransform cuboidTransform;
    cuboidTransform.setIdentity();
    cuboidTransform.setOrigin(btVector3(position.x, position.y, position.z));

    btScalar cuboidMass = isKinematic ? 0.0f : 1.0f;
    btVector3 cuboidLocalInertia(0, 0, 0);
    cuboidShape->calculateLocalInertia(cuboidMass, cuboidLocalInertia);

    btDefaultMotionState* cuboidMotionState = new btDefaultMotionState(cuboidTransform);
    btRigidBody::btRigidBodyConstructionInfo cuboidRigidBodyCI(cuboidMass, cuboidMotionState, cuboidShape, cuboidLocalInertia);
    cuboidRigidBodyCI.m_restitution = 1.0f;
    cuboidRigidBodyCI.m_friction = 0.0f;

    btRigidBody* cuboidRigidBody = new btRigidBody(cuboidRigidBodyCI);
    dynamicsWorld->addRigidBody(cuboidRigidBody);

    return cuboidRigidBody;
}

void createNonBrickCuboids(unsigned int numNonBrick, float(*positions)[3], float(*dimensions)[3], float(*colors)[3], std::vector<unsigned int>& cuboidVAOs, std::vector<unsigned int>& cuboidVBOs, std::vector<unsigned int>& cuboidEBOs, std::vector<std::vector<unsigned int>>& cuboidIndices) {
    for (int i = 0; i < numNonBrick; ++i) {
        // Generate and bind VAO, VBO, and EBO for the cuboid
        glGenVertexArrays(1, &cuboidVAOs[i]);
        glGenBuffers(1, &cuboidVBOs[i]);
        glGenBuffers(1, &cuboidEBOs[i]);
        glBindVertexArray(cuboidVAOs[i]);

        std::vector<float> cuboidVertices;
        generateCuboid(dimensions[i][0], dimensions[i][1], dimensions[i][2], cuboidVertices, cuboidIndices[i]);

        glBindBuffer(GL_ARRAY_BUFFER, cuboidVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, cuboidVertices.size() * sizeof(float), &cuboidVertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cuboidEBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cuboidIndices[i].size() * sizeof(unsigned int), &cuboidIndices[i][0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
}

void createBrickCuboids(int numBrickRows, int numBrickCols, float brickWidth, float brickHeight, float brickDepth, float brickSpacing, float brickStartX, float brickStartY, float brickStartZ, float(*positions)[3], float(*dimensions)[3], float(*colors)[3], std::vector<unsigned int>& cuboidVAOs, std::vector<unsigned int>& cuboidVBOs, std::vector<unsigned int>& cuboidEBOs, std::vector<std::vector<unsigned int>>& cuboidIndices, int brickIndex) {
    for (int row = 0; row < numBrickRows; ++row) {
        for (int col = 0; col < numBrickCols; ++col) {
            float brickX = brickStartX + col * (brickWidth + brickSpacing);
            float brickY = brickStartY + brickHeight / 2.0f;
            float brickZ = brickStartZ - row * (brickDepth + brickSpacing);

            positions[brickIndex][0] = brickX;
            positions[brickIndex][1] = brickY;
            positions[brickIndex][2] = brickZ;

            colors[brickIndex][0] = 0.0f;
            colors[brickIndex][1] = 1.0f;
            colors[brickIndex][2] = 0.0f;

            dimensions[brickIndex][0] = brickWidth;
            dimensions[brickIndex][1] = brickHeight;
            dimensions[brickIndex][2] = brickDepth;

            // Generate and bind VAO, VBO, and EBO for the brick cuboid
            glGenVertexArrays(1, &cuboidVAOs[brickIndex]);
            glGenBuffers(1, &cuboidVBOs[brickIndex]);
            glGenBuffers(1, &cuboidEBOs[brickIndex]);
            glBindVertexArray(cuboidVAOs[brickIndex]);

            std::vector<float> cuboidVertices;
            generateCuboid(dimensions[brickIndex][0], dimensions[brickIndex][1], dimensions[brickIndex][2], cuboidVertices, cuboidIndices[brickIndex]);

            glBindBuffer(GL_ARRAY_BUFFER, cuboidVBOs[brickIndex]);
            glBufferData(GL_ARRAY_BUFFER, cuboidVertices.size() * sizeof(float), &cuboidVertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cuboidEBOs[brickIndex]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, cuboidIndices[brickIndex].size() * sizeof(unsigned int), &cuboidIndices[brickIndex][0], GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            ++brickIndex;
        }
    }
}

void createNonBrickRigidBodies(btDiscreteDynamicsWorld* dynamicsWorld, unsigned int numNonBrick, float(*positions)[3], float(*dimensions)[3], std::vector<btCollisionShape*>& cuboidShapes, std::vector<btRigidBody*>& cuboidRigidBodies) {
    for (int i = 0; i < numNonBrick; ++i) {
        // Create Bullet Physics collision shape for the cuboid
        btVector3 cuboidHalfExtents(dimensions[i][0] / 2.0f, dimensions[i][1] / 2.0f, dimensions[i][2] / 2.0f);
        btCollisionShape* cuboidShape = new btBoxShape(cuboidHalfExtents);

        // Create Bullet Physics rigid body for the cuboid
        btTransform cuboidTransform;
        cuboidTransform.setIdentity();
        cuboidTransform.setOrigin(btVector3(positions[i][0], positions[i][1], positions[i][2]));

        btScalar cuboidMass(0.0f);
        btVector3 cuboidLocalInertia(0, 0, 0);
        cuboidShape->calculateLocalInertia(cuboidMass, cuboidLocalInertia);

        btDefaultMotionState* cuboidMotionState = new btDefaultMotionState(cuboidTransform);
        btRigidBody::btRigidBodyConstructionInfo cuboidRigidBodyCI(cuboidMass, cuboidMotionState, cuboidShape, cuboidLocalInertia);
        cuboidRigidBodyCI.m_restitution = 1.0f;
        cuboidRigidBodyCI.m_friction = 0.0f;

        btRigidBody* cuboidRigidBody = new btRigidBody(cuboidRigidBodyCI);

        // Add the cuboid rigid body to the dynamics world
        dynamicsWorld->addRigidBody(cuboidRigidBody);

        // Store the cuboid collision shape and rigid body in the vectors
        cuboidShapes[i] = cuboidShape;
        cuboidRigidBodies[i] = cuboidRigidBody;
    }
}

void createBrickRigidBodies(btDiscreteDynamicsWorld* dynamicsWorld, int numBrickRows, int numBrickCols, float brickWidth, float brickHeight, float brickDepth, float(*positions)[3], std::vector<btCollisionShape*>& cuboidShapes, std::vector<btRigidBody*>& cuboidRigidBodies, int brickIndex) {
    for (int row = 0; row < numBrickRows; ++row) {
        for (int col = 0; col < numBrickCols; ++col) {
            // Create Bullet Physics collision shape for the brick cuboid
            btVector3 cuboidHalfExtents(brickWidth / 2.0f, brickHeight / 2.0f, brickDepth / 2.0f);
            btCollisionShape* cuboidShape = new btBoxShape(cuboidHalfExtents);

            // Create Bullet Physics rigid body for the brick cuboid
            btTransform cuboidTransform;
            cuboidTransform.setIdentity();
            cuboidTransform.setOrigin(btVector3(positions[brickIndex][0], positions[brickIndex][1], positions[brickIndex][2]));

            btScalar cuboidMass(0.0f);
            btVector3 cuboidLocalInertia(0, 0, 0);
            cuboidShape->calculateLocalInertia(cuboidMass, cuboidLocalInertia);

            btDefaultMotionState* cuboidMotionState = new btDefaultMotionState(cuboidTransform);
            btRigidBody::btRigidBodyConstructionInfo cuboidRigidBodyCI(cuboidMass, cuboidMotionState, cuboidShape, cuboidLocalInertia);
            cuboidRigidBodyCI.m_restitution = 1.0f;
            cuboidRigidBodyCI.m_friction = 0.0f;

            btRigidBody* cuboidRigidBody = new btRigidBody(cuboidRigidBodyCI);

            // Add the brick cuboid rigid body to the dynamics world
            dynamicsWorld->addRigidBody(cuboidRigidBody);

            // Store the brick cuboid collision shape and rigid body in the vectors
            cuboidShapes[brickIndex] = cuboidShape;
            cuboidRigidBodies[brickIndex] = cuboidRigidBody;

            ++brickIndex;
        }
    }
}

void createCuboids() {
    // Brick configuration


    // Calculate the total number of cuboids (including bricks)
    int brickIndex = numNonBrick;
    isBrick = new bool[numCuboids];
    for (int i = 0; i < numCuboids; ++i) {
        isBrick[i] = false;
    }

    float(*positions)[3] = new float[numCuboids][3];
    positions[0][0] = 0.0f; positions[0][1] = 0.0f; positions[0][2] = 13.0f;
    positions[1][0] = 0.0f; positions[1][1] = 0.0f; positions[1][2] = -15.0f;
    positions[2][0] = -10.0f; positions[2][1] = 0.0f; positions[2][2] = 0.0f;
    positions[3][0] = 10.0f; positions[3][1] = 0.0f; positions[3][2] = 0.0f;
    positions[4][0] = 0.0f; positions[4][1] = 0.0f; positions[4][2] = 15.0f;
    // The remaining positions will be filled with brick positions

    float(*dimensions)[3] = new float[numCuboids][3];
    dimensions[0][0] = 4.0f; dimensions[0][1] = 1.0f; dimensions[0][2] = 1.0f;
    dimensions[1][0] = 20.0f; dimensions[1][1] = 1.0f; dimensions[1][2] = 1.0f;
    dimensions[2][0] = 1.0f; dimensions[2][1] = 1.0f; dimensions[2][2] = 30.0f;
    dimensions[3][0] = 1.0f; dimensions[3][1] = 1.0f; dimensions[3][2] = 30.0f;
    dimensions[4][0] = 20.0f; dimensions[4][1] = 0.5f; dimensions[4][2] = 1.0f;
    // The remaining dimensions will be filled with brick dimensions

    colors[0][0] = 1.0f; colors[0][1] = 1.0f; colors[0][2] = 0.0f;
    colors[1][0] = 1.0f; colors[1][1] = 1.0f; colors[1][2] = 0.0f;
    colors[2][0] = 1.0f; colors[2][1] = 1.0f; colors[2][2] = 0.0f;
    colors[3][0] = 1.0f; colors[3][1] = 1.0f; colors[3][2] = 0.0f;
    colors[4][0] = 1.0f; colors[4][1] = 1.0f; colors[4][2] = 0.0f;

    // Calculate the starting position of the brick wall


    cuboidIndices.resize(numCuboids);
    cuboidVAOs.resize(numCuboids);
    cuboidShapes.resize(numCuboids);
    cuboidVBOs.resize(numCuboids);
    cuboidEBOs.resize(numCuboids);
    cuboidRigidBodies.resize(numCuboids);

    createNonBrickCuboids(numNonBrick, positions, dimensions, colors, cuboidVAOs, cuboidVBOs, cuboidEBOs, cuboidIndices);
    createBrickCuboids(numBrickRows, numBrickCols, brickWidth, brickHeight, brickDepth, brickSpacing, brickStartX, brickStartY, brickStartZ, positions, dimensions, colors, cuboidVAOs, cuboidVBOs, cuboidEBOs, cuboidIndices, brickIndex);

    createNonBrickRigidBodies(dynamicsWorld, numNonBrick, positions, dimensions, cuboidShapes, cuboidRigidBodies);
    createBrickRigidBodies(dynamicsWorld, numBrickRows, numBrickCols, brickWidth, brickHeight, brickDepth, positions, cuboidShapes, cuboidRigidBodies, brickIndex);

    for (int i = numNonBrick; i < numNonBrick + numBrickRows * numBrickCols; ++i) {
        isBrick[i] = true;
    }
}

btRigidBody* createBallRigidBody(btDiscreteDynamicsWorld* dynamicsWorld, const glm::vec3& initialPosition, float radius, const glm::vec3& initialVelocity) {
    btCollisionShape* ballShape = new btSphereShape(radius);
    btTransform ballTransform;
    ballTransform.setIdentity();
    ballTransform.setOrigin(btVector3(initialPosition.x, initialPosition.y, initialPosition.z));
    btScalar ballMass(1.0f);
    btVector3 ballLocalInertia(0, 0, 0);
    ballShape->calculateLocalInertia(ballMass, ballLocalInertia);
    btDefaultMotionState* ballMotionState = new btDefaultMotionState(ballTransform);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(ballMass, ballMotionState, ballShape, ballLocalInertia);
    ballRigidBodyCI.m_restitution = 1.0f;
    ballRigidBodyCI.m_friction = 0.0f;
    btRigidBody* ballRigidBody = new btRigidBody(ballRigidBodyCI);
    ballRigidBody->setLinearVelocity(btVector3(initialVelocity.x, initialVelocity.y, initialVelocity.z));
    dynamicsWorld->addRigidBody(ballRigidBody);

    return ballRigidBody;
}

void renderBall(unsigned int& ballVBO, unsigned int& ballEBO)
{
    generateSphere(ballRadius, sphereSegments, sphereRings, ballVertices, ballIndices);
    GL_CHECK_ERROR();

    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    glGenBuffers(1, &ballEBO);
    glBindVertexArray(ballVAO);

    // Buffer the vertex data
    glBufferData(GL_ARRAY_BUFFER, ballVertices.size() * sizeof(float), ballVertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ballIndices.size() * sizeof(unsigned int), ballIndices.data(), GL_STATIC_DRAW);

    // Buffer the index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ballIndices.size() * sizeof(unsigned int), &ballIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind the VAO
    glBindVertexArray(0);
}





int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("LearnOpenGL");
    glutSpecialUpFunc(specialKeyboardUp);


    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }


    // Bullet Physics initialization
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.8, 0));


    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up GLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutTimerFunc(0, timer, 0);


    if (!compileAndLinkShaders(vertexShaderSource, fragmentShaderSource, shaderProgram)) {
        // Handle shader compilation or linking failure
        return -1;
    }


    //gen plane
    generatePlane(planeVAO, planeVBO);


    createPlaneRigidBody(dynamicsWorld);

    createCuboids();

    ballRigidBody = createBallRigidBody(dynamicsWorld, ballInitialPosition, ballRadius, ballInitialVelocity);

    unsigned int ballVBO, ballEBO;
    renderBall(ballVBO, ballEBO);


    glutDisplayFunc(displayStartUpText);
    glutKeyboardFunc(handleStartUpKeyPress);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeyboard);


    // Enter the GLUT event processing loop
    glutMainLoop();

    // Clean up
    // Clean up OpenGL objects
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glDeleteVertexArrays(1, &ballVAO);
    glDeleteBuffers(1, &ballVBO);
    glDeleteBuffers(1, &ballEBO);

    for (int i = 0; i < numCuboids; ++i) {
        glDeleteVertexArrays(1, &cuboidVAOs[i]);
        glDeleteBuffers(1, &cuboidVBOs[i]);
        glDeleteBuffers(1, &cuboidEBOs[i]);
    }


    // Cleanup Bullet Physics objects
    for (int i = 0; i < numCuboids; ++i) {
        if (cuboidRigidBodies[i] != nullptr) {
            dynamicsWorld->removeRigidBody(cuboidRigidBodies[i]);
            delete cuboidRigidBodies[i]->getMotionState();
            delete cuboidRigidBodies[i];
            delete cuboidShapes[i];
        }
    }

    dynamicsWorld->removeRigidBody(ballRigidBody);
    delete ballRigidBody->getMotionState();
    delete ballRigidBody;

    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;


    delete[] isBrick;
    delete[] colors;




    return 0;
}