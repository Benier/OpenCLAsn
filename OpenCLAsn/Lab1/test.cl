__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void test(
	__read_only image2d_t image,
	__global float * output
	) {

	const int2 pos = { get_global_id(0), get_global_id(1) };

	output[pos.x + pos.y*get_global_size(0)] = read_imagef(image, sampler, pos).z/2.0;
}