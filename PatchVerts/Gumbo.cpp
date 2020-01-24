//  ========================================================================
//  COSC422: Computer Graphics (2018);  University of Canterbury.
//
//  FILE NAME: Gumbo.cpp
//  This is part of Assignment1 files.
//
//  Required files:  Gumbo.vert (vertex shader), Gumbo.frag (fragment shader),
//  Gumbo.eval (evalution shader), Gumbo.geom (geometry shader), Gumbo.cont (control shader),
//  Floor.vert, Floor.frag, PatchVerts_Gumbo.txt
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

GLuint floorVao;
GLuint gumboVao;
GLuint floorProgram;
GLuint gumboProgram;
GLuint floorMvpMatrixLoc;
GLuint gumboMvpMatrixLoc;
GLuint gumboMvMatrixLoc;
GLuint gumboNorMatrixLoc;
float gumboAngle = 0.0;

GLuint cameraPositionLoc;
GLuint lightPosLoc;

GLuint wireframeLoc;
int wireframeBool = 1;

GLuint explosionLoc;
int explosionBool = 0;
GLuint explosionTimeLoc;
float explosionTime = 0.0;

glm::vec4 cameraPosition = glm::vec4(glm::vec3(0.0, 10.0, 50.0), 0);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -40.0f);
glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

float CDR = 3.14159265/180.0;     //Conversion from degrees to rad (required in GLM 0.9.6)

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];       //Element array for 81 quad patches

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
            verts[3*indx+2] = -10*j + 25;        //z  varies from 0 to -100
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
    
//--------Load shaders----------------------
    //Floor Program
    GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Floor.vert");
    GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Floor.frag");

    floorProgram = glCreateProgram();
    glAttachShader(floorProgram, shaderv);
    glAttachShader(floorProgram, shaderf);
    glLinkProgram(floorProgram);

    GLint floorStatus;
    glGetProgramiv (floorProgram, GL_LINK_STATUS, &floorStatus);
    
    if (floorStatus == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(floorProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(floorProgram, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }
    
    glUseProgram(floorProgram);
    
    floorMvpMatrixLoc = glGetUniformLocation(floorProgram, "floorMvpMatrix");

//---------Load buffer data-----------------------
    generateData();

    GLuint vboID[2];
    glGenVertexArrays(1, &floorVao);
    glBindVertexArray(floorVao);

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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Gumbo Program
    
    GLuint gumboShaderv = loadShader(GL_VERTEX_SHADER, "Gumbo.vert");
    GLuint gumboShaderf = loadShader(GL_FRAGMENT_SHADER, "Gumbo.frag");
    GLuint gumboShaderc = loadShader(GL_TESS_CONTROL_SHADER, "Gumbo.cont");
    GLuint gumboShadere = loadShader(GL_TESS_EVALUATION_SHADER, "Gumbo.eval");
    GLuint gumboShaderg = loadShader(GL_GEOMETRY_SHADER, "Gumbo.geom");
    
    glPatchParameteri(GL_PATCH_VERTICES, 16);

    gumboProgram = glCreateProgram();
    glAttachShader(gumboProgram, gumboShaderv);
    glAttachShader(gumboProgram, gumboShaderf);
    glAttachShader(gumboProgram, gumboShaderc);
    glAttachShader(gumboProgram, gumboShadere);
    glAttachShader(gumboProgram, gumboShaderg);
    glLinkProgram(gumboProgram);

    GLint gumboStatus;
    glGetProgramiv (gumboProgram, GL_LINK_STATUS, &gumboStatus);
    
    if (gumboStatus == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(gumboProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(gumboProgram, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }
    
    glUseProgram(gumboProgram);
    
    gumboMvpMatrixLoc = glGetUniformLocation(gumboProgram, "gumboMvpMatrix");
    gumboMvMatrixLoc = glGetUniformLocation(gumboProgram, "gumboMvMatrix");
    gumboNorMatrixLoc = glGetUniformLocation(gumboProgram, "gumboNorMatrix");
    lightPosLoc = glGetUniformLocation(gumboProgram, "lightPos");
    
    cameraPositionLoc = glGetUniformLocation(gumboProgram, "cameraPosition");
    
    explosionLoc = glGetUniformLocation(gumboProgram, "explosionBool");
    explosionTimeLoc = glGetUniformLocation(gumboProgram, "explosionTime");
    
    wireframeLoc = glGetUniformLocation(gumboProgram, "wireframeBool");
    
    //---------Load buffer data-----------------------
    ifstream infile;
    infile.open("PatchVerts_Gumbo.txt", ios::in);
    float gumboVerts[2048*3];
    int nvert;
    infile >> nvert;
    for(int i = 0; i < nvert; i++)
    {
        float x, y, z;
        infile >> x >> y >> z;
        gumboVerts[3*i] = x;
        gumboVerts[3*i + 1] = y;
        gumboVerts[3*i + 2] = z;
    }

    glGenVertexArrays(1, &gumboVao);
    glBindVertexArray(gumboVao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(gumboVerts), gumboVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    

}

void displayFloor()
{
     glm::mat4 proj, view, projView;   //Projection and view matrices
    
    //--------Compute matrices----------------------
    proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
    view = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraPosition) + cameraFront, cameraUpVector); //view matrix
    projView = proj * view;  //Product (mvp) matrix
    
    //Camera Calculations
    glUniform4fv(cameraPositionLoc, 1, &cameraPosition[0]);
    
    //Draw Floor
    
    glUseProgram(floorProgram);
    
    // BEWARE NOT TO MOVE THIS LINE
    glUniformMatrix4fv(floorMvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
    glBindVertexArray(floorVao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_QUADS, 81*4, GL_UNSIGNED_SHORT, NULL);
    
    glFlush();
}

void displayGumbo()
{
    //~ glm::mat4 proj, view, projView;   //Projection and view matrices
    
    //--------Compute matrices----------------------
    //~ proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
    //~ view = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraPosition) + cameraFront, cameraUpVector); //view matrix
    //~ projView = proj * view;  //Product (mvp) matrix


    //Lighting Calcs
    glm::vec4 light = glm::vec4(20.0, 10.0, 20.0, 1.0);
    glm::mat4 proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
    glm::mat4 view = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraPosition) + cameraFront, cameraUpVector); //view matrix
    glm::mat4 mvMatrix = glm::rotate(view, gumboAngle*CDR, glm::vec3(0.0, 1.0, 0.0));  //rotation matrix
    glm::mat4 mvpMatrix = proj * mvMatrix;   //The model-view-projection matrix
    glm::vec4 lightEye = view * light;     //Light position in eye coordinates
    glm::mat4 invMatrix = glm::inverse(mvMatrix);  //Inverse of model-view matrix for normal transformation
    
    //Draw Gumbo
    
    glUseProgram(gumboProgram);
    
    //Camera Calculations
    glUniform4fv(cameraPositionLoc, 1, &cameraPosition[0]);
    
    //~ glUniformMatrix4fv(gumboMvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
    glUniformMatrix4fv(gumboMvMatrixLoc, 1, GL_FALSE, &mvMatrix[0][0]);
    glUniformMatrix4fv(gumboMvpMatrixLoc, 1, GL_FALSE, &mvpMatrix[0][0]);
    glUniformMatrix4fv(gumboNorMatrixLoc, 1, GL_TRUE, &invMatrix[0][0]);  //Use transpose matrix here
    glUniform4fv(lightPosLoc, 1, &lightEye[0]);
    
    glBindVertexArray(gumboVao);
      
    if(wireframeBool == 1)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
     
    glUniform1i(wireframeLoc, wireframeBool);
    glUniform1i(explosionLoc, explosionBool);
    glUniform1f(explosionTimeLoc, explosionTime); 
    
    glDrawArrays(GL_PATCHES, 0, 2048);
    glFlush();  
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    displayGumbo();
    displayFloor();
    

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
    if(key == 'w')
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
    else if(key == 32)
    {
        if(explosionBool == 0)
        {
            explosionBool = 1;
        }
        else
        {
            explosionBool = 0;
        }
    }
  
}

void myTimer(int update)
{
    if(explosionBool == 1) 
    {
        explosionTime += 0.2;
    }
    else {
        gumboAngle++;
        explosionTime = 0;
    }
    glutTimerFunc(20, myTimer, 0);
    glutPostRedisplay();
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Andrew French - COSC422 Assignment 1 - Gumbo");
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
