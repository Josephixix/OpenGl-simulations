#include <iostream>
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;

// Callback for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Shader compilation checker
void checkShaderCompile(unsigned int shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }
}

// Shader program linking checker
void checkProgramLink(unsigned int program) {
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
}

// Function to create circle vertices
vector<float> createCircle(float radius, int segments) {
    vector<float> vertices;
    vertices.push_back(0.0f); // center x
    vertices.push_back(0.0f); // center y
    vertices.push_back(0.0f); // center z
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.1415926f * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
    }
    return vertices;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // --- Shader sources ---
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform vec2 offset;
        void main()
        {
            gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main()
        {
            FragColor = vec4(color, 1.0);
        }
    )";

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompile(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- Create circle vertices ---
    int circleSegments = 50;
    vector<float> circleVertices = createCircle(1.0f, circleSegments); // base circle radius=1.0, will scale later

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- Planet properties: {orbit radius, speed, size, color RGB} ---
    struct Planet {
        float orbitRadius;
        float speed;
        float size;
        float color[3];
        float angle;
    };

    vector<Planet> planets = {
        {0.25f, 1.0f, 0.05f, {1.0f, 0.0f, 0.0f}, 0.0f}, // Mercury
        {0.4f, 0.6f, 0.08f, {0.0f, 1.0f, 0.0f}, 0.0f},  // Venus
        {0.55f, 0.4f, 0.09f, {0.0f, 0.0f, 1.0f}, 0.0f},  // Earth
        {0.7f, 0.3f, 0.07f, {1.0f, 1.0f, 0.0f}, 0.0f}    // Mars
    };

    float lastTime = glfwGetTime();

    // --- Render loop ---
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Update planet angles
        for (auto &p : planets) {
            p.angle += p.speed * deltaTime;
            if (p.angle > 2.0f * 3.1415926f) p.angle -= 2.0f * 3.1415926f;
        }

        // Clear screen
        glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int colorLoc = glGetUniformLocation(shaderProgram, "color");
        int offsetLoc = glGetUniformLocation(shaderProgram, "offset");

        // --- Draw Sun ---
        float sunScale = 0.12f;
        vector<float> scaledSunVertices;
        for (size_t i = 0; i < circleVertices.size(); i += 3) {
            scaledSunVertices.push_back(circleVertices[i] * sunScale);
            scaledSunVertices.push_back(circleVertices[i+1] * sunScale);
            scaledSunVertices.push_back(circleVertices[i+2]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, scaledSunVertices.size() * sizeof(float), scaledSunVertices.data());
        glUniform3f(colorLoc, 1.0f, 0.5f, 0.0f); // orange sun
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, circleSegments + 2);

        // --- Draw planets ---
        for (auto &p : planets) {
            float x = p.orbitRadius * cos(p.angle);
            float y = p.orbitRadius * sin(p.angle);

            // Scale planet vertices
            vector<float> scaledVertices;
            for (size_t i = 0; i < circleVertices.size(); i += 3) {
                scaledVertices.push_back(circleVertices[i] * p.size);
                scaledVertices.push_back(circleVertices[i+1] * p.size);
                scaledVertices.push_back(circleVertices[i+2]);
            }
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, scaledVertices.size() * sizeof(float), scaledVertices.data());

            glUniform2f(offsetLoc, x, y);
            glUniform3f(colorLoc, p.color[0], p.color[1], p.color[2]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, circleSegments + 2);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}