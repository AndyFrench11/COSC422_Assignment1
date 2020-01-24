#version 330

uniform sampler2D water;
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D snow;

in float diffuse;
in vec2 texCoord;
in vec4 texWeight;
out vec4 FragColor;

uniform int toggleWireframe;

void main() 
{
    vec4 texWater = texture(water, texCoord);
    vec4 texGrass = texture(grass, texCoord);
    vec4 texRock = texture(rock, texCoord);
    vec4 texSnow = texture(snow, texCoord);
    
    //Do Lighting Calculations

    vec4 diffuseVector = vec4(diffuse, diffuse, diffuse, 1);
    vec4 grey = vec4(0.2);
    
    if(toggleWireframe == 0) {
        FragColor = (texWater * texWeight.x + texGrass * texWeight.y + texRock * texWeight.z + texSnow * texWeight.w);
        FragColor = FragColor * (diffuseVector + grey);
    } else
    {
        FragColor = vec4(0, 0, 1, 1) * (diffuseVector + grey);
    }
    
    
}
