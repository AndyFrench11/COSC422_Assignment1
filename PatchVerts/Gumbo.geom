#version 400
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform vec4 lightPos;
uniform mat4 gumboMvpMatrix;
uniform mat4 gumboMvMatrix;
uniform mat4 gumboNorMatrix;

out vec4 oColor;


void main()
{
    
    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(ab, ac));
    
    
    int i;
    
    for(i=0; i < gl_in.length(); i++)
    {
        vec4 position = gl_in[i].gl_Position;
        
        //~ vec4 lightVec = lightPos - position;
        //~ float diffuse = max(dot(vec4(normal, 0), normalize(lightVec)), 0);
        
        //~ //For lighting
    
        vec4 white = vec4(1.0);
        vec4 grey = vec4(0.2);
        vec4 cyan = vec4(0.0, 1.0, 1.0, 1.0);

        vec4 posnEye = gumboMvMatrix * position;
        vec4 normalEye = gumboNorMatrix * vec4(normal, 0);
        vec4 lgtVec = normalize(lightPos - posnEye); 
        vec4 viewVec = normalize(vec4(-posnEye.xyz, 0)); 
        vec4 halfVec = normalize(lgtVec + viewVec); 

        vec4 material = vec4(0.0, 1.0, 1.0, 1.0);
        vec4 ambOut = grey * material;
        float shininess = 100.0;
        float diffTerm = max(dot(lgtVec, normalEye), 0);
        vec4 diffOut = material * diffTerm;
        float specTerm = max(dot(halfVec, normalEye), 0);
        vec4 specOut = white * pow(specTerm, shininess);    
        
        gl_Position = gumboMvpMatrix * position;
        oColor = ambOut + diffOut + specOut;
        //oColor = vec4(0, 0, 1, 0);
        EmitVertex();
    }
    EndPrimitive();
}
