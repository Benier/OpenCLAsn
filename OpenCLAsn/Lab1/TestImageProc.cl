// A simple kernel that swaps the red and green channels.

 

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST;

 

kernel void red_to_green(read_only image2d_t input, write_only image2d_t output)

{

    size_t x = get_global_id(0);

    size_t y = get_global_id(1);

 

    uint4 tap = read_imageui(input, sampler, (int2)(x,y));

    write_imageui(output, (int2)(x,y), tap.yxzw);

}