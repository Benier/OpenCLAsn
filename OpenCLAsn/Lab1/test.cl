#define PI 3.1416

__kernel void test(
	__global uchar4* image,
	__global uchar4* output,
	const float time
	) {

	float2 resolution = { 1.0f, 1.0f };
	float2 position = { get_global_id(0), get_global_id(1) };
	float2 fragCoord = { position.x / 256.0f, position.y / 256.0f };

	//map the xy pixel co-ordinates to be between -1.0 to +1.0 on x and y axes
	//and alter the x value according to the aspect ratio so it isn't 'stretched'
	float2 p =  2.0f * fragCoord.xy / resolution.xy - 1.0f ;
	float2 ratio = { resolution.x / resolution.y, 1.0f };
	p = p * ratio;

	//now, this is the usual part that uses the formula for texture mapping a ray-
	//traced cylinder using the vector p that describes the position of the pixel
	//from the centre.
	float2 uv = { atan(p.y / p.x) * 1.0f / 3.14f, 1.0f / sqrt(dot(p, p)) };
	float2 scale = { 2.0f, 1.0f };
	uv = uv * scale;


	//now this just 'warps' the texture read by altering the u coordinate depending on
	//the val of the v coordinate and the current time
	uv.x += sin(2.0f * uv.y + time * 0.5f);

	uv.x = min(255.0f, uv.x * 128.0f);
	uv.y = min(255.0f, uv.y * 128.0f);

	uchar4 c = image[((int)(uv.x)) + (((int)(uv.y))*get_global_size(0))]; 
	output[(int)position.x + ((int)position.y)*get_global_size(0)] = c;
}