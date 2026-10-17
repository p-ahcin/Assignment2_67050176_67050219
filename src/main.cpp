#undef GLFW_DLL

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8

#include <stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

float yaw = -90.0f;
float pitch = 0.0f;

std::vector<Mesh *> meshList;
std::vector<Shader *> shaderList;
std::vector<GLuint> textureList;
std::vector<glm::vec3> materialColors;
std::vector<bool> alphaClipList;
std::map<std::filesystem::path, GLuint> textureCache;

Shader* depthShader;

const int CANDLE_LIGHT_COUNT = 4;
// OpenGL coordinates: Blender (x, y, z) becomes (x, z, -y).
const glm::vec3 candlePositions[CANDLE_LIGHT_COUNT] = {
    glm::vec3(8.111f, 11.24f, 47.729f),   // candle1 on the front table
    glm::vec3(-5.276f, 11.24f, 47.729f),  // candle2 on the front table
    glm::vec3(46.415f, 20.15f, 49.196f), // first altar candle
    glm::vec3(46.415f, 20.15f, 52.902f)  // second altar candle
};
const glm::vec3 candleColor(1.0f, 0.65f, 0.32f);
const float candleStrengths[CANDLE_LIGHT_COUNT] = {7.0f, 7.0f, 5.5f, 5.5f};
const float ambientStrength = 0.45f;
const float attenuationLinear = 0.07f;
const float attenuationQuadratic = 0.017f;
const float shadowFarPlane = 180.0f;
const unsigned int SHADOW_SIZE = 1024;
GLuint depthMaps[CANDLE_LIGHT_COUNT] = {};
GLuint depthMapFBO = 0;
GLuint uniformModel = 0, uniformView = 0, uniformProjection = 0, uniformTextureData = 0;
const glm::vec3 initialCameraPos(-5.28f, 110.0f, 330.0f);
glm::vec3 cameraPos = initialCameraPos;


const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);
const std::filesystem::path textureDirectory =
    std::filesystem::u8path(OPENGL_STARTER_TEXTURE_DIR);
const std::filesystem::path modelDirectory =
    std::filesystem::u8path(OPENGL_STARTER_MODEL_DIR);

void CreateTriangle()
{
    GLfloat vertices[] =
        {
            -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
            0.0, -1.0f, 1.0f,       0.5f, 0.0f,
            1.0f, -1.0f, 0.0f,      1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,       0.5f, 1.0f
        };

    unsigned int indices[] =
        {
            0, 3, 1,
            1, 3, 2,
            2, 3, 0,
            0, 1, 2
        };

    Mesh *obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 20, 12);
    meshList.push_back(obj1);
}

bool CreateObj(const std::filesystem::path& modelPath, bool flipV = false)
{
    Mesh *obj = new Mesh();
    bool loaded = obj->CreateMeshFromOBJ(modelPath.u8string().c_str(), flipV);

    if (!loaded)
    {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        delete obj;
        return false;
    }

    meshList.push_back(obj);
    return true;
}

void CreateShaders()
{
    Shader *shader1 = new Shader();
    shader1->CreateFromFiles(
        shaderDirectory / "shader.vert",
        shaderDirectory / "shader.frag"
    );
    shaderList.push_back(shader1);

    depthShader = new Shader();
    depthShader->CreateFromFiles(
        shaderDirectory / "depthShader.vert",
        shaderDirectory / "depthShader.frag"
    );
}

GLuint LoadtexTutre(const std::filesystem::path& texturePath)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load(texturePath.u8string().c_str(), 
                                    &width, &height, & channels, 0);

    if (data == nullptr)
    {
        std::cerr << "Failed to load texture: " << texturePath
                  << " (" << stbi_failure_reason() << ")\n";
        return 0;
    }

    GLenum format = GL_RGB;

    switch (channels)
    {
    case 4:format = GL_RGBA;
        break;
    case 3:format = GL_RGB;
        break;
    case 2:format = GL_RG;
        break;
    case 1:format = GL_RED;
        break;

    default:
        std::cerr << "Unsupported texture format: " << stbi_failure_reason() << "\n";
        stbi_image_free(data);
        return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
    
}

bool LoadScenePart(const std::filesystem::path& modelPath)
{
    // These exports contain one material:OBJ. Read only the MTL values
    std::filesystem::path materialPath = modelPath;
    materialPath.replace_extension(".mtl");
    std::ifstream materialFile(materialPath);
    if (!materialFile)
    {
        std::cerr << "Failed to open material: " << materialPath << '\n';
        return false;
    }

    glm::vec3 color(1.0f);
    std::filesystem::path texturePath = textureDirectory / "platform/plain_white.png";
    std::filesystem::path alphaPath;
    int materialCount = 0;
    std::string line;
    while (std::getline(materialFile, line))
    {
        std::istringstream values(line);
        std::string tag;
        values >> tag;
        if (tag == "newmtl")
            ++materialCount;
        else if (tag == "Kd")
            values >> color.r >> color.g >> color.b;
        else if (tag == "map_Kd" || tag == "map_d")
        {
            std::string imageName;
            std::getline(values >> std::ws, imageName);
            if (imageName.empty() || imageName.front() == '-')
            {
                std::cerr << "Unsupported texture entry in: " << materialPath << '\n';
                return false;
            }
            auto imagePath = (materialPath.parent_path() /
                              std::filesystem::u8path(imageName)).lexically_normal();
            if (tag == "map_Kd")
                texturePath = imagePath;
            else
                alphaPath = imagePath;
        }
    }
    if (materialCount != 1 || (!alphaPath.empty() && alphaPath != texturePath))
    {
        std::cerr << "Expected one material and a shared color/alpha image: "
                  << materialPath << '\n';
        return false;
    }

    texturePath = texturePath.lexically_normal();
    GLuint texture = 0;
    auto cached = textureCache.find(texturePath);
    if (cached != textureCache.end())
        texture = cached->second;
    else
    {
        texture = LoadtexTutre(texturePath);
        if (texture == 0)
            return false;
        textureCache.emplace(texturePath, texture);
    }

    // Cat OBJ UVs use Blender's bottom-left image origin; stb reads top-first.
    const std::string group = modelPath.parent_path().filename().u8string();
    const bool flipV = group == "cat_american" || group == "cat_black" || group == "cat_fred";
    if (!CreateObj(modelPath, flipV))
        return false;
    textureList.push_back(texture);
    materialColors.push_back(color);
    alphaClipList.push_back(!alphaPath.empty());
    return true;
}

bool LoadSceneGroup(const std::string& groupName)
{
    const auto directory = modelDirectory / groupName;
    if (!std::filesystem::is_directory(directory))
    {
        std::cerr << "Model folder not found: " << directory << '\n';
        return false;
    }

    std::vector<std::filesystem::path> modelFiles;
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".obj")
            modelFiles.push_back(entry.path());
    }
    std::sort(modelFiles.begin(), modelFiles.end());
    if (modelFiles.empty())
    {
        std::cerr << "No OBJ files in: " << directory << '\n';
        return false;
    }
    for (const auto& modelFile : modelFiles)
    {
        if (!LoadScenePart(modelFile))
            return false;
    }
    std::cout << "Loaded " << groupName << ": " << modelFiles.size() << " parts" << std::endl;
    return true;
}

bool CreateScene()
{
    if (!LoadSceneGroup("platform"))
        return false;

    const char* sceneGroups[] = {
        "coffin2", "Flower1", "Flower2", "wreath", "altar_table",
        "monk_pltform", "table", "incense", "candle1", "candle2",
        "cat_american", "cat_black", "cat_fred"
    };
    for (const char* group : sceneGroups)
    {
        if (!LoadSceneGroup(group))
            return false;
    }
    std::cout << "Scene ready: " << meshList.size() << " parts, "
              << textureCache.size() << " unique textures" << std::endl;
    return true;
}

void UpdateMouseLook(Window &window, bool resetMouse = false)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

    static double lastX = mouseX;
    static double lastY = mouseY;

    if (resetMouse)
    {
        lastX = mouseX;
        lastY = mouseY;
        return;
    }

    float xOffset = static_cast<float> (mouseX - lastX);
    float yOffset = static_cast<float> (mouseY - lastY);

    float sensitivity = 0.1f;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    lastX = mouseX;
    lastY = mouseY;

    yaw += xOffset;
    pitch -= yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

void RenderScene(glm::mat4 view, glm::mat4 projection, bool depthPass = false)
{
    glm::mat4 model(1.0f);
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

    Shader* activeShader = depthPass ? depthShader : shaderList[0];
    const GLint uniformColor = activeShader->GetUniformLocation("materialColor");
    const GLint uniformClip = activeShader->GetUniformLocation("clipAlpha");
    glUniform1i(activeShader->GetUniformLocation("texture_data"), 0);
    glActiveTexture(GL_TEXTURE0);

    // The platform and every other part share the same candle-lighting formula.
    for (size_t i = 0; i < meshList.size(); ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textureList[i]);
        glUniform1i(uniformClip, alphaClipList[i]);
        if (!depthPass)
            glUniform3fv(uniformColor, 1, glm::value_ptr(materialColors[i]));
        meshList[i]->RenderMesh();
    }
}

bool CreateCandleShadows()
{
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(CANDLE_LIGHT_COUNT, depthMaps);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);

    const glm::vec3 directions[] = {
        {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}
    };
    const glm::vec3 upVectors[] = {
        {0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}
    };
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f,
                                                0.1f, shadowFarPlane);
    depthShader->UseShader();
    uniformModel = depthShader->GetUniformLocation("model");
    uniformView = depthShader->GetUniformLocation("view");
    uniformProjection = depthShader->GetUniformLocation("projection");
    glUniform1f(depthShader->GetUniformLocation("farPlane"), shadowFarPlane);

    for (int light = 0; light < CANDLE_LIGHT_COUNT; ++light)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthMaps[light]);
        for (int face = 0; face < 6; ++face)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT24,
                         SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glUniform3fv(depthShader->GetUniformLocation("pointLightPos"),
                     1, glm::value_ptr(candlePositions[light]));

        for (int face = 0; face < 6; ++face)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthMaps[light], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cerr << "Candle shadow framebuffer is incomplete\n";
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return false;
            }
            glClear(GL_DEPTH_BUFFER_BIT);
            glm::mat4 lightView = glm::lookAt(candlePositions[light],
                                             candlePositions[light] + directions[face],
                                             upVectors[face]);
            RenderScene(lightView, lightProjection, true);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    std::cout << "Candle shadows ready: 4 lights, 6 views per light" << std::endl;
    return true;
}

void SetCandleLights(Shader* shader)
{
    glUniform3fv(shader->GetUniformLocation("lightColor"), 1, glm::value_ptr(candleColor));
    glUniform1f(shader->GetUniformLocation("ambientStrength"), ambientStrength);
    glUniform1f(shader->GetUniformLocation("attenuationLinear"), attenuationLinear);
    glUniform1f(shader->GetUniformLocation("attenuationQuadratic"), attenuationQuadratic);
    glUniform1f(shader->GetUniformLocation("farPlane"), shadowFarPlane);
    for (int i = 0; i < CANDLE_LIGHT_COUNT; ++i)
    {
        const std::string index = "[" + std::to_string(i) + "]";
        glUniform3fv(shader->GetUniformLocation(("lightPositions" + index).c_str()),
                     1, glm::value_ptr(candlePositions[i]));
        glUniform1f(shader->GetUniformLocation(("lightStrengths" + index).c_str()),
                    candleStrengths[i]);
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthMaps[i]);
        glUniform1i(shader->GetUniformLocation(("shadowMaps" + index).c_str()), 1 + i);
    }
    glActiveTexture(GL_TEXTURE0);
}

void Cleanup()
{
    for (Mesh *mesh : meshList)
        delete mesh;
    meshList.clear();
    textureList.clear();
    materialColors.clear();
    alphaClipList.clear();
    glDeleteTextures(CANDLE_LIGHT_COUNT, depthMaps);
    std::fill(depthMaps, depthMaps + CANDLE_LIGHT_COUNT, 0);
    glDeleteFramebuffers(1, &depthMapFBO);
    depthMapFBO = 0;

    for (const auto& texture : textureCache)
        glDeleteTextures(1, &texture.second);
    textureCache.clear();

    for (Shader *shader : shaderList)
        delete shader;
    shaderList.clear();

    delete depthShader;
    depthShader = nullptr;
}

int main()
{
    Window mainWindow(WIDTH, HEIGHT, 3, 3);
    if (mainWindow.initialise() != 0)
        return 1;

    CreateShaders();
    if (!CreateScene())
    {
        Cleanup();
        return 1;
    }

    const glm::vec3 cameraTarget(-5.28f, 16.0f, 71.90f);
    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);

    yaw = glm::degrees(atan2(cameraDirection.z, cameraDirection.x));
    pitch = glm::degrees(asin(cameraDirection.y));

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

    float lastTime = static_cast<float> (glfwGetTime());

    if (!CreateCandleShadows())
    {
        Cleanup();
        return 1;
    }
    lastTime = static_cast<float>(glfwGetTime());
    bool resetMouse = true;

    while (!mainWindow.getShouldClose())
    {
        glfwPollEvents();
        if (mainWindow.getShouldClose())
            break;

        // Keep processing restore/close events without rendering a minimized window.
        const int bufferWidth = mainWindow.getBufferWidth();
        const int bufferHeight = mainWindow.getBufferHeight();
        if (glfwGetWindowAttrib(mainWindow.getWindow(), GLFW_ICONIFIED) ||
            bufferWidth <= 0 || bufferHeight <= 0)
        {
            resetMouse = true;
            glfwWaitEventsTimeout(0.1);
            lastTime = static_cast<float>(glfwGetTime());
            continue;
        }

        float currentTime = static_cast<float> (glfwGetTime());
        float deltaTime = resetMouse ? 0.0f : currentTime - lastTime;
        lastTime = currentTime;

        if (glfwGetWindowAttrib(mainWindow.getWindow(), GLFW_FOCUSED))
        {
            UpdateMouseLook(mainWindow, resetMouse);
            resetMouse = false;
        }
        else
            resetMouse = true;

        glm::mat4 projection = glm::perspective(
            glm::radians(25.0f),
            static_cast<float>(bufferWidth) / static_cast<float>(bufferHeight),
            0.1f,
            600.0f
        );

        float moveSpeed = 50.0f;
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos = cameraPos + cameraDirection * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos = cameraPos - cameraDirection * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos = cameraPos - cameraRight * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos = cameraPos + cameraRight * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            cameraPos = initialCameraPos;
            cameraDirection = glm::normalize(cameraTarget - cameraPos);
            yaw = glm::degrees(atan2(cameraDirection.z, cameraDirection.x));
            pitch = glm::degrees(asin(cameraDirection.y));
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view(1.0f);
        glm::mat4 cameraRotateMat (1.0f);
        glm::mat4 cameraPosMat (1.0f);

        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraRight = glm::normalize(glm::cross(cameraDirection, up));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

        view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);

        // Color pass: the floor and objects now receive the same four candle lights.
        glViewport(0, 0, bufferWidth, bufferHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        shaderList[0]->UseShader();
        uniformModel = shaderList[0]->GetUniformLocation("model");
        uniformView = shaderList[0]->GetUniformLocation("view");
        uniformProjection = shaderList[0]->GetUniformLocation("projection");
        uniformTextureData = shaderList[0]->GetUniformLocation("texture_data");
        glUniform1i(uniformTextureData, 0);
        glActiveTexture(GL_TEXTURE0);

        SetCandleLights(shaderList[0]);
        glUniform3fv(shaderList[0]->GetUniformLocation("viewPos"),
                     1, glm::value_ptr(cameraPos));

        RenderScene(view, projection);
        glBindTexture(GL_TEXTURE_2D, 0);

        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    Cleanup();
    return 0;
}
