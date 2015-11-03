__kernel void test(
	__global uchar4* image,
	__global uchar4* output
	) {

	const int2 pos = { get_global_id(0), get_global_id(1) };
	int index = pos.x + pos.y*get_global_size(0);

	//float2 p = (2.0 * image[pos.x + pos.y*get_global_size(0)].xy / image[pos.x + pos.y*get_global_size(0)].xy - 1.0) * float2(image[pos.x + pos.y*get_global_size(0)].x / image[pos.x + pos.y*get_global_size(0)].y, 1.0);

	//float2 uv = float2(atan(p.y, p.x) * 1.0 / 3.1416, 1.0 / sqrt(dot(p,p))) * float2(2.0, 1.0);

	//uv.x += sin(2.0 * uv.y + 0.000006 * 0.5);

	output[pos.x + pos.y*get_global_size(0)] = image[pos.x + pos.y*get_global_size(0)];
	//image[pos.x + pos.y*get_global_size(0)].x += sin(2.0 * image[pos.x + pos.y*get_global_size(0)].y + 0.000068 * 0.5);
	//output[pos.x + pos.y*get_global_size(0)] = image[pos.x + pos.y*get_global_size(0)] / (uv.y * 0.5 + 1.0);
}