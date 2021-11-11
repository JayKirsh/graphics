#include "lab.hpp"


float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Camera and Mouse Related variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);

bool mouse;
double dX;
double dY;
float yaw = -90.0f;
float pitch = -10.0f;

//1280, 720
float lastX = 1280 / 2.0f;
float lastY = 720 / 2.0f;

// ===-----------------LIGHT------------------===

Light::Light() : position{ 3,5,7 }, intensities{1,1,1} {

}

Light gLight;

// ===---------------TRIANGLE-----------------===

Triangle::Triangle()
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle    = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle    = glCreateShader(GL_FRAGMENT_SHADER);
}

void Triangle::loadShaders()
{
    std::string shaderRoot{ShaderPath};
    vertexSource =
        glx::readShaderSource(shaderRoot + "triangle.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "triangle.frag", IncludeDir);

    if (auto result{glx::compileShader(vertexSource.sourceString, mVertHandle)};
        result)
    {
        throw OpenGLError(*result);
    }

    if (auto result =
            glx::compileShader(fragmentSource.sourceString, mFragHandle);
        result)
    {
        throw OpenGLError(*result);
    }

    // communicate to OpenGL the shaders used to render the Triangle
    glAttachShader(mProgramHandle, mVertHandle);
    glAttachShader(mProgramHandle, mFragHandle);

    if (auto result = glx::linkShaders(mProgramHandle); result)
    {
        throw OpenGLError(*result);
    }

	setupUniformVariables();
}

void Triangle::loadDataToGPU(std::array<float, ENTRIES_PER_VERTEX* VERTICES_PER_TRIANGLE* TRIANGLES> const& vertices)
{
    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(9));

    // enable attributes for the three components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);
	glEnableVertexArrayAttrib(mVao, 2);

    // specify to OpenGL how the vertices, colors and normals are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));
	glVertexArrayAttribFormat(
		mVao, 2, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(6));

    // associate the vertex attributes (coordinates, color and normal) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
	glVertexArrayAttribBinding(mVao, 2, 0);
}

void Triangle::reloadShaders()
{
    if (glx::shouldShaderBeReloaded(vertexSource))
    {
        glx::reloadShader(
            mProgramHandle, mVertHandle, vertexSource, IncludeDir);
    }

    if (glx::shouldShaderBeReloaded(fragmentSource))
    {
        glx::reloadShader(
            mProgramHandle, mFragHandle, fragmentSource, IncludeDir);
    }
}

void Triangle::render([[maybe_unused]] bool paused,
	//movement key presses
	[[maybe_unused]] bool forward,
	[[maybe_unused]] bool backward,
	[[maybe_unused]] bool left,
	[[maybe_unused]] bool right,
	//Width n Height
	[[maybe_unused]] int width,
	[[maybe_unused]] int height)
{
	
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	float cameraSpeed = 10.0f * deltaTime;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

	if (!paused) {
		position = static_cast<float>(glfwGetTime()) * 64.0f;
	}

	if (mouse)
	{
		lastX = (float)dX;
		lastY = (float)dY;
		mouse = false;
	}

	float xOff = (float)dX - lastX;
	float yOff = lastY - (float)dY;
	lastX = (float)dX;
	lastY = (float)dY;

	float sens = 0.1f;

	xOff *= sens;
	yOff *= sens;

	yaw += xOff;
	pitch += yOff;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;

	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(direction);
	
	if (forward) {
		cameraPos += cameraSpeed * cameraFront;
	}

	if (backward) {
		cameraPos -= cameraSpeed * cameraFront;
	}

	if (left) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, up)) * cameraSpeed;
	}

	if (right) {
		cameraPos += glm::normalize(glm::cross(cameraFront, up)) * cameraSpeed;
	}

	auto modelMat{ glm::rotate(
		math::Matrix4{ 0.1f },
		glm::radians(position),
		math::Vector{ 1.0f, 1.0f, 0.0f }) };

	auto viewMat{ glm::lookAt(
		cameraPos, cameraPos + cameraFront, up)};

	auto projMat{ glm::perspective(
		glm::radians(60.0f),
		static_cast<float>(width) / height,
		nearVal,
		farVal
	) };

    reloadShaders();

    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);

	glUniformMatrix4fv(mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
	glUniformMatrix4fv(mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat));
	glUniform3f(mUniformLightPositionLoc, gLight.position[0], gLight.position[1], gLight.position[2]);
	glUniform3f(mUniformLightIntensityLoc, gLight.intensities[0], gLight.intensities[1], gLight.intensities[2]);
	
    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    glDrawArrays(GL_TRIANGLES, 0, 3*TRIANGLES);
}

void Triangle::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Triangle::setupUniformVariables()
{
	mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
	mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
	mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "projection");
	mUniformLightPositionLoc = glGetUniformLocation(mProgramHandle, "lightPosition");
	mUniformLightIntensityLoc = glGetUniformLocation(mProgramHandle, "lightIntensity");
}

// ===------------IMPLEMENTATIONS-------------===

Program::Program(int width, int height, std::string title) :
    settings{}, callbacks{}, paused{}, mWindow{nullptr}
{
    settings.size.width  = width;
    settings.size.height = height;
    settings.title       = title;

    if (!glx::initializeGLFW(errorCallback))
    {
        throw OpenGLError("Failed to initialize GLFW with error callback");
    }

    mWindow = glx::createGLFWWindow(settings);
    if (mWindow == nullptr)
    {
        throw OpenGLError("Failed to create GLFW Window");
    }

	callbacks.keyPressCallback = [&](int key, int, int action, int) {

		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			paused = !paused;
		}

		//Movement keypresses
		if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			forward = 1;
		}
		else {
			forward = 0;
		}


		if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			backward = 1;
		}
		else {
			backward = 0;
		}

		if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			left = 1;
		}
		else {
			left = 0;
		}

		if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			right = 1;
		}
		else {
			right = 0;
		}
	};

	callbacks.mouseMoveCallback = [&](double x, double y) {
		dX = x;
		dY = y;
	};

    createGLContext();
}

void Program::run(Triangle& tri)
{
    glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(mWindow))
    {
        int width;
        int height;

        glfwGetFramebufferSize(mWindow, &width, &height);
        // setup the view to be the window's size
        glViewport(0, 0, width, height);
        // tell OpenGL the what color to clear the screen to
        glClearColor(0, 0, 0, 1);
        // actually clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tri.render(paused, forward, backward, left, right, width, height);

        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
}

void Program::freeGPUData()
{
    glx::destroyGLFWWindow(mWindow);
    glx::terminateGLFW();
}

void Program::createGLContext()
{
    using namespace magic_enum::bitwise_operators;

    glx::bindWindowCallbacks(mWindow, callbacks);
    glfwMakeContextCurrent(mWindow);
    glfwSwapInterval(1);

    if (!glx::createGLContext(mWindow, settings.version))
    {
        throw OpenGLError("Failed to create OpenGL context");
    }

    glx::initializeGLCallback(glx::ErrorSource::All,
                              glx::ErrorType::All,
                              glx::ErrorSeverity::High |
                                  glx::ErrorSeverity::Medium);
}

// ===-----------------DRIVER-----------------===

int main()
{
    try
    {
        // clang-format off
        std::array<float, ENTRIES_PER_VERTEX * VERTICES_PER_TRIANGLE * TRIANGLES> vertices
        {
            // Vertices          Colours				Normals

			- 0.5f,-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,	0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,	0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f
        };
        // clang-format on

        Program prog{1280, 720, "Rotating Cube"};
        Triangle tri{};

        tri.loadShaders();
        tri.loadDataToGPU(vertices);

        prog.run(tri);

        prog.freeGPUData();
        tri.freeGPUData();
    }
    catch (OpenGLError& err)
    {
        fmt::print("OpenGL Error:\n\t{}\n", err.what());
    }

    return 0;
}
