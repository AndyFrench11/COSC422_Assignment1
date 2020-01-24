#version 330

in vec4 oColor;

uniform int wireframeBool;

void main() 
{
    if(wireframeBool == 1)
    {
        gl_FragColor = vec4(0,0,1,1);
    }
    else
    {
        gl_FragColor = oColor;
    }
    
}
