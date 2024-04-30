
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

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

void generateRectangle(float h, float l, float w, float x, float y, float z, float* vertices, int& vertexIndex) {
    // Front face
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y - h / 2.0f, z + w / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f); // Bottom left
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y + h / 2.0f, z + w / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f); // Top left
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y - h / 2.0f, z + w / 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f); // Bottom right
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y + h / 2.0f, z + w / 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); // Top right

    // Back face
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y - h / 2.0f, z - w / 2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f); // Bottom right
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y + h / 2.0f, z - w / 2.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f); // Top right
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y - h / 2.0f, z - w / 2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f); // Bottom left
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y + h / 2.0f, z - w / 2.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f); // Top left

    // Left face
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y - h / 2.0f, z - w / 2.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f); // Bottom left
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y + h / 2.0f, z - w / 2.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f); // Top left
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y - h / 2.0f, z + w / 2.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f); // Bottom right
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y + h / 2.0f, z + w / 2.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f); // Top right

    // Right face
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y - h / 2.0f, z + w / 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f); // Bottom left
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y + h / 2.0f, z + w / 2.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f); // Top left
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y - h / 2.0f, z - w / 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f); // Bottom right
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y + h / 2.0f, z - w / 2.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f); // Top right

    // Top face
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y + h / 2.0f, z - w / 2.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f); // Bottom left
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y + h / 2.0f, z + w / 2.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f); // Top left
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y + h / 2.0f, z - w / 2.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f); // Bottom right
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y + h / 2.0f, z + w / 2.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f); // Top right

    // Bottom face
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y - h / 2.0f, z + w / 2.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f); // Bottom left
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y - h / 2.0f, z + w / 2.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f); // Bottom right
    addVertexData(vertices, vertexIndex, x - l / 2.0f, y - h / 2.0f, z - w / 2.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f); // Top left
    addVertexData(vertices, vertexIndex, x + l / 2.0f, y - h / 2.0f, z - w / 2.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f); // Top right
}



std::vector<float> ballVertices;
std::vector<unsigned int> ballIndices;

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

// Ball variables 
glm::vec3 ballInitialVelocity = glm::vec3(5.0f, 0.0f,6.0f);
glm::vec3 ballInitialPosition = glm::vec3(5.0f, 1.0f, 6.0f);
glm::vec3 ballVelocity = ballInitialVelocity;
glm::vec3 ballPosition = ballInitialPosition; // Initial velocity
float ballRadius = 0.5f;
int sphereSegments = 40; // Increase segments and rings for a smoother sphere
int sphereRings = 40;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set up the window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the window's framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Make the context current
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Vertex data for a plane
    float planeVertices[] = {
        // Positions          // Normals           // Texture Coords
        -10.0f, 0.0f, -10.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
         10.0f, 0.0f, -10.0f,  0.0f,  1.0f,  0.0f,  10.0f,  0.0f,
         10.0f, 0.0f,  10.0f,  0.0f,  1.0f,  0.0f,  10.0f,  10.0f,
         10.0f, 0.0f,  10.0f,  0.0f,  1.0f,  0.0f,  10.0f,  10.0f,
        -10.0f, 0.0f,  10.0f,  0.0f,  1.0f,  0.0f,  0.0f,  10.0f,
        -10.0f, 0.0f, -10.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f
    };

    // Vertex Buffer Object and Vertex Array Object for the plane
    unsigned int planeVAO, planeVBO;

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

    // Brick configuration
    const int numBrickRows = 4;
    const int numBrickCols = 8;
    const float brickWidth =0.5f;
    const float brickHeight = 1.5f;
    const float brickDepth = 1.0f;
    const float brickSpacing = 0.05f;

    // Calculate the total number of cuboids (including bricks)
    const int numNonBrick = 5;
    int brickIndex = numNonBrick;
    const int numCuboids = numNonBrick + numBrickRows * numBrickCols;

    float positions[numCuboids][3] = {
        {0.0f, 0.5f, -4.0f},
        {-4.0f, 0.5f, 0.0f},
        {4.0f, 0.5f,  0.0f},
        {0.0f, 0.0f,  3.0f},
        {0.0f, 1.0f,  3.0f},
        
        // The remaining positions will be filled with brick positions
    };

    float colors[numCuboids][3] = {
        {1.0f, 1.0f, 0.0f}, // Yellow
        {1.0f, 1.0f, 0.0f}, // Yellow
        {1.0f, 1.0f, 0.0f}, // Yellow
        {0.0f, 1.0f, 0.0f}, 
        {1.0f, 1.0f, 0.0f},
        // The remaining colors will be filled with brick colors
    };

    float dimensions[numCuboids][3] = {
        {1.0f, 20.0f, 1.0f}, // Dimensions for cuboid 0 (length, height, width)
        {1.0f, 1.0f, 20.0f}, // Dimensions for cuboid 1 (length, height, width)
        {1.0f, 1.0f, 20.0f}, // Dimensions for cuboid 2 (length, height, width)
        {1.0f, 2.0f, 4.0f},
        {6.5f,1.2f, 0.2f},
        // The remaining dimensions will be filled with brick dimensions
    };

   

    // Calculate the starting position of the brick wall
    const float brickStartX = -3.3f;
    const float brickEndX = 3.3f;

    const float brickStartY = 0.0f; // Adjust the y-coordinate to match the plane level
    const float brickStartZ = -3.5f;
    const float totalBrickWidth = brickEndX - brickStartX;
    // Generate brick positions, colors, and dimensions
     // Start index for bricks in the arrays
    for (int row = 0; row < numBrickRows; ++row) {
        for (int col = 0; col < numBrickCols; ++col) {
            float brickX = brickStartX + (col * totalBrickWidth) / (numBrickCols - 1);
            float brickY = brickStartY + 0.5;
            float brickZ = brickStartZ + row * (brickDepth + brickSpacing);

            positions[brickIndex][0] = brickX;
            positions[brickIndex][1] = brickY;
            positions[brickIndex][2] = brickZ;

            colors[brickIndex][0] = 1.0f;
            colors[brickIndex][1] = 0.0f;
            colors[brickIndex][2] = 0.0f;

            dimensions[brickIndex][0] = brickWidth;
            dimensions[brickIndex][1] = brickHeight;
            dimensions[brickIndex][2] = brickDepth;

            ++brickIndex;
        }
    }

    // Vertex Buffer Object and Vertex Array Object for the cuboids
    unsigned int cuboidVAO, cuboidVBO;
    glGenVertexArrays(1, &cuboidVAO);
    glGenBuffers(1, &cuboidVBO);
    glBindVertexArray(cuboidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cuboidVBO);

    // Generate cuboid vertex data and buffer it
    int totalVertices = 0;
    for (int i = 0; i < numCuboids; ++i) {
        totalVertices += 24; // Each cuboid has 24 vertices
    }
    float* cuboidVertices = new float[totalVertices * 8]; // Each vertex has 8 floats (3 position, 3 normal, 2 texture coordinates)
    int vertexIndex = 0;

    for (int i = 0; i < numCuboids; ++i) {
        generateRectangle(dimensions[i][0], dimensions[i][1], dimensions[i][2], positions[i][0], positions[i][1], positions[i][2], cuboidVertices, vertexIndex);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * totalVertices * 8, cuboidVertices, GL_STATIC_DRAW);
    // Set vertex attribute pointers for the rectangles
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Generate sphere vertices and indices
    std::vector<float> ballVertices;
    std::vector<unsigned int> ballIndices;
    generateSphere(ballRadius, sphereSegments, sphereRings, ballVertices, ballIndices);

    // Create and bind the VAO for the ball
    unsigned int ballVAO, ballVBO, ballEBO;
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    glGenBuffers(1, &ballEBO);
    glBindVertexArray(ballVAO);

    // Buffer the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    glBufferData(GL_ARRAY_BUFFER, ballVertices.size() * sizeof(float), &ballVertices[0], GL_STATIC_DRAW);

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
    delete[] cuboidVertices; // Free the dynamically allocated memory


    // Shader compilation
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader program
    unsigned int shaderProgram = glCreateProgram();
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
    }

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Update ball position and velocity
        float deltaTime = 0.01f; // Assuming a constant time step for simplicity
        ballPosition += ballVelocity * deltaTime;

        // Check collision with plane limits
        if (ballPosition.x - ballRadius < -10.0f || ballPosition.x + ballRadius > 10.0f) {
            ballVelocity.x = -ballVelocity.x;
        }
        if (ballPosition.z - ballRadius < -10.0f || ballPosition.z + ballRadius > 10.0f) {
            ballVelocity.z = -ballVelocity.z;
        }

        // Check collision with cuboids
        for (int i = 0; i < numCuboids; ++i) {
            float h = dimensions[i][0];
            float l = dimensions[i][1];
            float w = dimensions[i][2];
            float x = positions[i][0];
            float y = positions[i][1];
            float z = positions[i][2];

            // Calculate the minimum and maximum coordinates of the cuboid
            glm::vec3 cuboidMin = glm::vec3(x - l / 2.0f, y - h / 2.0f, z - w / 2.0f);
            glm::vec3 cuboidMax = glm::vec3(x + l / 2.0f, y + h / 2.0f, z + w / 2.0f);

            // Calculate the minimum and maximum coordinates of the ball
            glm::vec3 ballMin = ballPosition - glm::vec3(ballRadius);
            glm::vec3 ballMax = ballPosition + glm::vec3(ballRadius);

            // Check for overlap between the ball and cuboid on each axis
            bool collisionX = ballMax.x >= cuboidMin.x && ballMin.x <= cuboidMax.x;
            bool collisionY = ballMax.y >= cuboidMin.y && ballMin.y <= cuboidMax.y;
            bool collisionZ = ballMax.z >= cuboidMin.z && ballMin.z <= cuboidMax.z;

            // If there is a collision on all three axes, resolve the collision
            if (collisionX && collisionY && collisionZ) {
                // Calculate the overlap on each axis
                float overlapX = std::min(ballMax.x - cuboidMin.x, cuboidMax.x - ballMin.x);
                float overlapY = std::min(ballMax.y - cuboidMin.y, cuboidMax.y - ballMin.y);
                float overlapZ = std::min(ballMax.z - cuboidMin.z, cuboidMax.z - ballMin.z);

                // Find the axis with the minimum overlap
                if (overlapX < overlapY && overlapX < overlapZ) {
                    // Collision on the X-axis
                    if (ballPosition.x < x) {
                        // Ball is on the left side of the cuboid
                        ballPosition.x -= overlapX;
                    }
                    else {
                        // Ball is on the right side of the cuboid
                        ballPosition.x += overlapX;
                    }
                    ballVelocity.x = -ballVelocity.x;
                }
                else if (overlapY < overlapX && overlapY < overlapZ) {
                    // Collision on the Y-axis
                    if (ballPosition.y < y) {
                        // Ball is below the cuboid
                        ballPosition.y -= overlapY;
                    }
                    else {
                        // Ball is above the cuboid
                        ballPosition.y += overlapY;
                    }
                    ballVelocity.y = -ballVelocity.y;
                }
                else {
                    // Collision on the Z-axis
                    if (ballPosition.z < z) {
                        // Ball is behind the cuboid
                        ballPosition.z -= overlapZ;
                    }
                    else {
                        // Ball is in front of the cuboid
                        ballPosition.z += overlapZ;
                    }
                    ballVelocity.z = -ballVelocity.z;
                }
            }
        }

        // Ensure ball stays at a fixed y-coordinate

        ballPosition.y = 1.0f;

        // Render commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the shader to use
        glUseProgram(shaderProgram);

        // Set up the transformation matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // Set up the transformation matrices for the plane
        glm::mat4 planeModel = glm::mat4(1.0f);
        planeModel = glm::translate(planeModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Slightly below the origin

        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

        // Define camera position and orientation
        glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 20.0f); // Position the camera at (0, 0, 20)
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Looking at the origin
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Up vector is the positive y-axis

        // Create the view matrix using glm::lookAt
        view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        // Set the uniform variables in the shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
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

        // Render the cuboids
        for (int i = 0; i < numCuboids; ++i) {
            glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), colors[i][0], colors[i][1], colors[i][2]);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(positions[i][0], positions[i][1], positions[i][2]));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cuboidVAO);

            // Each cuboid has 6 faces, and each face is drawn with 4 vertices (2 triangles)
            for (int face = 0; face < 6; ++face) {
                // Calculate the starting vertex index for the current face
                int startVertex = i * 24 + face * 4;
                glDrawArrays(GL_TRIANGLE_STRIP, startVertex, 4);
            }
        }

        // Render the ball using indexed rendering
        glBindVertexArray(ballVAO);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0f), ballPosition)));
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(ballIndices.size()), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &cuboidVAO);
    glDeleteBuffers(1, &cuboidVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &ballEBO);


    // Terminate GLFW
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}
