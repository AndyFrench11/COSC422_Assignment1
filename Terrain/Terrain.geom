#version 400
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform vec3 lightPos;
uniform mat4 mvpMatrix;
uniform float waterHeight;
uniform float snowHeight;

out float diffuse;
out vec2 texCoord;
out vec4 texWeight;

void main()
{
    int i;
    
    //For lighting
    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(ab, ac));
    
    for(i=0; i<gl_in.length(); i++)
    {
        vec4 posn = gl_in[i].gl_Position;
        vec3 lightVec = lightPos - posn.xyz;
        diffuse = max(dot(normal, normalize(lightVec)), 0);
        
        texCoord = posn.xz;
        
        if (posn.y < waterHeight) 
        {
            texWeight = vec4(1 * (1 - waterHeight/8), 0, 0, 0);
        }
        else if (posn.y > snowHeight)
        {
            texWeight = vec4(0, 0, 0, 1);
        }
        else if (posn.y < 1) 
        {
            texWeight = vec4(0, 0.25, 0, 0);
        }
        else if (posn.y < 1.5) 
        {
            texWeight = vec4(0, 0.375, 0, 0);
        }
        else if (posn.y < 2) 
        {
            texWeight = vec4(0, 0.5, 0, 0);
        }
        else if (posn.y < 2.5) 
        {
            texWeight = vec4(0, 0.625, 0, 0);
        }
        else if (posn.y < 3) 
        {
            texWeight = vec4(0, 0.75, 0, 0);
        }
        else if (posn.y < 3.5) 
        {
            texWeight = vec4(0, 0.875, 0, 0);
        }
        else if (posn.y < 4)
        {
            texWeight = vec4(0, 1, 0, 0);
        }
        else if (posn.y < 5)
        {
            texWeight = vec4(0, 0.75, 0.25, 0);
        }
        else if (posn.y < 6)
        {
            texWeight = vec4(0, 0.5, 0.5, 0);
        }
        else if (posn.y < 8)
        {
            texWeight = vec4(0, 0, 1, 0);
        }
        else if (posn.y < 9)
        {
            texWeight = vec4(0, 0, 0.75, 0.25);
        }
        else if (posn.y < 10) 
        {
            texWeight = vec4(0, 0, 0.5, 0.5);
        }
        else if (posn.y < 11) 
        {
            texWeight = vec4(0, 0, 0.25, 0.75);
        }
        
        
        gl_Position = mvpMatrix * posn;
        EmitVertex();
    }
    EndPrimitive();
}
