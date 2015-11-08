#define PI 3.1416

__kernel void water(
	__global uchar4* image,
	__global uchar4* output,
	const float time
	) {

	float2 resolution = { 1.0f, 1.0f };
	float2 position = { get_global_id(0), get_global_id(1) };
	float2 fragCoord = { position.x / 256.0f, position.y / 256.0f };

	float2 uv = fragCoord.xy / resolution.xy;

	uv.y += (cos((uv.y + (time * 0.04f)) * 45.0f) * 0.0019f) +
		(cos((uv.y + (time * 0.1f)) * 15.0f) * 0.002f);
	uv.x += (sin((uv.y + (time * 0.07f)) * 15.0f) * 0.0029f) +
		(sin((uv.y + (time * 0.1f)) * 15.0f) * 0.002f);

	uv.x = min(255.0f, uv.x * 256.0f);
	uv.y = min(255.0f, uv.y * 256.0f);

	uchar4 c = image[((int)(uv.x)) + (((int)(uv.y))*get_global_size(0))]; 
	output[(int)position.x + ((int)position.y)*get_global_size(0)] = c;
}