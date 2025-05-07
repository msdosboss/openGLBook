#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D ourTexture;
uniform sampler2D texture2;

uniform float mixValue;

void main(){
    FragColor = mix(texture(ourTexture, texCoord), texture(texture2, vec2(-texCoord.x, texCoord.y)), mixValue);
}
