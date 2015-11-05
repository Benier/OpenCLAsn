__kernel void test(
	__global uchar4* image,
	__global uchar4* output,
	const float time
	) {

	int2 pos = { get_global_id(0), get_global_id(1) };
	float2 res = { 1.0f, 1.0f };
	float2 fragCoord = { ((float)get_global_id(0)) / get_global_size(0), ((float)get_global_id(1)) / get_global_size(1) };

	//map the xy pixel co-ordinates to be between -1.0 to +1.0 on x and y axes
	//and alter the x value according to the aspect ratio so it isn't 'stretched'
	float2 ratio = { res.x / res.y, 1.0f };
	float2 p = (2.0f * fragCoord.xy / res.xy - 1.0f) * ratio;

	//now, this is the usual part that uses the formula for texture mapping a ray-
	//traced cylinder using the vector p that describes the position of the pixel
	//from the centre.
	float2 scaleFactor = { 2.0f, 1.0f };
	float2 uv = { (atan(p.y / p.x) * (1.0f / 3.1416f)) * scaleFactor.x, (1.0f / sqrt(dot(p, p))) * scaleFactor.y };

	//now this just 'warps' the texture read by altering the u coordinate depending on
	//the val of the v coordinate and the current time
	uv.x += sin((2.0 * uv.y) + (time * 0.5));

	uv.x = min(res.x, uv.x * res.x);
	uv.y = min(res.y, uv.y * res.y);

	uchar4 c = image[((int)(uv.x)) + (((int)(uv.y))*get_global_size(0))]; /// (uv.y * 0.5f + 1.0f);

	output[pos.x + pos.y*get_global_size(0)] = c;
}