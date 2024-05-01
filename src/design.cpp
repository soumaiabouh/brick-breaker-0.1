
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <btBulletDynamicsCommon.h>

float cuboidMoveSpeed = 0.2f;
glm::vec3 cuboidMoveDirection(0.0f, 0.0f, 0.0f);
int selectedCuboidIndex = 0;
float cuboidMinX = -7.0f;
float cuboidMaxX = 7.0f;

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
    indices = {
        0, 1, 2, 0, 2, 3,       // Front face
        4, 5, 6, 4, 6, 7,       // Back face
        8, 9, 10, 8, 10, 11,    // Left face
        12, 13, 14, 12, 14, 15, // Right face
        16, 17, 18, 16, 18, 19, // Top face
        20, 21, 22, 20, 22, 23  // Bottom face
    };
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
glm::vec3 ballInitialVelocity = glm::vec3(0.0f, 0.0f, -10.0f);
glm::vec3 ballInitialPosition = glm::vec3(4.0f, 0.0f,9.0f);
glm::vec3 ballVelocity = ballInitialVelocity;
glm::vec3 ballPosition = ballInitialPosition; // Initial velocity
float ballRadius = 0.2f;
int sphereSegments = 40; // Increase segments and rings for a smoother sphere
int sphereRings = 40;

int main() {

    // Bullet Physics initialization
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
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

    float planeVertices[] = {
        // Positions          // Normals           // Texture Coords
        -10.0f, 0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,   0.0f,
         10.0f, 0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  10.0f,  0.0f,
         10.0f, 0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  10.0f,  15.0f,
         10.0f, 0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  10.0f,  15.0f,
        -10.0f, 0.0f,  15.0f,  0.0f,  1.0f,  0.0f,  0.0f,   15.0f,
        -10.0f, 0.0f, -15.0f,  0.0f,  1.0f,  0.0f,  0.0f,   0.0f
    };



    // Vertex Buffer Object and Vertex Array Object for the plane
    unsigned int planeVAO, planeVBO;

    // Generate and bind the VAO and VBO for the planeradius
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
    const int numBrickRows = 12;
    const int numBrickCols = 10;
    const float brickWidth = 1.5f;
    const float brickHeight = 0.5f;
    const float brickDepth = 1.0f;
    const float brickSpacing = 0.1f;

    // Calculate the total number of cuboids (including bricks)
    const int numNonBrick = 5;
    int brickIndex = numNonBrick;
    const int numCuboids = numNonBrick + numBrickRows * numBrickCols;
    float positions[numCuboids][3] = {
        {0.0f, 0.0f, 13.0f},
        {0.0f, 0.0f, -15.0f},
        {-10.0f, 0.0f, 0.0f},
        {10.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 15.0f}
        // The remaining positions will be filled with brick positions
    };

    float colors[numCuboids][3] = {
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f}
        // The remaining colors will be filled with brick colors
    };

    float dimensions[numCuboids][3] = {
        {4.0f, 1.0f, 1.0f},
        {20.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 30.0f},
        {1.0f, 1.0f, 30.0f},
        {20.0f, 1.0f, 1.0f}
        // The remaining dimensions will be filled with brick dimensions
    };

    // Calculate the starting position of the brick wall
    const float brickStartX = -7.0f;
    const float brickEndX = 7.0f;
    const float brickStartY = 0.0f;
    const float brickStartZ = -10.0f;
    const float totalBrickWidth = brickEndX - brickStartX;
    const float totalBrickDepth = numBrickRows * (brickDepth + brickSpacing);

    for (int row = 0; row < numBrickRows; ++row) {
        for (int col = 0; col < numBrickCols; ++col) {
            float brickX = brickStartX + col * (brickWidth + brickSpacing);
            float brickY = brickStartY + brickHeight / 2.0f;
            float brickZ = brickStartZ + row * (brickDepth + brickSpacing);

            positions[brickIndex][0] = brickX;
            positions[brickIndex][1] = brickY;
            positions[brickIndex][2] = brickZ;

            colors[brickIndex][0] = 0.0f;
            colors[brickIndex][1] = 1.0f;
            colors[brickIndex][2] = 0.0f;

            dimensions[brickIndex][0] = brickWidth;
            dimensions[brickIndex][1] = brickHeight;
            dimensions[brickIndex][2] = brickDepth;

            ++brickIndex;
        }
    }

    bool isBrick[numCuboids] = { false };
    for (int i = numNonBrick; i < numCuboids; ++i) {
        isBrick[i] = true;
    }
    

    std::vector<btCollisionShape*> cuboidShapes(numCuboids);
    std::vector<btRigidBody*> cuboidRigidBodies(numCuboids);
    std::vector<unsigned int> cuboidVAOs(numCuboids);
    std::vector<unsigned int> cuboidVBOs(numCuboids);
    std::vector<unsigned int> cuboidEBOs(numCuboids);
    std::vector<std::vector<unsigned int>> cuboidIndices(numCuboids);

    for (int i = 0; i < numCuboids; ++i) {
        // Create Bullet Physics collision shape for the cuboid
        btVector3 cuboidHalfExtents(dimensions[i][0] / 2.0f, dimensions[i][1] / 2.0f, dimensions[i][2] / 2.0f);
        btCollisionShape* cuboidShape = new btBoxShape(cuboidHalfExtents);

        // Create Bullet Physics rigid body for the cuboid
        btTransform cuboidTransform;
        cuboidTransform.setIdentity();
        cuboidTransform.setOrigin(btVector3(positions[i][0], positions[i][1], positions[i][2]));

        btScalar cuboidMass(0.0f); // Set mass to a non-zero value for dynamic objects
        btVector3 cuboidLocalInertia(0, 0, 0);
        cuboidShape->calculateLocalInertia(cuboidMass, cuboidLocalInertia);

        btDefaultMotionState* cuboidMotionState = new btDefaultMotionState(cuboidTransform);
        btRigidBody::btRigidBodyConstructionInfo cuboidRigidBodyCI(cuboidMass, cuboidMotionState, cuboidShape, cuboidLocalInertia);
        cuboidRigidBodyCI.m_restitution = 1.0f;
        cuboidRigidBodyCI.m_friction = 0.0f;

        btRigidBody* cuboidRigidBody = new btRigidBody(cuboidRigidBodyCI);

        if (i == selectedCuboidIndex) {
            // Enable collision detection for the selected cuboid
            cuboidRigidBody->setCollisionFlags(cuboidRigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
            cuboidRigidBody->setCollisionFlags(cuboidRigidBody->getCollisionFlags() | btCollisionObject::CF_DYNAMIC_OBJECT);
        }


        // Add the cuboid rigid body to the dynamics world
        dynamicsWorld->addRigidBody(cuboidRigidBody);

        // Store the cuboid collision shape and rigid body in the vectors
        cuboidShapes[i] = cuboidShape;
        cuboidRigidBodies[i] = cuboidRigidBody;

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


    



    
    
    
    // Create Bullet Physics shape and rigid body for the ball
    btCollisionShape* ballShape = new btSphereShape(ballRadius);
    btTransform ballTransform;
    ballTransform.setIdentity();
    ballTransform.setOrigin(btVector3(ballPosition.x, ballPosition.y, ballPosition.z));
    btScalar ballMass(1.0f);
    btVector3 ballLocalInertia(0, 0, 0);
    ballShape->calculateLocalInertia(ballMass, ballLocalInertia);
    btDefaultMotionState* ballMotionState = new btDefaultMotionState(ballTransform);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(ballMass, ballMotionState, ballShape, ballLocalInertia);
    ballRigidBodyCI.m_restitution = 1.0f;
    ballRigidBodyCI.m_friction = 0.0f;
    btRigidBody* ballRigidBody = new btRigidBody(ballRigidBodyCI);
    ballRigidBody->setLinearVelocity(btVector3(ballVelocity.x, ballVelocity.y, ballVelocity.z));
    dynamicsWorld->addRigidBody(ballRigidBody);
    // COLLISION ///////////////////////

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
        //ballPosition += ballVelocity * deltaTime;

        // Update ball position and velocity using Bullet Physics simulation
        dynamicsWorld->stepSimulation(deltaTime, 10);

        // In the main loop, after the physics simulation step
        for (int i = 0; i < numCuboids; ++i) {
            if (isBrick[i] && cuboidRigidBodies[i] != nullptr) {
                MyContactResultCallback callback;
                dynamicsWorld->contactPairTest(ballRigidBody, cuboidRigidBodies[i], callback);

                if (callback.hasContact()) {
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

                    // Delete the cuboid's collision shape, rigid body, and motion state
                    delete cuboidRigidBodies[i]->getMotionState();
                    delete cuboidRigidBodies[i];
                    delete cuboidShapes[i];

                    // Remove the cuboid's VAO, VBO, and EBO
                    glDeleteVertexArrays(1, &cuboidVAOs[i]);
                    glDeleteBuffers(1, &cuboidVBOs[i]);
                    glDeleteBuffers(1, &cuboidEBOs[i]);

                    // Mark the cuboid as deleted
                    isBrick[i] = false;

                    // Set the corresponding rigid body and collision shape pointers to nullptr
                    cuboidRigidBodies[i] = nullptr;
                    cuboidShapes[i] = nullptr;
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
        // Render commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the shader to use
        glUseProgram(shaderProgram);

        // Set up the transformation matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        glm::mat4 planeModel = glm::mat4(1.0f);
        planeModel = glm::translate(planeModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate the plane to y = -3




        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

        glm::vec3 cameraPos = glm::vec3(3.0f, 15.0f, 25.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

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

        // Get the final model matrix of the rendering plane
        glm::mat4 finalPlaneModel = planeModel * model;
        glm::vec3 planePosition = glm::vec3(finalPlaneModel[3][0], finalPlaneModel[3][1], finalPlaneModel[3][2]);

        // Create the Bullet Physics plane at the same position as the rendering plane
        btCollisionShape* planeShape = new btStaticPlaneShape(btVector3(0, 1, 0), planePosition.y);
        btTransform planeTransform;
        planeTransform.setIdentity();
        planeTransform.setOrigin(btVector3(planePosition.x, planePosition.y, planePosition.z));
        btScalar planeMass(0.0f); // Set mass to 0 for static objects
        btVector3 planeLocalInertia(0, 0, 0);
        btDefaultMotionState* planeMotionState = new btDefaultMotionState(planeTransform);
        btRigidBody::btRigidBodyConstructionInfo planeRigidBodyCI(planeMass, planeMotionState, planeShape, planeLocalInertia);
        planeRigidBodyCI.m_restitution = 1.0f; // Set restitution to 1 for perfectly elastic collisions
        planeRigidBodyCI.m_friction = 0.0f; // Set friction to 0 to avoid slowing down
        btRigidBody* planeRigidBody = new btRigidBody(planeRigidBodyCI);
        dynamicsWorld->addRigidBody(planeRigidBody);

        // Update the position of the selected cuboid based on user input
        int selectedCuboidIndex = 0;
        btTransform cuboidTransform;
        cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->getWorldTransform(cuboidTransform);
        glm::vec3 cuboidPosition = glm::vec3(cuboidTransform.getOrigin().getX(), cuboidTransform.getOrigin().getY(), cuboidTransform.getOrigin().getZ());
        cuboidPosition += cuboidMoveDirection * cuboidMoveSpeed;

        // Clamp the cuboid position within the x-coordinate limits
        cuboidPosition.x = glm::clamp(cuboidPosition.x, cuboidMinX, cuboidMaxX);

        cuboidTransform.setOrigin(btVector3(cuboidPosition.x, cuboidPosition.y, cuboidPosition.z));
        cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->setWorldTransform(cuboidTransform);

        // Synchronize the Bullet Physics world with the updated cuboid position
        cuboidRigidBodies[selectedCuboidIndex]->setWorldTransform(cuboidTransform);
        cuboidRigidBodies[selectedCuboidIndex]->getMotionState()->setWorldTransform(cuboidTransform);


        // Render the cuboids
        for (int i = 0; i < numCuboids; ++i) {
            // Skip rendering deleted cuboids
            if (cuboidRigidBodies[i] == nullptr) {
                continue;
            }

            btTransform cuboidTransform;
            cuboidRigidBodies[i]->getMotionState()->getWorldTransform(cuboidTransform);
            glm::vec3 cuboidPosition = glm::vec3(cuboidTransform.getOrigin().getX(), cuboidTransform.getOrigin().getY(), cuboidTransform.getOrigin().getZ());

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
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(ballIndices.size()), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
 
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &ballEBO);

    // Cleanup Bullet Physics objects
    for (int i = 0; i < numCuboids; ++i) {
        dynamicsWorld->removeRigidBody(cuboidRigidBodies[i]);
        delete cuboidRigidBodies[i]->getMotionState();
        delete cuboidRigidBodies[i];
        delete cuboidShapes[i];
    }

    dynamicsWorld->removeRigidBody(ballRigidBody);
    delete ballRigidBody->getMotionState();
    delete ballRigidBody;
    delete ballShape;
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    // Clean up
    for (int i = 0; i < numCuboids; ++i) {
        glDeleteVertexArrays(1, &cuboidVAOs[i]);
        glDeleteBuffers(1, &cuboidVBOs[i]);
        glDeleteBuffers(1, &cuboidEBOs[i]);
    }

  

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

    // Update cuboid move direction based on left and right arrow key inputs
    cuboidMoveDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cuboidMoveDirection.x = -1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cuboidMoveDirection.x = 1.0f;
    }
}
