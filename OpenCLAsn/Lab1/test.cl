__kernel void test(
	__global uchar4* image,
	__global uchar4* output,
	const float time
	) {

	const int2 pos = { get_global_id(0), get_global_id(1) };
	int index = pos.x + pos.y*get_global_size(0);
	float2 res = { get_global_size(0), get_global_size(1)};
	float2 fragCoord = { (float)get_global_id(0) / res.x, (float)get_global_id(1) / res.y };
	float2 ratio = { res.x / res.y, 1.0f };
	float2 p = (2.0f * fragCoord.xy / res.xy - 1.0f) * ratio;
	float2 scaleFactor = {2.0f, 1.0f};
	float2 uv = { (atan(p.y / p.x) * 1.0f / 3.1416f) * scaleFactor.x, (1.0f / sqrt(dot(p,p))) * scaleFactor.y  } ;

	uv.x += sin(2.0f * uv.y + time * 0.5f);

	uchar4 c = image[(int)(uv.x * res.x) + (int)((uv.y)*get_global_size(0) * res.y)];
	//output[pos.x + pos.y*get_global_size(0)] = image[pos.x + pos.y*get_global_size(0)];
	//image[pos.x + pos.y*get_global_size(0)].x += sin(2.0f * image[pos.x + pos.y*get_global_size(0)].y + 0.000068 * 0.5);
	//output[pos.x + pos.y*get_global_size(0)] = image[pos.x + pos.y*get_global_size(0)] / (uv.y * 0.5 + 1.0);
	output[pos.x + pos.y*get_global_size(0)] = c;
}