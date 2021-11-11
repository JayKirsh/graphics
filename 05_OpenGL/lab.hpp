#pragma once

#include "paths.hpp"

#include <exception>
#include <iostream>
#include <string>

#include <atlas/glx/Buffer.hpp>
#include <atlas/glx/Context.hpp>
#include <atlas/glx/ErrorCallback.hpp>
#include <atlas/glx/GLSL.hpp>
#include <atlas/utils/Cameras.hpp>
#include <atlas/utils/LoadObjFile.hpp>

#include <fmt/printf.h>
#include <magic_enum.hpp>

#define ENTRIES_PER_VERTEX 9
#define VERTICES_PER_TRIANGLE 3
#define TRIANGLES 12

using namespace atlas;

static constexpr float nearVal{1.0f};
static constexpr float farVal{10000000000.0f};

static const std::vector<std::string> IncludeDir{ShaderPath};

struct OpenGLError : std::runtime_error
{
    OpenGLError(const std::string& what_arg) : std::runtime_error(what_arg){};
    OpenGLError(const char* what_arg) : std::runtime_error(what_arg){};
};

struct Light {
	Light();
	glm::vec3 direction;
	glm::vec3 position;
	glm::vec3 intensities;
};

class Triangle
{
public:
    Triangle();

    void loadShaders();

    void loadDataToGPU(std::array<float, ENTRIES_PER_VERTEX * VERTICES_PER_TRIANGLE * TRIANGLES> const& vertices);

    void reloadShaders();

	void render(bool paused, bool forward, bool backward, bool left, bool right, int width, int height);

    void freeGPUData();

private:
    void setupUniformVariables();

    float position;

    // Vertex buffers.
    GLuint mVao;
    GLuint mVbo;

    // Shader data.
    GLuint mVertHandle;
    GLuint mFragHandle;
    GLuint mProgramHandle;
    glx::ShaderFile vertexSource;
    glx::ShaderFile fragmentSource;

    // Uniform variable data.
    GLuint mUniformModelLoc;
    GLuint mUniformViewLoc;
    GLuint mUniformProjectionLoc;
	GLuint mUniformLightPositionLoc;
	GLuint mUniformLightIntensityLoc;
};

class Program
{
public:
    Program(int width, int height, std::string title);

    void run(Triangle& object);

    void freeGPUData();

private:
    static void errorCallback(int code, char const* message)
    {
        fmt::print("error ({}): {}\n", code, message);
    }

    void createGLContext();

    GLFWwindow* mWindow;
    glx::WindowSettings settings;
    glx::WindowCallbacks callbacks;

	bool paused;
	bool forward;
	bool backward;
	bool left;
	bool right;
};
