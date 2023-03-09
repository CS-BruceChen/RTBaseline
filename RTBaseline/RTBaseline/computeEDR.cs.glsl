#version 460 core

struct DPInfo {
	uint wd;
	uint ht;
	bool isWdBiggerThanHt;
	uint min_wd_ht;
	uint d_w_h;
	uint total_step;
};
const float epsl = 1.0;
///DPInfo
uniform DPInfo dpinfo;
uniform uint step_now;
layout(binding = 0, r32i) uniform iimageBuffer DPBuf;
layout(binding = 1, r32f) uniform imageBuffer APointsBuf_X;
layout(binding = 2, r32f) uniform imageBuffer APointsBuf_Y;
layout(binding = 3, r32f) uniform imageBuffer BPointsBuf_X;
layout(binding = 4, r32f) uniform imageBuffer BPointsBuf_Y;
//declare the local size
layout (local_size_x = 1024, local_size_y = 1) in;

bool isInWorkingRange(){
	uint arrayId = gl_LocalInvocationID.x + 1;
	uint min_bound = 1;
	uint max_bound = dpinfo.min_wd_ht;
	if(step_now<=dpinfo.min_wd_ht){
		min_bound = 1;
		max_bound = step_now;
	}
	else if(step_now<=dpinfo.min_wd_ht+dpinfo.d_w_h){
		min_bound = 1;
		max_bound = dpinfo.min_wd_ht;
	}
	else if(step_now<=dpinfo.total_step){
		min_bound = 1 + step_now - dpinfo.min_wd_ht - dpinfo.d_w_h;
		max_bound = dpinfo.min_wd_ht;
	}
	
	if(min_bound <= arrayId && arrayId <= max_bound) return true;
	else return false;
}

int minVal(int a,int b,int c){
	if(a<=b && a<=c) return a;
	else if(b<=a && b<=c) return b;
	else if(c<=a && c<=b) return c;
	
	return 0;
}

int subcost(uint A,uint B){//减一是因为点数组下标从0开始
	vec2 PointA = vec2(imageLoad(APointsBuf_X,int(A-1)).x,imageLoad(APointsBuf_Y,int(A-1)).x);
	vec2 PointB = vec2(imageLoad(BPointsBuf_X,int(B-1)).x,imageLoad(BPointsBuf_Y,int(B-1)).x);
	if( abs(PointA.x - PointB.x) <= epsl && abs(PointA.y - PointB.y) <= epsl) return 0;
	else return 1;
}

int bufferIndex(uint x,uint y){
	int W = int(dpinfo.wd + 1);//因为buf实际上比wd大一圈
	return int(x + y * W);
}

void slashDP(){
	bool isLocalIDinRange = isInWorkingRange();
	uint tid = gl_LocalInvocationID.x;
	if(isLocalIDinRange) {
		uint x = dpinfo.isWdBiggerThanHt ? (step_now - tid) : (1 + tid);
		uint y = dpinfo.isWdBiggerThanHt ? (1 + tid) : (step_now - tid);
		int val = minVal(
			imageLoad(DPBuf,bufferIndex(x-1,y)).x + 1,
			imageLoad(DPBuf,bufferIndex(x,y-1)).x + 1,
			imageLoad(DPBuf,bufferIndex(x-1,y-1)).x + subcost(x,y)
		);
		imageAtomicExchange(DPBuf,bufferIndex(x,y),val);
	}
}

void main(){
	slashDP();
}