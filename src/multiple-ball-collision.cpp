#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>  // for rand()
#include <ctime>    // for srand()
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Shader error checking
void checkShaderCompile(unsigned int shader)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }
}

// Program linking error checking
void checkProgramLink(unsigned int program)
{
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
}

// Ball structure
struct Ball {
    float x, y;
    float vx, vy;
    float radius;
    float r, g, b;
};

// Function to generate circle vertices
vector<float> generateCircleVertices(float radius, int segments)
{
    vector<float> vertices;
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    for(int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.1415926f * i / segments;
        float vx = radius * cos(angle);
        float vy = radius * sin(angle);
        vertices.push_back(vx);
        vertices.push_back(vy);
        vertices.push_back(0.0f);
    }
    return vertices;
}

int main()
{
    srand(time(0));

    // --- GLFW init ---
    if(!glfwInit())
    {
        cout << "Failed to init GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800,600,"Multi Ball Collision",NULL,NULL);
    if(!window)
    {
        cout << "Failed to create window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to init GLAD" << endl;
        return -1;
    }

    // --- Shaders ---
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform float offsetX;
        uniform float offsetY;
        void main()
        {
            gl_Position = vec4(aPos.x + offsetX, aPos.y + offsetY, aPos.z, 1.0);
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main()
        {
            FragColor = vec4(color,1.0);
        }
    )";

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs,1,&vertexShaderSrc,NULL);
    glCompileShader(vs);
    checkShaderCompile(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs,1,&fragmentShaderSrc,NULL);
    glCompileShader(fs);
    checkShaderCompile(fs);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vs);
    glAttachShader(shaderProgram,fs);
    glLinkProgram(shaderProgram);
    checkProgramLink(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // --- Circle geometry ---
    const int segments = 50;
    float radius = 0.05f;
    vector<float> vertices = generateCircleVertices(radius, segments);

    unsigned int VAO,VBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    // --- Physics setup ---
    const int ballCount = 5;
    vector<Ball> balls(ballCount);
    for(int i=0;i<ballCount;i++)
    {
        balls[i].x = ((rand()%100)/100.0f)*1.8f - 0.9f; // x in -0.9 to 0.9
        balls[i].y = ((rand()%100)/100.0f)*0.5f + 0.5f; // y in 0.5 to 1.0
        balls[i].vx = ((rand()%200)/100.0f - 1.0f)*0.5f; // vx -0.5 to 0.5
        balls[i].vy = 0.0f;
        balls[i].radius = radius;
        balls[i].r = (rand()%100)/100.0f;
        balls[i].g = (rand()%100)/100.0f;
        balls[i].b = (rand()%100)/100.0f;
    }

    float gravity = -1.5f;
    float restitution = 0.8f;

    float lastTime = glfwGetTime();

    // --- Render loop ---
    while(!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        // Physics update
        for(int i=0;i<ballCount;i++)
        {
            balls[i].vy += gravity*dt;
            balls[i].x += balls[i].vx*dt;
            balls[i].y += balls[i].vy*dt;

            // Ground collision
            if(balls[i].y - balls[i].radius < -1.0f)
            {
                balls[i].y = -1.0f + balls[i].radius;
                balls[i].vy = -balls[i].vy*restitution;
            }
            // Left/Right walls
            if(balls[i].x - balls[i].radius < -1.0f)
            {
                balls[i].x = -1.0f + balls[i].radius;
                balls[i].vx = -balls[i].vx*restitution;
            }
            if(balls[i].x + balls[i].radius > 1.0f)
            {
                balls[i].x = 1.0f - balls[i].radius;
                balls[i].vx = -balls[i].vx*restitution;
            }
        }

        // Ball-to-ball collisions
        for(int i=0;i<ballCount;i++)
        {
            for(int j=i+1;j<ballCount;j++)
            {
                float dx = balls[j].x - balls[i].x;
                float dy = balls[j].y - balls[i].y;
                float dist = sqrt(dx*dx + dy*dy);
                float minDist = balls[i].radius + balls[j].radius;

                if(dist < minDist && dist > 0.0f)
                {
                    // Normalize
                    float nx = dx/dist;
                    float ny = dy/dist;

                    // Relative velocity
                    float vx_rel = balls[j].vx - balls[i].vx;
                    float vy_rel = balls[j].vy - balls[i].vy;
                    float relVel = vx_rel*nx + vy_rel*ny;

                    // Skip if moving apart
                    if(relVel > 0) continue;

                    // Elastic impulse
                    float impulse = -(1+restitution)*relVel/2.0f;
                    balls[i].vx -= impulse*nx;
                    balls[i].vy -= impulse*ny;
                    balls[j].vx += impulse*nx;
                    balls[j].vy += impulse*ny;

                    // Separate overlapping balls
                    float overlap = 0.5f*(minDist - dist);
                    balls[i].x -= overlap*nx;
                    balls[i].y -= overlap*ny;
                    balls[j].x += overlap*nx;
                    balls[j].y += overlap*ny;
                }
            }
        }

        // Clear screen
        glClearColor(0.05f,0.05f,0.05f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw balls
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for(int i=0;i<ballCount;i++)
        {
            int offsetXLoc = glGetUniformLocation(shaderProgram,"offsetX");
            int offsetYLoc = glGetUniformLocation(shaderProgram,"offsetY");
            int colorLoc = glGetUniformLocation(shaderProgram,"color");

            glUniform1f(offsetXLoc, balls[i].x);
            glUniform1f(offsetYLoc, balls[i].y);
            glUniform3f(colorLoc, balls[i].r, balls[i].g, balls[i].b);

            glDrawArrays(GL_TRIANGLE_FAN,0,segments+2);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}