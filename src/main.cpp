#undef GLFW_DLL

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <vector>

#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

const glm::vec3 initialCameraPos(100.0f, 90.0f, 135.0f);
const glm::vec3 sceneTarget(0.0f, 12.0f, 7.0f);

float yaw = 0.0f;
float pitch = 0.0f;

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;
std::vector<GLuint> textureList;
std::vector<glm::vec3> materialColors;
std::vector<bool> clipAlphaList;
std::vector<bool> emissiveList;

const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);
const std::filesystem::path textureDirectory =
    std::filesystem::u8path(OPENGL_STARTER_TEXTURE_DIR);
const std::filesystem::path modelDirectory =
    std::filesystem::u8path(OPENGL_STARTER_MODEL_DIR);

bool CreateObj(const std::filesystem::path& modelPath)
{
    Mesh* obj = new Mesh();
    bool loaded = obj->CreateMeshFromOBJ(modelPath.u8string());

    if (!loaded)
    {
        std::cerr << "Failed to load model: " << modelPath << '\n';
        delete obj;
        return false;
    }
    meshList.push_back(obj);
    return true;
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(
        shaderDirectory / "shader.vert",
        shaderDirectory / "shader.frag"
    );
    shaderList.push_back(shader1);
    /*
    Shader* shader2 = new Shader();
    shader2->CreateFromFiles(
        shaderDirectory / "lightShader.vert",
        shaderDirectory / "lightShader.frag"
    );
    shaderList.push_back(shader2);
    */
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
        std::cerr << "Failed to load texture: " << stbi_failure_reason() << "\n";
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

// ธูปเทียนไม่มี texture มีแค่สี material
GLuint CreatePlainTexture()
{
    const unsigned char white[] = {255, 255, 255, 255};

    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

// Load an OBJ together with its image and color, keeping all lists aligned.
// An empty texture filename uses a white texture tinted by the material color.
bool AddScenePart(const char* objFile,
                  const char* textureFile = "",
                  const glm::vec3& color = glm::vec3(1.0f),
                  bool clipAlpha = false,
                  bool emissive = false)
{
    GLuint texture = 0;
    if (textureFile[0] != '\0')
    {
        texture = LoadtexTutre(textureDirectory / textureFile);
    }
    else
    {
        texture = CreatePlainTexture();
    }
    if (!texture)
        return false;
    if (!CreateObj(modelDirectory / objFile))
    {
        glDeleteTextures(1, &texture);
        return false;
    }
    textureList.push_back(texture);
    materialColors.push_back(color);
    clipAlphaList.push_back(clipAlpha);
    emissiveList.push_back(emissive);

    return true;
}

bool LoadPlatform()
{
    return AddScenePart(
        "platform__platform_natural_wood.obj",
        "01_natural_wooden_background.png"
    );
}

bool LoadCoffin()
{
    return AddScenePart(
        "coffin__material_0.obj",
        "12_image_0.png"
    );
}

bool LoadCats()
{
    // American
    if (!AddScenePart(
        "cat_american__material.obj",
        "15_base_color.png"
    )) return false;

    // Black: แยกเป็นสาม material
    if (!AddScenePart(
        "cat_black__material_105.obj",
        "23_material_105_basecolor.png"
    )) return false;

    if (!AddScenePart(
        "cat_black__material_81.obj",
        "22_material_81_basecolor.png"
    )) return false;

    if (!AddScenePart(
        "cat_black__material_93.obj",
        "24_material_93_basecolor.png",
        glm::vec3(1.0f),
        true // ตัดบริเวณโปร่งใสของภาพ
    )) return false;

    // Fred
    if (!AddScenePart(
        "cat_fred__material_0.obj",
        "19_image_1.png"
    )) return false;

    return true;
}

bool LoadTable()
{
    if (!AddScenePart(
        "table__gold_marble.obj",
        "05_marble_07_color.png"
    )) return false;

    if (!AddScenePart(
        "table__tauari_wood.obj",
        "09_mdf_bp_tauari_guararapes_imagem_01_transformed.png"
    )) return false;

    if (!AddScenePart(
        "table__unassigned.obj",
        "",
        glm::vec3(1.0f)
    )) return false;

    return true;
}

bool LoadIncense()
{
    // กระถางธูป
    if (!AddScenePart(
        "incense__material_002.obj",
        "",
        glm::vec3(0.8f)
    )) return false;

    // ก้านธูป
    if (!AddScenePart(
        "incense__stick.obj",
        "",
        glm::vec3(0.8f)
    )) return false;

    return true;
}

bool LoadCandles()
{
    const glm::vec3 holderColor(0.382486f, 0.199769f, 0.121162f);
    const glm::vec3 plainColor(0.8f);
    //ตั้ง mesh เปลวเทียนให้เป็นสีส้ม
    const glm::vec3 flameColor(1.0f, 0.38f, 0.06f);

    // เทียนฝั่งซ้าย
    if (!AddScenePart(
        "candle_left__material_003.obj", "", holderColor
    )) return false;
    if (!AddScenePart(
        "candle_left__material_004.obj", "", plainColor
    )) return false;
    if (!AddScenePart(
        "candle_left__material_005.obj", "", plainColor
    )) return false;

    // เปลวไฟใช้สีส้มและไม่รับแสงจาก shader
    if (!AddScenePart(
        "candle_left__material_007.obj", "", flameColor, false, true
    )) return false;

    // เทียนฝั่งขวา
    if (!AddScenePart(
        "candle_right__material_003.obj", "", holderColor
    )) return false;
    if (!AddScenePart(
        "candle_right__material_004.obj", "", plainColor
    )) return false;
    if (!AddScenePart(
        "candle_right__material_005.obj", "", plainColor
    )) return false;

    if (!AddScenePart(
        "candle_right__material_007.obj", "", flameColor, false, true
    )) return false;

    return true;
}

bool CreateScene()
{
    if (!LoadPlatform())
        return false;
    if (!LoadCoffin())
        return false;
    if (!LoadCats())
        return false;
    if (!LoadTable())
        return false;
    if (!LoadIncense())
        return false;
    if (!LoadCandles())
        return false;

    std::cout << "Loaded " << meshList.size() << " scene parts and their textures.\n";

    return true;
}

void UpdateMouseLook(Window& window)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

    static double lastX = mouseX;
    static double lastY = mouseY;

    float xOffset = static_cast<float>(mouseX - lastX);
    float yOffset = static_cast<float>(mouseY - lastY);
    float sensitivity = 0.1f;

    xOffset = xOffset * sensitivity;
    yOffset = yOffset * sensitivity;
    lastX = mouseX;
    lastY = mouseY;

    yaw += xOffset;
    pitch -= yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

void Cleanup()
{
    for (Mesh* mesh : meshList)
        delete mesh;
    meshList.clear();

    for (Shader* shader : shaderList)
        delete shader;
    shaderList.clear();

    for (GLuint texture : textureList)
        glDeleteTextures(1, &texture);
    textureList.clear();

    materialColors.clear();
    clipAlphaList.clear();
    emissiveList.clear();
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

    GLint uniformModel = 0, uniformView = 0, uniformProjection = 0, uniformTextureData = 0;

    glm::vec3 lightPos = glm::vec3(20.0f, 100.0f, 45.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    //แสงเทียน
    const glm::vec3 candleLightPos[2] = {
        glm::vec3(8.1f, 12.2f, 48.1f),
        glm::vec3(-5.3f, 12.2f, 48.1f)
    };
    const glm::vec3 candleLightColor(1.0f, 0.55f, 0.2f);
    glm::vec3 cameraPos = initialCameraPos;
    glm::vec3 cameraTarget = sceneTarget;
    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

    yaw = glm::degrees(std::atan2(cameraDirection.z, cameraDirection.x));
    pitch = glm::degrees(std::asin(cameraDirection.y));
    float lastTime = static_cast<float>(glfwGetTime());

    while (!mainWindow.getShouldClose())
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        UpdateMouseLook(mainWindow);
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mainWindow.getWindow(), GLFW_TRUE);

        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection = glm::normalize(cameraDirection);
        cameraRight = glm::normalize(glm::cross(cameraDirection, up));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

        float moveSpeed = 20.0f;
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
            yaw = -90.0f;
            pitch = 0.0f;
            cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
            cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
            cameraRight = glm::normalize(glm::cross(cameraDirection, up));
            cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));
        }

        int bufferWidth = 0;
        int bufferHeight = 0;
        glfwGetFramebufferSize(mainWindow.getWindow(), &bufferWidth, &bufferHeight);
        if (bufferWidth <= 0 || bufferHeight <= 0)
            continue;

        glm::mat4 projection = glm::perspective(
            glm::radians(30.0f),
            static_cast<float>(bufferWidth) / static_cast<float>(bufferHeight),
            0.1f,
            600.0f
        );
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);

        glClearColor(0.16f, 0.17f, 0.19f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw every OBJ once, binding the matching texture before each draw.
        for (size_t i = 0; i < meshList.size(); i++)
        {
            shaderList[0]->UseShader();
            uniformModel = shaderList[0]->GetUniformLocation("model");
            uniformView = shaderList[0]->GetUniformLocation("view");
            uniformProjection = shaderList[0]->GetUniformLocation("projection");
            uniformTextureData = shaderList[0]->GetUniformLocation("texture_data");

            glUniform1i(uniformTextureData, 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureList[i]);

            glUniform3fv(shaderList[0]->GetUniformLocation("lightColor"), 1, glm::value_ptr(lightColor));
            glUniform3fv(shaderList[0]->GetUniformLocation("lightPos"), 1, glm::value_ptr(lightPos));
            glUniform3fv(shaderList[0]->GetUniformLocation("candleLightPos[0]"), 1, glm::value_ptr(candleLightPos[0]));
            glUniform3fv(shaderList[0]->GetUniformLocation("candleLightPos[1]"), 1, glm::value_ptr(candleLightPos[1]));
            glUniform3fv(shaderList[0]->GetUniformLocation("candleLightColor"), 1, glm::value_ptr(candleLightColor));
            glUniform3fv(shaderList[0]->GetUniformLocation("viewPos"), 1, glm::value_ptr(cameraPos));
            glUniform3fv(shaderList[0]->GetUniformLocation("objectColor"), 1, glm::value_ptr(materialColors[i]));
            glUniform1i(shaderList[0]->GetUniformLocation("clipAlpha"), clipAlphaList[i]);
            glUniform1i(shaderList[0]->GetUniformLocation("emissive"), emissiveList[i]);

            glm::mat4 model(1.0f);

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

            meshList[i]->RenderMesh();

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        /* lightShader
        shaderList[1]->UseShader();
        glm::mat4 model(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        glUniformMatrix4fv(shaderList[1]->GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(shaderList[1]->GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(shaderList[1]->GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(shaderList[1]->GetUniformLocation("lightColor"), 1, glm::value_ptr(lightColor));
        */

        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    Cleanup();
    return 0;
}
