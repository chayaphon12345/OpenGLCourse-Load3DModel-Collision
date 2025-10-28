#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(vector<std::string> faces);
void planeTurn(float direction);
bool checkSphereBoxCollision(glm::vec3 sphereCenter, float sphereRadius, glm::vec3 boxCenter, glm::vec3 boxHalfSize);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera firstPersonCamera(glm::vec3(0.0f, 0.9f, -0.45f));  
Camera thirdPersonCamera(glm::vec3(0.0f, 2.0f, 5.0f));  
bool useThirdPersonCamera = false;  
glm::vec3 cockpitOffsetLocal(0.0f, 0.9f, -0.45f);
float cameraYawOffset = 0.0f;  // mouse relative yaw
float cameraPitchOffset = 0.0f; // mouse relative pitch
float cameraMoveSpeed = 0.5f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 planePosition = glm::vec3(60.0f, 400.0f, 8000.0f);
float accelerate = 10.0f;
float maxSpeed = 80.0f;
float avgSpeed = 50.0f;
float minSpeed = 30.0f;
float planeSpeed = avgSpeed;
bool planeTurning = false;
bool planeModSpeed = false;
float planeScale = 0.2f;
float planeYaw = 0.0f; // left-right
float yawSpeed = 20.0f;
float planePitch = 0.0f; // up-down
float pitchSpeed = 20.0f;
float planeRoll = 0.0f; // swing sideward
float rollSpeed = 20.0f;

glm::vec3 bombPosition = glm::vec3(0.0f, -0.5f, 1.8f);
float bombScale = 5.0f;
float bombYaw = 0.0f;
float bombPitch = 0.0f;
float bombRoll = 0.0f; 
bool bombAttached = true;      
bool bombReleased = false;     
glm::vec3 bombVelocity(0.0f);  
float gravity = -9.81f;        

int hitCount = 0;
bool bombHit = false;
float bombHitRadius = 0.3f; 

bool showExplosion = false;
glm::vec3 explosionPosition(0.0f);
float explosionScale = 5.0f;

bool showHitboxes = false;

glm::vec3 shipBoxHalfSize(15.0f, 10.0f, 100.0f);
glm::vec3 shipPosition(0.0f, -5.0f, 0.0f);
float shipScale = 60.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
    Shader skyboxShader("6.1.skybox.vs", "6.1.skybox.fs");
    Shader hitboxShader("hitbox.vs", "hitbox.fs");


    // load models
    // -----------
    Model ourModel(FileSystem::getPath("resources/objects/bomber_plane/untitled.obj"));
    Model shipModel(FileSystem::getPath("resources/objects/ijn_akagi/akagi.obj"));
    Model bombModel(FileSystem::getPath("resources/objects/bomb/bomb.obj"));
    Model explosionModel(FileSystem::getPath("resources/objects/explosion/explosion.obj"));

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // Ship hitbox setup
    unsigned int hitboxVAO, hitboxVBO;
    float hitboxVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };
    unsigned int hitboxIndices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, // back face
        4, 5, 5, 6, 6, 7, 7, 4, // front face
        0, 4, 1, 5, 2, 6, 3, 7  // sides
    };
    unsigned int hitboxEBO;
    glGenVertexArrays(1, &hitboxVAO);
    glGenBuffers(1, &hitboxVBO);
    glGenBuffers(1, &hitboxEBO);
    glBindVertexArray(hitboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hitboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hitboxVertices), hitboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hitboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hitboxIndices), hitboxIndices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Bomb hitbox sphere setup
    unsigned int bombSphereVAO, bombSphereVBO;
    std::vector<float> bombSphereVertices;
    const int sectorCount = 24;
    const int stackCount = 12;
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
        float xy = cosf(stackAngle);
        float z = sinf(stackAngle);
        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            bombSphereVertices.push_back(x);
            bombSphereVertices.push_back(y);
            bombSphereVertices.push_back(z);
        }
    }
    glGenVertexArrays(1, &bombSphereVAO);
    glGenBuffers(1, &bombSphereVBO);
    glBindVertexArray(bombSphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bombSphereVBO);
    glBufferData(GL_ARRAY_BUFFER, bombSphereVertices.size() * sizeof(float), bombSphereVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);


    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/skybox/ocean/right.jpg"),
        FileSystem::getPath("resources/textures/skybox/ocean/left.jpg"),
        FileSystem::getPath("resources/textures/skybox/ocean/top.jpg"),
        FileSystem::getPath("resources/textures/skybox/ocean/bottom.jpg"),
        FileSystem::getPath("resources/textures/skybox/ocean/front.jpg"),
        FileSystem::getPath("resources/textures/skybox/ocean/back.jpg")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        if (planeTurning == false) {
            if (planeRoll > 0.0f) {
                planeRoll -= rollSpeed * deltaTime * 0.8;
                planeRoll = max(planeRoll, 0.0f);
            }
            if (planeRoll < 0.0f) {
                planeRoll += rollSpeed * deltaTime * 0.8;
                planeRoll = min(planeRoll, 0.0f);
            }
        }
        planeTurning = false;

        if (planeModSpeed == false) {
            if (planeSpeed > avgSpeed) {
                planeSpeed -= accelerate * deltaTime * 0.5;
                planeSpeed = max(planeSpeed, avgSpeed);
            }
            if (planeSpeed < avgSpeed) {
                planeSpeed += accelerate * deltaTime * 0.5;
                planeSpeed = min(planeSpeed, avgSpeed);
            }
        }
        planeModSpeed = false;
        //std::cout << "planeSpeed: " << planeSpeed << std::endl;

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        Camera& activeCamera = useThirdPersonCamera ? thirdPersonCamera : firstPersonCamera;

        // Plane rotation
        glm::mat4 planeRotationMatrix = glm::mat4(1.0f);
        planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(planeYaw), glm::vec3(0, 1, 0));   // yaw
        planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(-planePitch), glm::vec3(1, 0, 0));  // inverted pitch
        planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(-planeRoll), glm::vec3(0, 0, 1));  // inverted roll

        glm::vec3 planeForward = glm::normalize(glm::vec3(planeRotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));

        // Cockpit offset
        glm::vec3 cockpitOffsetLocal = glm::vec3(0.0f, 0.9f, -0.45f);
        if (useThirdPersonCamera) cockpitOffsetLocal = glm::vec3(0.0f, 1.8f, 10.0f);
        glm::vec3 rotatedOffset = glm::vec3(planeRotationMatrix * glm::vec4(cockpitOffsetLocal, 1.0f));
        glm::vec3 cockpitWorldPos = planePosition + rotatedOffset;

        // Camera rotation (plane + mouse look)
        glm::mat4 cameraRotation = planeRotationMatrix;
        cameraRotation = glm::rotate(cameraRotation, glm::radians(cameraYawOffset), glm::vec3(0, 1, 0));
        cameraRotation = glm::rotate(cameraRotation, glm::radians(cameraPitchOffset), glm::vec3(1, 0, 0));

        // Extract camera basis vectors
        glm::vec3 cameraFront = glm::normalize(glm::vec3(cameraRotation * glm::vec4(0, 0, -1, 0)));
        glm::vec3 cameraUp = glm::normalize(glm::vec3(cameraRotation * glm::vec4(0, 1, 0, 0)));

        // --- Update camera ---
        activeCamera.Position = cockpitWorldPos;
        activeCamera.Front = cameraFront;
        activeCamera.Up = cameraUp;



        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(activeCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 8000.0f);
        glm::mat4 view = activeCamera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, shipPosition); 
        model = glm::scale(model, glm::vec3(shipScale, shipScale, shipScale));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
        ourShader.setMat4("model", model);
        shipModel.Draw(ourShader);

        // Draw ship hitbox
        if (showHitboxes) {
            hitboxShader.use();
            hitboxShader.setMat4("projection", projection);
            hitboxShader.setMat4("view", view);
            glm::mat4 hitboxModel = glm::mat4(1.0f);
            hitboxModel = glm::translate(hitboxModel, glm::vec3(0.0f, -10.0f, 0.0f));
            hitboxModel = glm::scale(hitboxModel, glm::vec3(30.0f, 20.0f, 200.0f)); // width, height, length
            hitboxModel = glm::rotate(hitboxModel, glm::radians(90.0f), glm::vec3(0, 1, 0));
            hitboxShader.setMat4("model", hitboxModel);
            hitboxShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
            glBindVertexArray(hitboxVAO);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }


        ourShader.use();
        
        if (bombAttached) {
            // Bomb follows the plane
            glm::vec3 bombOffsetLocal(0.0f, -0.5f, 1.8f);
            glm::mat4 planeRotationMatrix = glm::mat4(1.0f);
            planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(planeYaw), glm::vec3(0, 1, 0));
            planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(-planePitch), glm::vec3(1, 0, 0));
            planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(-planeRoll), glm::vec3(0, 0, 1));

            glm::vec3 rotatedBombOffset = glm::vec3(planeRotationMatrix * glm::vec4(bombOffsetLocal, 1.0f));
            bombPosition = planePosition + rotatedBombOffset;

            bombYaw = planeYaw;
            bombPitch = planePitch;
            bombRoll = planeRoll;

        }
        else if (bombReleased) {
            // Apply gravity and motion
            bombVelocity.y += gravity * deltaTime;
            bombPosition += bombVelocity * deltaTime;
        }
        
        glm::mat4 bombRotationMatrix = glm::mat4(1.0f);
        bombRotationMatrix = glm::rotate(bombRotationMatrix, glm::radians(planeYaw), glm::vec3(0, 1, 0));
        bombRotationMatrix = glm::rotate(bombRotationMatrix, glm::radians(-planePitch), glm::vec3(1, 0, 0));
        bombRotationMatrix = glm::rotate(bombRotationMatrix, glm::radians(-planeRoll), glm::vec3(0, 0, 1));

        model = glm::mat4(1.0f);
        model = glm::translate(model, bombPosition);
        model *= bombRotationMatrix;
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(bombScale));
        ourShader.setMat4("model", model);
        bombModel.Draw(ourShader);

        // Check bomb collision with ship
        if (bombReleased && !bombHit) {
            if (checkSphereBoxCollision(bombPosition, bombHitRadius, shipPosition, shipBoxHalfSize)) {
                bombHit = true;
                hitCount++;
                showExplosion = true;
                explosionPosition = bombPosition;
                std::cout << "Hit Target!" << std::endl;
            }
        }

        // Draw explosion when bomb hits
        if (showExplosion) {
            ourShader.use();
            glm::mat4 explosionModelMat = glm::mat4(1.0f);
            explosionModelMat = glm::translate(explosionModelMat, explosionPosition + glm::vec3(0.0f, -10.0f, 0.0f));
            explosionModelMat = glm::scale(explosionModelMat, glm::vec3(explosionScale));
            ourShader.setMat4("model", explosionModelMat);
            explosionModel.Draw(ourShader);
        }


        // Draw bomb hitbox sphere
        if (showHitboxes) {
            hitboxShader.use();
            hitboxShader.setMat4("projection", projection);
            hitboxShader.setMat4("view", view);
            glm::mat4 bombSphereModel = glm::mat4(1.0f);
            bombSphereModel = glm::translate(bombSphereModel, bombPosition);
            bombSphereModel = glm::scale(bombSphereModel, glm::vec3(bombHitRadius));
            hitboxShader.setMat4("model", bombSphereModel);
            hitboxShader.setVec3("color", bombHit ? glm::vec3(0.0f, 1.0f, 0.0f)
                : glm::vec3(1.0f, 1.0f, 0.0f));
            glBindVertexArray(bombSphereVAO);
            glDrawArrays(GL_LINE_LOOP, 0, bombSphereVertices.size() / 3);
            glBindVertexArray(0);
        }


        ourShader.use();

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, planePosition);
        model = glm::scale(model, glm::vec3(planeScale, planeScale, planeScale));
        model = glm::rotate(model, glm::radians(180.0f + planeYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(planePitch), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(planeRoll), glm::vec3(0.0, 0.0, 1.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        planePosition += planeForward * planeSpeed * deltaTime;

        // draw skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(activeCamera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);


        std::string windowTitle = "LearnOpenGL - Hit: " + std::to_string(hitCount);
        glfwSetWindowTitle(window, windowTitle.c_str());


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Switch cameras based on key press
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        useThirdPersonCamera = false; 
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        useThirdPersonCamera = true; 
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        planePitch -= pitchSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        planePitch += pitchSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        planeTurn(1);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        planeTurn(-1);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        planeModSpeed = true;
        planeSpeed += accelerate * deltaTime;
        planeSpeed = min(planeSpeed, maxSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        planeModSpeed = true;
        planeSpeed -= accelerate * deltaTime;
        planeSpeed = max(planeSpeed, minSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && bombAttached) {
        bombAttached = false;
        bombReleased = true;

        glm::mat4 planeRotationMatrix = glm::mat4(1.0f);
        planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(planeYaw), glm::vec3(0, 1, 0));
        planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(-planePitch), glm::vec3(1, 0, 0));
        planeRotationMatrix = glm::rotate(planeRotationMatrix, glm::radians(-planeRoll), glm::vec3(0, 0, 1));
        glm::vec3 planeForward = glm::normalize(glm::vec3(planeRotationMatrix * glm::vec4(0, 0, -1, 0)));

        bombVelocity = planeForward * planeSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        bombAttached = true;
        bombReleased = false;
        bombVelocity = glm::vec3(0.0f);
        bombHit = false;
    }
    static bool hKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        showHitboxes = !showHitboxes;
        hKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        hKeyPressed = false;
    }

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cameraYawOffset += xoffset;
    cameraPitchOffset += yoffset;

    if (cameraPitchOffset > 89.0f)
        cameraPitchOffset = 89.0f;
    if (cameraPitchOffset < -89.0f)
        cameraPitchOffset = -89.0f;
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    firstPersonCamera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void planeTurn(float direction) {
    planeTurning = true;
    planeYaw += yawSpeed * direction * deltaTime;
    planeRoll -= rollSpeed * direction * deltaTime;
    if (planeRoll > 45.0f) planeRoll = 45.0f;
    if (planeRoll < -45.0f) planeRoll = -45.0f;
}

bool checkSphereBoxCollision(glm::vec3 sphereCenter, float sphereRadius, glm::vec3 boxCenter, glm::vec3 boxHalfSize)
{
    float x = std::max(boxCenter.x - boxHalfSize.x, std::min(sphereCenter.x, boxCenter.x + boxHalfSize.x));
    float y = std::max(boxCenter.y - boxHalfSize.y, std::min(sphereCenter.y, boxCenter.y + boxHalfSize.y));
    float z = std::max(boxCenter.z - boxHalfSize.z, std::min(sphereCenter.z, boxCenter.z + boxHalfSize.z));

    float distance = glm::distance(sphereCenter, glm::vec3(x, y, z));

    return distance < sphereRadius;
}

