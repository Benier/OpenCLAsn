__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void test(
	__global uint4* image,
	__global uint4* output
	) {

	const int2 pos = { get_global_id(0), get_global_id(1) };

	output[pos.x + pos.y*get_global_size(0)].x = 0;
	output[pos.x + pos.y*get_global_size(0)].y = 150;
	output[pos.x + pos.y*get_global_size(0)].z = 0;
	output[pos.x + pos.y*get_global_size(0)].w = 150;
}