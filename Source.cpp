#include <iostream>         //Import Necessary Libraries 
#include <cstdlib>          
#include <GL/glew.h>        
#include <GLFW/glfw3.h>  
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>  

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>  
#include <camera.h>



using namespace std; // Standard namespace
/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif
namespace
{
    const char* const WINDOW_TITLE = "Kevin Raddatz---FInal Project---CS330"; // Macro for window title

    glm::vec3 gCameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 gCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float gDeltaTime = 0.0f; // Time between current frame and last frame
    float gLastFrame = 0.0f;

    const int WINDOW_WIDTH = 800; //Define Window Width and Height Constants 
    const int WINDOW_HEIGHT = 600;

    struct GLMesh //Creation of GLMesh structure 
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };
    GLFWwindow* gWindow = nullptr;
    GLMesh gMesh;
    GLuint gProgramId;

    GLuint gCubeProgramId; //Added Light Programs 
    GLuint gLampProgramId;
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f); //Lighting Properties
    glm::vec3 gCubeScale(2.0f);
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightPosition(1.5f, 0.5f, 3.0f);
    glm::vec3 gLightScale(0.3f)

}

//Function Declarations 
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

//Vertex Shader Source Code, updated for texturing 
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 2) in vec2 textureCoordinate;

out vec2 vertexTextureCoordinate;

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    vertexTextureCoordinate = textureCoordinate;
}
);


//Fragment shader source code, updated for texturing 
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform sampler2D uTexture;

void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate); // Sends texture to the GPU for rendering
}
);


void flipImageVertically(unsigned char* image, int width, int height, int channels) //Function to flip image
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

//Main Function 
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Creation of Shader Program and Setting Background to Black. In future, want to set background color to match wall
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    const char* texFilename = "structure-board-wood-grain.jpg"; //Texture file 
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;

        glUseProgram(gProgramId); //Set tecture unit 0 
        glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Render loop
    while (!glfwWindowShouldClose(gWindow))
    {
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        UProcessInput(gWindow);

        GLUquadric* gluNewQuadric; //From Source Documentation, Attempting to Create cylinder using gluCylinder from GLU. 
        gluCylinder(gluNewQuadric, 0.5f, 0.5f, 1.0f, 1, 1 );
        gluSphere(gluNewQuadric, 0.4f,1, 1 ); //Making Sphere slightly smaller than stand


        //TODO: Need to add in code to move Cylinder/Cone to correct positions. 

        URender();
        glfwPollEvents();
    }
    UDestroyMesh(gMesh);
    UDestroyShaderProgram(gProgramId);
    exit(EXIT_SUCCESS);
}


//Function Definitions 
bool UInitialize(int argc, char* argv[], GLFWwindow** window) //Initialization function 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}

void UProcessInput(GLFWwindow* window) //Process input function 
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraOffset = cameraSpeed * gDeltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) //Rotating camera view when keys pressed
        gCameraPos += cameraOffset * gCameraFront; //NOTE: Some Issues with W and S key. Object Disappears until returns to original state
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) //Possibly a Matrix Multiplication Error? 
        gCameraPos -= cameraOffset * gCameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCameraPos -= glm::normalize(glm::cross(gCameraFront, gCameraUp)) * cameraOffset;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCameraPos += glm::normalize(glm::cross(gCameraFront, gCameraUp)) * cameraOffset;

}

void UResizeWindow(GLFWwindow* window, int width, int height) //Function to resize window 
{
    glViewport(0, 0, width, height);
}

void URender() //Render Function 
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //Scaling, rotating, and translating of shape 
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(1.0, 1.0f, 1.0f));
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    //Creating Model matrix
    glm::mat4 model = translation * rotation * scale;
    //glm::mat4 view = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f));
    glm::mat4 view = glm::lookAt(gCameraPos, gCameraPos + gCameraFront, gCameraUp);
    glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    // glm::mat4 view = gCamera.GetViewMatrix();

     //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    glUseProgram(gProgramId);

    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(gMesh.vao);
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);
    glfwSwapBuffers(gWindow);
}



void UCreateMesh(GLMesh& mesh)//UCreateMesh function 
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex               // Colors                   //textures
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   0.5f, 0.5f, 0.5f, 0.5f, //Adding a bit of Shadow to surfaces
        -0.5f, -0.5f, 0.0f,   0.5f, 0.5f, 0.5f, 0.5f,
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,

         0.5f, -0.5f, -1.0f,  0.5f, 0.5f, 0.5f, 0.5f,
         0.5f,  0.5f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f

        - 5.0f, -5.0f, -5.0f,  0.56f, 0.28f, 0.0f, 1.0f, //8
         5.0f, -5.0f, -5.0f,   0.56f, 0.28f, 0.0f, 1.0f,
         5.0f, -5.0f, 5.0f,    0.56f, 0.28f, 0.0f, 1.0f,
         5.0f, -5.0f, 5.0f,    0.56f, 0.28f, 0.0f, 1.0f, //11




    };
    // Indices
    GLushort indices[] = {
        0, 1, 3,
        1, 2, 3,
        0, 1, 4, 
        0, 4, 5,
        0, 5, 6,
        0, 3, 6, 
        4, 5, 6,
        4, 6, 7,
        2, 3, 6, 
        2, 6, 7,
        1, 4, 7,
        1, 2, 7,

       8, 9, 10, 
       9, 10, 11
        

        
    };
 
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    /*unsigned int VBO, VAO; //Possible Solution?
    glGenVertexArrays(1, &VAO); //Code Required for Plane
    glGenBuffers(1, &VBO); 
    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW); */

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

   //Creating buffers for Vertex and indices 
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    /*
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW);

    mesh.nIndices = sizeof(planeIndices) / sizeof(planeIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);*/ 


    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

void UDestroyMesh(GLMesh& mesh) //Destroy mesh Function 
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

//UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    int success = 0;
    char infoLog[512];
    programId = glCreateProgram();

    // Creating the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    //Pulling Shader source code from top of project
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compiling Shaders and Checking for errors
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }
    glCompileShader(fragmentShaderId);
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);

    return true;
}
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    cout << "Mouse at (" << xpos << ", " << ypos << ")" << endl;
}
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    cout << "Mouse wheel (" << xoffset << ", " << yoffset << ")" << endl;
}
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


void UDestroyShaderProgram(GLuint programId) //Destroy Shader Program
{
    glDeleteProgram(programId);
}

bool UCreateTexture(const char* filename, GLuint& textureId) //Create Texture
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //Setting texture wrapping and filtering parameters 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture.

        return true;
    }

    // Error loading the image
    return false;
}
void UDestroyTexture(GLuint textureId) //Destroy Texture
{
    glGenTextures(1, &textureId);
}
