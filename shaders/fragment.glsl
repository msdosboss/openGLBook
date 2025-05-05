#version 330 core
out vec4 FragColor;
in vec3 ourColor;
//uniform vec4 ourColor;
uniform float colorOffset;


void main(){
    FragColor = vec4(ourColor.x + colorOffset, ourColor.y + colorOffset, ourColor.z + colorOffset, 1.0f);
}
