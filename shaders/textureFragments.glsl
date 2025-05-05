#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 texCoord;
//uniform vec4 ourColor;
//uniform float colorOffset;
uniform sampler2D ourTexture;
uniform sampler2D texture2;

uniform float mixValue;

void main(){
   //FragColor = vec4(ourColor.x + colorOffset, ourColor.y + colorOffset, ourColor.z + colorOffset, 1.0f);
    FragColor = mix(texture(ourTexture, texCoord), texture(texture2, vec2(-texCoord.x, texCoord.y)), mixValue);
}
