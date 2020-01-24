//  ========================================================================
//  COSC422: Computer Graphics (2018);  University of Canterbury.
//
//  FILE NAME: Terrain.cpp
//  This is part of Assignment1 files.
//
//  The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Required files:  Terrain.vert (vertex shader), Terrain.frag (fragment shader), 
//  HeightMap1.tga, HeightMap2.tga  (height map), Terrain.geom (geometry shader), 
//  Terrain.cont (tesselation control shader), Terrain.eval (evaluation shader)
//  snow.tga, grass.tga, rock.tga, water.tga
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"
using namespace std;

GLuint vaoID;
GLuint theProgram;
GLuint mvpMatrixLoc;
GLuint cameraPositionLoc;
GLuint lightPosLoc;
glm::vec3 light;

//Texture properties
GLuint heightMap;
GLuint waterTexture;
GLuint rockTexture;
GLuint grassTexture;
GLuint snowTexture;

GLuint waterHeightLoc;
float waterHeight = 2;
GLuint snowHeightLoc;
float snowHeight = 9;

GLuint wireframeLoc;
int wireframeBool = 0;

glm::mat4 projView;
glm::vec4 cameraPosition = glm::vec4(glm::vec3(0.0, 10.0, 30.0), 0);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -40.0f);
glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

float CDR = 3.14159265/180.0;     //Conversion from degrees to rad (required in GLM 0.9.6)

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];       //Element array for 81 quad patches

//Loads terrain texture
void loadTextures()
{
    glGenTextures(1, &heightMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    loadTGA("HeightMap1.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &waterTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, waterTexture);
    loadTGA("water.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &rockTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rockTexture);
    loadTGA("rock.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &grassTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    loadTGA("grass.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &snowTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, snowTexture);
    loadTGA("snow.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    
}

//Generate vertex and element data for the terrain floor
void generateData()
{
    int indx, start;
    //verts array
    for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
    {
        for(int j = 0; j < 10; j++)
        {
            indx = 10*i + j;
            verts[3*indx] = 10*i - 45;      //x  varies from -45 to +45
            verts[3*indx+1] = 0;            //y  is set to 0 (ground plane)
            verts[3*indx+2] = -10*j;        //z  varies from 0 to -100
        }
    }

    //elems array
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            indx = 9*i +j;
            start = 10*i + j;
            elems[4*indx] = start;
            elems[4*indx+1] = start+10;
            elems[4*indx+2] = start+11;
            elems[4*indx+3] = start+1;          
        }
    }
}


//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
    ifstream shaderFile(filename.c_str());
    if(!shaderFile.good()) cout << "Error opening shader file." << endl;
    stringstream shaderData;
    shaderData << shaderFile.rdbuf();
    shaderFile.close();
    string shaderStr = shaderData.str();
    const char* shaderTxt = shaderStr.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderTxt, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        const char *strShaderType = NULL;
        cerr <<  "Compile failure in shader: " << strInfoLog << endl;
        delete[] strInfoLog;
    }
    return shader;
}

//Initialise the shader program, create and load buffer data
void initialise()
{
    
//--------Load terrain height map-----------
    loadTextures();
//--------Load shaders----------------------
    GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Terrain.vert");
    GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Terrain.frag");
    GLuint shaderc = loadShader(GL_TESS_CONTROL_SHADER, "Terrain.cont");
    GLuint shadere = loadShader(GL_TESS_EVALUATION_SHADER, "Terrain.eval");
    GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "Terrain.geom");
    
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    GLuint program = glCreateProgram();
    glAttachShader(program, shaderv);
    glAttachShader(program, shaderf);
    glAttachShader(program, shaderc);
    glAttachShader(program, shadere);
    glAttachShader(program, shaderg);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv (program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }
    glUseProgram(program);
    
    //Lighting
    light = glm::vec3(500.0, 1000.0, 500.0);
    lightPosLoc = glGetUniformLocation(program, "lightPos");
    glUniform3fv(lightPosLoc, 1, &light[0]);
    
    wireframeLoc = glGetUniformLocation(program, "toggleWireframe");

    mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
    
    GLuint texLoc = glGetUniformLocation(program, "heightMap");
    glUniform1i(texLoc, 0);
    texLoc = glGetUniformLocation(program, "water");
    glUniform1i(texLoc, 1);
    texLoc = glGetUniformLocation(program, "rock");
    glUniform1i(texLoc, 2);
    texLoc = glGetUniformLocation(program, "grass");
    glUniform1i(texLoc, 3);
    texLoc = glGetUniformLocation(program, "snow");
    glUniform1i(texLoc, 4);
    texLoc = glGetUniformLocation(program, "heightMap2");
    glUniform1i(texLoc, 5);
    
    //Water Level
    waterHeightLoc = glGetUniformLocation(program, "waterHeight");
    
    //Snow Level
    snowHeightLoc = glGetUniformLocation(program, "snowHeight");
    
    cameraPositionLoc = glGetUniformLocation(program, "cameraPosition");
   

//---------Load buffer data-----------------------
    generateData();

    GLuint vboID[2];
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
    glm::mat4 proj, view;   //Projection and view matrices
    
    //Camera Calculations
    glUniform4fv(cameraPositionLoc, 1, &cameraPosition[0]);
    
        //--------Compute matrices----------------------
    proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
    view = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraPosition) + cameraFront, cameraUpVector); //view matrix
    projView = proj * view;  //Product (mvp) matrix
    
    // BEWARE NOT TO MOVE THIS LINE
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
    
    // Set wireframe mode
    if(wireframeBool == 1)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
    }
    glUniform1i(wireframeLoc, wireframeBool); 
    
    //Raising levels binding
    glUniform1f(waterHeightLoc, waterHeight);
    glUniform1f(snowHeightLoc, snowHeight);
    
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vaoID);
    glDrawElements(GL_PATCHES, 81*4, GL_UNSIGNED_SHORT, NULL);

    glFlush();
}

void specialKeyEvent(int key, int x, int y)
{
    if(key == GLUT_KEY_LEFT)
    {
        cameraPosition = cameraPosition - glm::vec4(glm::vec3(1, 0, 0), 0);
    }
    else if(key == GLUT_KEY_UP)
    {
        cameraPosition = cameraPosition - glm::vec4(glm::vec3(0, 0, 1), 0);
    }
    else if(key == GLUT_KEY_RIGHT)
    {
        cameraPosition = cameraPosition + glm::vec4(glm::vec3(1, 0, 0), 0);
    }
    else if(key == GLUT_KEY_DOWN)
    {
        cameraPosition = cameraPosition + glm::vec4(glm::vec3(0, 0, 1), 0);
    }
}

void keyboardEvent(unsigned char key, int x, int y)
{
    if(key == '1')
    {
        glGenTextures(1, &heightMap);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightMap);
        loadTGA("HeightMap1.tga");

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else if(key == '2')
    {
        glGenTextures(1, &heightMap);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightMap);
        loadTGA("HeightMap2.tga");

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else if(key == 'w')
    {
        if(wireframeBool == 1)
        {
            wireframeBool = 0;
        } 
        else
        {
            wireframeBool = 1;
        }
    }
    else if(key == 'p')
    {
        //Move the water level up
        if(waterHeight < 4)
        {
            waterHeight += 0.1;
        }
    }
    else if(key == 'o')
    {
        //Move the water level down
        if(waterHeight > 1)
        {
            waterHeight -= 0.1; 
        }
    }
    else if(key == 'l')
    {
        //Move the snow level up
        if(snowHeight < 10)
        {
            snowHeight += 0.1;
        }
    }
    else if(key == 'k')
    {
        //Move the snow level down
        if(snowHeight > 6)
        {
            snowHeight -= 0.1;
        }
    }
  
}

void myTimer(int update)
{
    glutTimerFunc(20, myTimer, 0);
    glutPostRedisplay();
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Andrew French - COSC422 Assignment 1 - Terrain");
    glutInitContextVersion (4, 2);
    glutInitContextProfile ( GLUT_CORE_PROFILE );

    if(glewInit() == GLEW_OK)
    {
        cout << "GLEW initialization successful! " << endl;
        cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
    }
    else
    {
        cerr << "Unable to initialize GLEW  ...exiting." << endl;
        exit(EXIT_FAILURE);
    }

    initialise();
    glutDisplayFunc(display);
    glutSpecialFunc(specialKeyEvent);
    glutKeyboardFunc(keyboardEvent);
    glutTimerFunc(20, myTimer, 0);
    glutMainLoop();
}

