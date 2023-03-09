#version 460 core
out vec4 FragColor;
uniform sampler2D trajFBO;

uniform float MAXN;

uniform layout(binding = 0, r32i) iimageBuffer resultBuf;

void main()
{
    vec4 pix = texelFetch(trajFBO, ivec2(gl_FragCoord.xy), 0);
    int id = int(pix.r * MAXN);
    imageAtomicAdd(resultBuf, id-1, 1);
    FragColor=pix;
}