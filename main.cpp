#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <Camera.h>
#include <FrameBufferHandler.h>

#include <iostream>
#include <vector>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
const unsigned int NUM_PATCH_PTS = 4;
int useWireframe = 0;
int displayGrayscale = 0;

// camera settings - start from good spot
Camera camera(glm::vec3(67.0f, 627.5f, 169.9f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -128.1f, -42.4f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// setting the height of the water
float waterHeight = 10.0f;
float waveSpeed = 0.03f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain GPU", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGl function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shader programs
    // ---------------------------------
    Shader heightMapShader("TessellationGPU_Vert.txt", "TessellationGPU_Frag.txt",
                            "TessellationGPU_TCS.txt", "TessellationGPU_TES.txt");

    Shader waterShader("WaterShader_Vert.txt", "WaterShader_Frag.txt");

    Shader debugShader("Debug_Vert.txt", "Debug_Frag.txt");

    // load and create a texture for height map
    // ----------------------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    //stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("iceland_heightmap.png", &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        heightMapShader.use();
        heightMapShader.setInt("heightMap", 0);
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    
    stbi_image_free(data);

    // load and create a texture for the terrain - level 0
    // ---------------------------------------------------
    unsigned int terrainTexture0;
    glGenTextures(1, &terrainTexture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, terrainTexture0);

    // Set the texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int colorWidth, colorHeight, colorChannels;
    data = stbi_load("dirt1.png", &colorWidth, &colorHeight, &colorChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, colorWidth, colorHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        heightMapShader.use();
        heightMapShader.setInt("textureHeight0", 1);
        //std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    // create and load texture for terrain - level 1
    // ---------------------------------------------
    unsigned int terrainTexture1;
    glGenTextures(1, &terrainTexture1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, terrainTexture1);

    // Set the texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //int colorWidth, colorHeight, colorChannels;
    data = stbi_load("dirt4.png", &colorWidth, &colorHeight, &colorChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, colorWidth, colorHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        heightMapShader.use();
        heightMapShader.setInt("textureHeight1", 2);
        //std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    // create and load texture for terrain - level 2
    // ---------------------------------------------
    unsigned int terrainTexture2;
    glGenTextures(1, &terrainTexture2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, terrainTexture2);

    // Set the texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //int colorWidth, colorHeight, colorChannels;
    data = stbi_load("grass_mossy.png", &colorWidth, &colorHeight, &colorChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, colorWidth, colorHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        heightMapShader.use();
        heightMapShader.setInt("textureHeight2", 3);
        //std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    // create and load texture for terrain - level 3
    // ---------------------------------------------
    unsigned int terrainTexture3;
    glGenTextures(1, &terrainTexture3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, terrainTexture3);

    // Set the texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //int colorWidth, colorHeight, colorChannels;
    data = stbi_load("snow01.png", &colorWidth, &colorHeight, &colorChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, colorWidth, colorHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        heightMapShader.use();
        heightMapShader.setInt("textureHeight3", 4);
        //std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    //create and load texture - dudv map
    //----------------------------------
    unsigned int dudvTexture;
    glGenTextures(1, &dudvTexture);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, dudvTexture);

    // Set the texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //int colorWidth, colorHeight, colorChannels;
    data = stbi_load("waterDUDV.png", &colorWidth, &colorHeight, &colorChannels, 3);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colorWidth, colorHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);

        waterShader.use();
        waterShader.setInt("dudvMap", 5);
        //std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }

    stbi_image_free(data);

    // set up vertex data (and buffers) and configure vertex attributes
    // ----------------------------------------------------------------
    std::vector<float> vertices;
    unsigned int rez = 20;
       
    for (int i = 0; i <= rez - 1; ++i)
    {
        for (int j = 0; j <= rez - 1; ++j)
        {
            vertices.push_back(-width / 2.0f + width * i / (float)rez);     // v.x
            vertices.push_back(0.0f);                                       // v.y
            vertices.push_back(-height / 2.0f + height * j / (float)rez);   // v.z
            vertices.push_back(i / (float)rez);                             // u
            vertices.push_back(j / (float)rez);                             // v

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back(j / (float)rez); // v

            vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
            vertices.push_back(i / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
            vertices.push_back((i + 1) / (float)rez); // u
            vertices.push_back((j + 1) / (float)rez); // v
        }
    }

    std::cout << "Loaded " << rez * rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez * rez * 4 << " vertices in vertex shader" << std::endl;

    // VAO configuration
    unsigned int terrainVAO, terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // texCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    // set up vertex data and buffers for the water surface
    // ----------------------------------------------------
    float waterVertices[] =
    {
        // positions                                // texture coords
        -width / 2.0f, waterHeight, -height / 2.0f, 0.0f, 0.0f,     // bottom left
         width / 2.0f, waterHeight, -height / 2.0f, 1.0f, 0.0f,     // bottom right
         width / 2.0f, waterHeight,  height / 2.0f, 1.0f, 1.0f,     // top right
        -width / 2.0f, waterHeight,  height / 2.0f, 0.0f, 1.0f,     // top left
    };

    // water indices
    unsigned int waterIndices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    // VAO configuration
    unsigned int waterVAO, waterVBO, waterEBO;

    glGenVertexArrays(1, &waterVAO);
    glBindVertexArray(waterVAO);

    glGenBuffers(1, &waterVBO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &waterEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(waterIndices), waterIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // unbind water VAO
    glBindVertexArray(0);

    // sending data to water fragment shader
    // -------------------------------------
    waterShader.use();
    waterShader.setFloat("width", width);
    waterShader.setFloat("height", height);

    // setting up the debug quad data
    // ------------------------------
    float quadVertices[] = {
        // Positions   // Texture Coords
        -1.0f, -1.0f,   0.0f, 0.0f, // Bottom-left
         1.0f, -1.0f,   1.0f, 0.0f, // Bottom-right
        -1.0f,  1.0f,   0.0f, 1.0f, // Top-left
         1.0f,  1.0f,   1.0f, 1.0f  // Top-right
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        1, 2, 3
    };

    unsigned int quadVAO, quadVBO, quadEBO;

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // set up the FBO handler
    // ----------------------
    FrameBufferHandler fbHandler(6);    // start from texture slot 5

    // declaring the clipping planes
    glm::vec4 reflectionClippingPlane = glm::vec4(0.0f, 1.0f, 0.0f, -waterHeight);
    glm::vec4 refractionClippingPlane = glm::vec4(0.0f, -1.0f, 0.0f, waterHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // the move factor for the waves
    float moveFactor = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //std::cout << deltaTime << std::endl;

        // input
        // -----
        processInput(window);

        // Toggle wireframe mode
        if (useWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // render Reflection
        // -----------------
        glEnable(GL_CLIP_DISTANCE0);
        fbHandler.bindReflectionFrameBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        heightMapShader.use();
        heightMapShader.setVec4("clippingPlane", reflectionClippingPlane);

        // moving the camera 
        glm::vec3 originalCameraPosition = camera.Position;
        float originalCameraPitch = camera.Pitch;
        
        camera.Position.y = 2 * waterHeight - camera.Position.y;
        camera.Pitch = -camera.Pitch;
        camera.updateCameraVectors();
        glm::mat4 view = camera.GetViewMatrix();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        heightMapShader.setMat4("projection", projection);
        heightMapShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        heightMapShader.setMat4("model", model);

        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS* rez* rez);

        fbHandler.unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);

        // render Refraction
        // -----------------
        fbHandler.bindRefractionFrameBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        heightMapShader.use();
        heightMapShader.setVec4("clippingPlane", refractionClippingPlane);

        // moving the camera
        camera.Pitch = originalCameraPitch;
        camera.Position = originalCameraPosition;
        camera.updateCameraVectors();

        // view/projection transformations
        view = camera.GetViewMatrix();
        heightMapShader.setMat4("projection", projection);
        heightMapShader.setMat4("view", view);

        // world transformation
        heightMapShader.setMat4("model", model);

        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS* rez* rez);

        fbHandler.unbindCurrentFrameBuffer(SCR_WIDTH, SCR_HEIGHT);

        // render scene normally
        // ---------------------
        glDisable(GL_CLIP_DISTANCE0);
        //glClearColor(0.529, 0.808, 0.922, 1.0);
        glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader
        heightMapShader.use();

        // view/projection transformations
        //view = camera.GetViewMatrix();
        heightMapShader.setVec4("clippingPlane", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
        heightMapShader.setMat4("projection", projection);
        heightMapShader.setMat4("view", view);

        // world transformation
        heightMapShader.setMat4("model", model);

        // render the terrain
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS * rez * rez);

        // render water surface
        // --------------------
        waterShader.use();

        // binding reflection and refraction textures
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, fbHandler.getReflectionTexture());
        waterShader.setInt("reflectionTexture", 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, fbHandler.getRefractionTexture());
        waterShader.setInt("refractionTexture", 7);

        // setting the matrices
        waterShader.setMat4("projection", projection);
        waterShader.setMat4("view", view);
        waterShader.setMat4("model", model);

        // setting the move factor
        moveFactor += waveSpeed * deltaTime;
        moveFactor = fmod(moveFactor, 1);
        waterShader.setFloat("moveFactor", moveFactor);

        // setting the camera position
        camera.updateCameraVectors();
        waterShader.setVec3("cameraPosition", camera.Position);

        // render water surface
        glBindVertexArray(waterVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //// render debug textures
        //// ---------------------
        //debugShader.use();

        //// Render reflection texture in top-left corner
        //glViewport(0, SCR_HEIGHT * 3 / 4, SCR_WIDTH / 4, SCR_HEIGHT / 4);
        //glActiveTexture(GL_TEXTURE8); // Slot for debug reflection
        //glBindTexture(GL_TEXTURE_2D, fbHandler.getReflectionTexture());
        //debugShader.setInt("screenTexture", 8); // Set the sampler to texture slot 9
        //glBindVertexArray(quadVAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //// Render refraction texture in top-right corner
        //glViewport(SCR_WIDTH * 3 / 4, SCR_HEIGHT * 3 / 4, SCR_WIDTH / 4, SCR_HEIGHT / 4);
        //glActiveTexture(GL_TEXTURE9); // Slot for debug refraction
        //glBindTexture(GL_TEXTURE_2D, fbHandler.getRefractionTexture());
        //debugShader.setInt("screenTexture", 9); // Set the sampler to texture slot 10
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Reset the viewport for the main window
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        // glfw: swap buffers and poll IO events
        // -------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all resources once we're done
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        //std::cout << "Moving Forward" << std::endl;
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (OS or user resize) this callback function executes
// ------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever a key event occurs, this callback is called
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_SPACE:
            useWireframe = 1 - useWireframe;
            break;
        case GLFW_KEY_G:
            displayGrayscale = 1 - displayGrayscale;
            break;
        default:
            break;
        }
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
