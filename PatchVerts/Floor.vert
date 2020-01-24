#version 330

layout (location = 0) in vec4 position;

uniform mat4 floorMvpMatrix;
out vec4 gl_Position;


void main()
{
   gl_Position = floorMvpMatrix * position;
}
