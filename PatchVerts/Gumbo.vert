#version 330

layout (location = 0) in vec4 position;

uniform mat4 gumboMvpMatrix;
out vec4 gl_Position;


void main()
{
   gl_Position = position;
}
