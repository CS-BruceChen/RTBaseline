#version 460 core
in float id;
out vec4 FragColor;

uniform float MAXN;

void main(){
	float val = id / (MAXN-1);
	FragColor = vec4(val, 1-val, 0, 1);
}
