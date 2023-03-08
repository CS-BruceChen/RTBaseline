#version 460 core
out vec4 FragColor;
uniform sampler2D trajFBO;

uniform float MAXN;

uniform layout(binding = 0, r32i) iimageBuffer resultBuf;

void main()
{
    //pix r is right 
    vec4 pix = texelFetch(trajFBO, ivec2(gl_FragCoord.xy), 0);//pix值正确
    int id = int(pix.r * (MAXN-1));//问题可能在类型转换或者uniform的值
    imageAtomicAdd(resultBuf, id, 1);
    if(1<MAXN && MAXN<1.5){
        FragColor=vec4(0,0,0,1);
    }
    else if(pix.r<0.6){
        FragColor=pix;
    }
    else FragColor=vec4(0,0,0,1);
    //float val = id/(MAXN-1);
    
    
}