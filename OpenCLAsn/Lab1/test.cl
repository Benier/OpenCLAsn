__kernel void test(
	__global uchar4* image,
	__global uchar4* output
	) {

	const int2 pos = { get_global_id(0), get_global_id(1) };
	int index = pos.x + pos.y*get_global_size(0);

	output[pos.x + pos.y*get_global_size(0)] = image[pos.x + pos.y*get_global_size(0)];
}