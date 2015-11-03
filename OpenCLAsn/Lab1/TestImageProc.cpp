/*#include <stdio.h>

#include <stdlib.h>

#include <CL/cl.h>



// Include the automatically-generated header which provides the kernel block

// declaration.

#include "kernels.cl.h"


#define COUNT 2048



static void display_device(cl_device_id device)

{

	char name_buf[128];

	char vendor_buf[128];



	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(char)* 128, name_buf, NULL);

	clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(char)* 128, vendor_buf, NULL);



	fprintf(stdout, "Using OpenCL device: %s %s\n", vendor_buf, name_buf);

}



static void image_test(const dispatch_queue_t dq)

{

	// This example uses a dispatch semaphore to achieve synchronization

	// between the host application and the work done for us by the OpenCL device.

	dispatch_semaphore_t dsema = dispatch_semaphore_create(0);



	// This example creates a "fake" RGBA, 8-bit-per channel image, solid red.

	// In a real program, you would use some real raster data.

	// Most OpenCL devices support a wide variety of image formats.



	unsigned int i;

	size_t height = 2048, width = 2048;



	unsigned int *pixels =

		(unsigned int*)malloc(sizeof(unsigned int)* width * height);



	for (i = 0; i < width*height; i++)

		pixels[i] = 0xFF0000FF; // 0xAABBGGRR: 8bits per channel, all red.



	// This image data is on the host side.

	// You need to create two OpenCL images in order to perform some

	// manipulations: one for the input and one for the ouput.



	// This describes the format of the image data.

	cl_image_format format;

	format.image_channel_order = CL_RGBA;

	format.image_channel_data_type = CL_UNSIGNED_INT8;



	cl_mem input_image = gcl_create_image(&format, width, height, 1, NULL);

	cl_mem output_image = gcl_create_image(&format, width, height, 1, NULL);



	dispatch_async(dq, ^{



		// This kernel is written such that each work item processes one pixel.

		// Thus, it executes over a two-dimensional range, with the width and

		// height of the image determining the dimensions

		// of execution.



		cl_ndrange range = {

			2,                  // Using a two-dimensional execution.

			{ 0 },                // Start at the beginning of the range.

			{ width, height },    // Execute width * height work items.

			{ 0 }                 // And let OpenCL decide how to divide

			// the work items into work-groups.

		};



		// Copy the host-side, initial pixel data to the image memory object on

		// the OpenCL device.  Here, we copy the whole image, but you could use

		// the origin and region parameters to specify an offset and sub-region

		// of the image, if you'd like.

		const size_t origin[3] = { 0, 0, 0 };

		const size_t region[3] = { width, height, 1 };

		gcl_copy_ptr_to_image(input_image, pixels, origin, region);



		// Do it!

		red_to_green_kernel(&range, input_image, output_image);



		// Read back the results; then reuse the host-side buffer we

		// started with.

		gcl_copy_image_to_ptr(pixels, output_image, origin, region);



		// Let the host know we're done.

		dispatch_semaphore_signal(dsema);

	});



	// Do other work, if you'd like...



	// ... but eventually, you will want to wait for OpenCL to finish up.

	dispatch_semaphore_wait(dsema, DISPATCH_TIME_FOREVER);



	// We expect '0xFF00FF00' for each pixel.

	// Solid green, all the way.

	int results_ok = 1;

	for (i = 0; i < width*height; i++) {

		if (pixels[i] != 0xFF00FF00) {

			fprintf(stdout,

				"Oh dear. Pixel %d was not correct.

				Expected 0xFF00FF00, saw %x\n",

				i, pixels[i]);

			results_ok = 0;

			break;

		}

	}



	if (results_ok)

		fprintf(stdout, "Image results OK!\n");



	// Clean up device-size allocations.

	// Note that we use the "standard" OpenCL API here.

	clReleaseMemObject(input_image);

	clReleaseMemObject(output_image);



	// Clean up host-side allocations.

	free(pixels);

}



int main(int argc, const char * argv[])

{

	// Grab a CPU-based dispatch queue.

	dispatch_queue_t dq = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);

	if (!dq)

	{

		fprintf(stdout, "Unable to create a CPU-based dispatch queue.\n");

		exit(1);

	}



	// Display the OpenCL device associated with this dispatch queue.

	display_device(gcl_get_device_id_with_dispatch_queue(dq));



	image_test(dq);



	fprintf(stdout, "\nDone.\n\n");



	dispatch_release(dq);

}*/