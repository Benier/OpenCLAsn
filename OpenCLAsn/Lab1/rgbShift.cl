
__kernel void rgbShift(
	__global uchar4* image,
	__global uchar4* output,
	const int offset
	) {
	float xOffset = 0.04;
	float yOffset = 0.02;

	uchar4 color;
	float2 position = { get_global_id(0), get_global_id(1) + offset };
	float2 fragCoord = { position.x / 1920.0f, position.y / 1080.0f };

	xOffset = xOffset * sin(20.0f);
	yOffset = yOffset * cos(20.0f);

	float2 rFract = { fragCoord.x + xOffset, -fragCoord.y + yOffset };
	float2 rCoords = { fmin(rFract.x - floor(rFract.x), 1.0f), fmin(rFract.y - floor(rFract.y), 1.0f) };
	rCoords.x = min(1919.0f, rCoords.x * 1920.0f);
	rCoords.y = min(1079.0f, rCoords.y * 1080.0f);
	color.x = image[((int)(rCoords.x)) + (((int)(rCoords.y))*get_global_size(0))].x;

	float2 gFract = { fragCoord.x + 0.000, -fragCoord.y };
	float2 gCoords = { fmin(gFract.x - floor(gFract.x), 1.0f), fmin(gFract.y - floor(gFract.y), 1.0f) };
	gCoords.x = min(1919.0f, gCoords.x * 1920.0f);
	gCoords.y = min(1079.0f, gCoords.y * 1080.0f);
	color.y = image[((int)(gCoords.x)) + (((int)(gCoords.y))*get_global_size(0))].y;

	float2 bFract = { fragCoord.x - xOffset, -fragCoord.y - yOffset };
	float2 bCoords = { fmin(bFract.x - floor(bFract.x), 1.0f), fmin(bFract.y - floor(bFract.y), 1.0f) };
	bCoords.x = min(1919.0f, bCoords.x * 1920.0f);
	bCoords.y = min(1079.0f, bCoords.y * 1080.0f);
	color.z = image[((int)(bCoords.x)) + (((int)(bCoords.y))*get_global_size(0))].z;

	color.w = image[(int)position.x + ((int)position.y)*get_global_size(0)].w;

	output[(int)position.x + ((int)position.y)*get_global_size(0)] = color;
}