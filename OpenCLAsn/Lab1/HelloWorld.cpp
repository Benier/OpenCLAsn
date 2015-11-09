/////////////////////////////////////////////////////////////////////////////////
//
// Sample OpenCL application
// (c) 2015 Borna Noureddin
// British Columbia Institute of Technology
// Adapted from: OpenCL(R) Programming Guide
//      Authors: Aaftab Munshi, Benedict Gaster, Timothy Mattson, James Fung, Dan Ginsburg
//
/////////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SDL.h"
#include <chrono>

#ifdef __APPLE__
#include <sys/time.h>
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#include <Windows.h>

#undef main
/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

/*
* timezone information is stored outside the kernel so tzp isn't used anymore.
*
* Note: this function is not for Win32 high precision timing purpose. See
* elapsed_time().
*/
int
gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

	return 0;
}

#endif

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

char* CLErrorToString(cl_int error) {
	switch (error) {
	case CL_SUCCESS:                            return "Success!";
	case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
	case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
	case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
	case CL_OUT_OF_RESOURCES:                   return "Out of resources";
	case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
	case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
	case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
	case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
	case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
	case CL_MAP_FAILURE:                        return "Map failure";
	case CL_INVALID_VALUE:                      return "Invalid value";
	case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
	case CL_INVALID_PLATFORM:                   return "Invalid platform";
	case CL_INVALID_DEVICE:                     return "Invalid device";
	case CL_INVALID_CONTEXT:                    return "Invalid context";
	case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
	case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
	case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
	case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
	case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
	case CL_INVALID_SAMPLER:                    return "Invalid sampler";
	case CL_INVALID_BINARY:                     return "Invalid binary";
	case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
	case CL_INVALID_PROGRAM:                    return "Invalid program";
	case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
	case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
	case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
	case CL_INVALID_KERNEL:                     return "Invalid kernel";
	case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
	case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
	case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
	case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
	case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
	case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
	case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
	case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
	case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
	case CL_INVALID_EVENT:                      return "Invalid event";
	case CL_INVALID_OPERATION:                  return "Invalid operation";
	case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
	case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
	case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
	default: return "Unknown";
	}
}

// Helper class for timing calculations
class CTiming
{
public:
	CTiming() {}
	~CTiming() {}

	void Start() { start = std::chrono::high_resolution_clock::now(); }
	void End() { end = std::chrono::high_resolution_clock::now(); }
	void Seconds(double &seconds)
	{
		std::chrono::duration<double> elapsed = end - start;
		seconds = elapsed.count();
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
};

// Function to check return value of OpenCL calls and
// output custom error message to cerr
bool CheckOpenCLError(cl_int errNum, const char *errMsg)
{
	if (errNum != CL_SUCCESS)
	{
		std::cerr << errMsg << std::endl;
		return false;
	}
	return true;
}

//  Create an OpenCL context on the first available platform using
//  either a GPU or CPU depending on what is available.
cl_context CreateContext()
{
	// First, select an OpenCL platform to run on.  For this example, we
	// simply choose the first available platform.  Normally, you would
	// query for all available platforms and select the most appropriate one.
	cl_platform_id firstPlatformId;
	cl_uint numPlatforms;
	cl_int errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if (!CheckOpenCLError(errNum, "Failed to find any OpenCL platforms."))
		return NULL;
	if (numPlatforms <= 0)
	{
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}
	std::cout << std::endl << numPlatforms << " platforms in total" << std::endl;


	// Get information about the platform
	char pname[1024];
	size_t retsize;
	errNum = clGetPlatformInfo(firstPlatformId, CL_PLATFORM_NAME,
		sizeof(pname), (void *)pname, &retsize);
	if (!CheckOpenCLError(errNum, "Could not get platform info"))
		return NULL;
	std::cout << std::endl << "Selected platform <" << pname << ">" << std::endl;


	// Next, create an OpenCL context on the platform
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)firstPlatformId,
		0
	};
	cl_context context = NULL;
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL,
		NULL, NULL, &errNum);
	if (!CheckOpenCLError(errNum, "Failed to create an OpenCL GPU or CPU context."))
		return NULL;

	return context;
}

//  Create a command queue on the first device available on the context
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device, cl_device_type requestedDevice)
{
	// Get number of devices
	cl_int numDevices;
	size_t retSize;
	cl_int errNum = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(numDevices), (void *)&numDevices, &retSize);
	if (!CheckOpenCLError(errNum, "Could not get context info!"))
		return NULL;
	std::cout << std::endl << "There are " << numDevices << " devices." << std::endl;


	// Get list of devices
	cl_device_id *deviceList;
	deviceList = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, numDevices * sizeof(cl_device_id), (void *)deviceList, &retSize);
	if (!CheckOpenCLError(errNum, "Could not get device list!"))
	{
		std::cerr << " ERROR code " << errNum;
		switch (errNum) {
		case CL_INVALID_CONTEXT:
			std::cerr << " (CL_INVALID_CONTEXT)";
			break;
		case CL_INVALID_VALUE:
			std::cerr << " (CL_INVALID_VALUE)";
			break;
		case CL_OUT_OF_RESOURCES:
			std::cerr << " (CL_OUT_OF_RESOURCES)";
			break;
		case CL_OUT_OF_HOST_MEMORY:
			std::cerr << " (CL_OUT_OF_HOST_MEMORY)";
			break;
		default:
			break;
		}
		std::cerr << " size = " << numDevices * sizeof(cl_device_id) << ";" << retSize << std::endl;
		return NULL;
	}


	// Get device information for each device
	cl_device_type devType;
	int selectedDevice = 0;
	std::cout << std::endl << "Device list:" << std::endl;
	for (int i = 0; i<numDevices; i++)
	{

		std::cout << "   " << deviceList[i] << ": ";

		// device type
		errNum = clGetDeviceInfo(deviceList[i], CL_DEVICE_TYPE, sizeof(cl_device_type), (void *)&devType, &retSize);
		if (!CheckOpenCLError(errNum, "ERROR getting device info!"))
		{
			free(deviceList);
			return NULL;
		}
		std::cout << " type " << devType << ":";
		if (devType & CL_DEVICE_TYPE_CPU)
			std::cout << " CPU";
		if (devType & CL_DEVICE_TYPE_GPU)
			std::cout << " GPU";
		if (devType & CL_DEVICE_TYPE_ACCELERATOR)
			std::cout << " accelerator";
		if (devType & CL_DEVICE_TYPE_DEFAULT)
			std::cout << " default";

		if (devType == requestedDevice)
			selectedDevice = i;

		// device name
		char devName[1024];
		errNum = clGetDeviceInfo(deviceList[i], CL_DEVICE_NAME, 1024, (void *)devName, &retSize);
		if (!CheckOpenCLError(errNum, "ERROR getting device name!"))
		{
			free(deviceList);
			return NULL;
		}
		std::cout << " name=<" << devName << ">" << std::endl;

	}
	std::cout << std::endl;


	// In this example, we just choose the first available device.  In a
	// real program, you would likely use all available devices or choose
	// the highest performance device based on OpenCL device queries
	cl_command_queue commandQueue = clCreateCommandQueue(context, deviceList[0], 0, NULL);
	if (commandQueue == NULL)
	{
		free(deviceList);
		std::cerr << "Failed to create commandQueue for device 0";
		return NULL;
	}

	errNum = clGetDeviceInfo(deviceList[selectedDevice], CL_DEVICE_TYPE, sizeof(cl_device_type), (void *)&devType, &retSize);
	if (!CheckOpenCLError(errNum, "ERROR getting device info!"))
	{
		free(deviceList);
		return NULL;
	}

	if (devType != requestedDevice) // If the requested device isn't available
	{
		std::cerr << "Requested device:";

		if (requestedDevice & CL_DEVICE_TYPE_CPU)
			std::cout << " CPU";
		if (requestedDevice & CL_DEVICE_TYPE_GPU)
			std::cout << " GPU";
		if (requestedDevice & CL_DEVICE_TYPE_ACCELERATOR)
			std::cout << " accelerator";
		if (requestedDevice & CL_DEVICE_TYPE_DEFAULT)
			std::cout << " default";

		std::cout << " not available." << std::endl << std::endl;
	}

	*device = deviceList[selectedDevice];

	free(deviceList);

	return commandQueue;
}

//  Create an OpenCL program from the kernel source file
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
	cl_int errNum;
	cl_program program;

	std::ifstream kernelFile(fileName, std::ios::in);
	if (!kernelFile.is_open())
	{
		std::cerr << "Failed to open file for reading: " << fileName << std::endl;
		return NULL;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	program = clCreateProgramWithSource(context, 1,
		(const char**)&srcStr,
		NULL, NULL);
	if (program == NULL)
	{
		std::cerr << "Failed to create CL program from source." << std::endl;
		return NULL;
	}

	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(buildLog), buildLog, NULL);

		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(program);
		return NULL;
	}

	return program;
}

//  Cleanup any created OpenCL resources
void Cleanup(cl_context context, cl_command_queue commandQueue,
	cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
	for (int i = 0; i < 3; i++)
	{
		if (memObjects[i] != 0)
			clReleaseMemObject(memObjects[i]);
	}
	if (commandQueue != 0)
		clReleaseCommandQueue(commandQueue);

	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

	if (context != 0)
		clReleaseContext(context);
}

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;

	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

void RunSerialWater(SDL_Surface* loadedImage, SDL_Texture* texture, void* pixels, void* buffer, const float time)
{
	SDL_LockTexture(texture, NULL, &pixels, &loadedImage->pitch);

	memcpy(buffer, pixels, loadedImage->pitch * loadedImage->h);
	cl_char4 * output = (cl_char4 *)pixels;
	cl_char4 * input = (cl_char4 *)buffer;

	for (int x = 0; x < loadedImage->w; x++)
	{
		for (int y = 0; y < loadedImage->h; y++)
		{
			float resX = 1.0f;
			float resY = 1.0f;

			float fragCoordX = x / 256.0f;
			float fragCoordY = y / 256.0f;

			float uvX = fragCoordX / resX;
			float uvY = fragCoordY / resY;

			uvY += (cos((uvY + (time * 0.04f)) * 45.0f) * 0.0019f) +
				(cos((uvY + (time * 0.1f)) * 15.0f) * 0.002f);
			uvX += (sin((uvY + (time * 0.07f)) * 15.0f) * 0.0029f) +
				(sin((uvY + (time * 0.1f)) * 15.0f) * 0.002f);

			uvX = min(255.0f, uvX * 256.0f);
			uvY = min(255.0f, uvY * 256.0f);

			output[x + (y * loadedImage->w)] = input[((int)ceil(uvX)) + (((int)ceil(uvY)) * loadedImage->w)];
		}
	}
}

void RunSerialShader(SDL_Surface* loadedImage, SDL_Texture* texture, void* pixels, void* buffer, const float time)
{
	SDL_LockTexture(texture, NULL, &pixels, &loadedImage->pitch);

	memcpy(buffer, pixels, loadedImage->pitch * loadedImage->h);
	cl_char4 * output = (cl_char4 *)pixels;
	cl_char4 * input = (cl_char4 *)buffer;

	for (int x = 0; x < loadedImage->w; x++)
	{
		for (int y = 0; y < loadedImage->h; y++)
		{
			
			float resX = 1.0f;
			float resY = 1.0f;

			float fragCoordX = x / 256.0f;
			float fragCoordY = y / 256.0f;

			float pX = (2.0f * fragCoordX / resX) - 1.0f;
			float pY = (2.0f * fragCoordY / resY) - 1.0f;

			float ratioX = resX / resY;
			float ratioY = 1.0f;
			pX *= ratioX;
			pY *= ratioY;

			float uvX;
			if (pX == 0.0f)
			{
				uvX = atan(1.0f) * (1.0f / 3.14f);
			}
			else
			{
				uvX = atan(pY / pX) * (1.0f / 3.14f);
			}
			float uvY;
			if (pY == 0.0f)
			{
				uvY = 1.0f;
			}
			else
			{
				uvY = 1.0f / sqrt(((pX * pX) + (pY * pY)));
			}
			float scaleX = 2.0f;
			float scaleY = 1.0f;

			uvX *= scaleX;
			uvY *= scaleY;

			uvX += sin(((2.0f * uvY) + time) * 0.5f);

			uvX = min(255.0f, uvX * 128.0f);
			uvY = min(255.0f, uvY * 128.0f);

			output[x + (y * loadedImage->w)] = input[((int)uvX) + (((int)uvY) * loadedImage->w)];
		}
	}

	SDL_UnlockTexture(texture);
}

void RunOpenCL(cl_kernel kernel,
				cl_command_queue commandQueue,
				size_t* globalWorkSize,
				size_t* localWorkSize,
				cl_mem outputBuffer,
				SDL_Surface* loadedImage, 
				SDL_Texture *texture, 
				void* pixels, 
				const float time) {

	cl_int err = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL,
		globalWorkSize, localWorkSize,
		0, NULL, NULL);

	if (err != CL_SUCCESS)
	{
		CLErrorToString(err);
		return;
	}

	err = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE,
		0, loadedImage->pitch * loadedImage->h, pixels,
		0, NULL, NULL);

	if (err != CL_SUCCESS) {
		CLErrorToString(err);
		return;
	}

}

//	main() for HelloWorld example
int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Assignment 3", 100, 100, SCREEN_WIDTH,
		SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	cl_context context = 0;
	cl_command_queue gpuCommandQueue = 0;
	cl_command_queue cpuCommandQueue = 0;
	cl_program gpuProgram = 0;
	cl_program cpuProgram = 0;
	cl_device_id gpuDevice = 0;
	cl_device_id cpuDevice = 0;
	cl_kernel gpuKernel = 0;
	cl_kernel cpuKernel = 0;
	cl_int errNum;
	cl_mem* memObjects;

	// Create an OpenCL context on first available platform
	context = CreateContext();
	if (context == NULL)
	{
		std::cerr << "Failed to create OpenCL context." << std::endl;
		return 1;
	}

	{
		// Try and create a gpu command-queue
		gpuCommandQueue = CreateCommandQueue(context, &gpuDevice, CL_DEVICE_TYPE_GPU);
		if (gpuCommandQueue == NULL)
		{
			Cleanup(context, gpuCommandQueue, gpuProgram, gpuKernel, memObjects);
			return 1;
		}

		// Create OpenCL program from HelloWorld.cl kernel source
		gpuProgram = CreateProgram(context, gpuDevice, "rgbShift.cl");
		if (gpuProgram == NULL)
		{
			Cleanup(context, gpuCommandQueue, gpuProgram, gpuKernel, memObjects);
			return 1;
		}

		// Create OpenCL kernel
		gpuKernel = clCreateKernel(gpuProgram, "rgbShift", NULL);
		if (gpuKernel == NULL)
		{
			std::cerr << "Failed to create gpu kernel" << std::endl;
			Cleanup(context, gpuCommandQueue, gpuProgram, gpuKernel, memObjects);
			return 1;
		}
	}

	{
		// Try and create a cpu command-queue
		cpuCommandQueue = CreateCommandQueue(context, &cpuDevice, CL_DEVICE_TYPE_CPU);
		if (cpuCommandQueue == NULL)
		{
			Cleanup(context, cpuCommandQueue, cpuProgram, cpuKernel, memObjects);
			return 1;
		}

		// Create OpenCL program from HelloWorld.cl kernel source
		cpuProgram = CreateProgram(context, cpuDevice, "rgbShift.cl");
		if (cpuProgram == NULL)
		{
			Cleanup(context, cpuCommandQueue, cpuProgram, cpuKernel, memObjects);
			return 1;
		}

		// Create OpenCL kernel
		cpuKernel = clCreateKernel(cpuProgram, "rgbShift", NULL);
		if (cpuKernel == NULL)
		{
			std::cerr << "Failed to create gpu kernel" << std::endl;
			Cleanup(context, cpuCommandQueue, cpuProgram, cpuKernel, memObjects);
			return 1;
		}
	}

	// image setup
	const std::string file = "background3.bmp";
	SDL_Texture *texture = nullptr;
	SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
	loadedImage = SDL_ConvertSurface(loadedImage, SDL_GetWindowSurface(window)->format, NULL); // get the right format

	void* pixels = nullptr;
	void* buffer = malloc(loadedImage->pitch * loadedImage->h);

	texture = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_STREAMING, loadedImage->w, loadedImage->h);

	SDL_LockTexture(texture, NULL, &pixels, &loadedImage->pitch);
	memcpy(pixels, loadedImage->pixels, loadedImage->pitch * loadedImage->h);
	SDL_UnlockTexture(texture);

	cl_mem inputImage = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		loadedImage->pitch * loadedImage->h, pixels, NULL);

	cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
		loadedImage->pitch * loadedImage->h, NULL, NULL);

	cl_float time = 0;
	cl_int offsetGPU = 0;
	cl_int offsetCPU = loadedImage->h / 2;

	clSetKernelArg(gpuKernel, 0, sizeof(cl_mem), &inputImage);
	clSetKernelArg(gpuKernel, 1, sizeof(cl_mem), &outputBuffer);
	clSetKernelArg(gpuKernel, 2, sizeof(cl_int), &offsetGPU);

	clSetKernelArg(cpuKernel, 0, sizeof(cl_mem), &inputImage);
	clSetKernelArg(cpuKernel, 1, sizeof(cl_mem), &outputBuffer);
	clSetKernelArg(cpuKernel, 2, sizeof(cl_int), &offsetCPU);

	CTiming timer;
	double seconds;
	int frames = 0;
	char array[64];

	bool openCLRunning = true;
	char* type = openCLRunning ? "OpenCL" : "Serial";

	// Run Serially
	timer.Start();
	RunSerialWater(loadedImage, texture, pixels, buffer, time);
	timer.End();
	timer.Seconds(seconds);
	std::cout << "Serially" << " ran in " << seconds << " seconds" << std::endl << std::endl;

	// Run CPU + GPU
	size_t globalWorkSize[2] = { loadedImage->w, loadedImage->h / 2 };
	size_t localWorkSize[2] = { 16, 16 / 2 };
	SDL_LockTexture(texture, NULL, &pixels, &loadedImage->pitch);
	timer.Start();
	RunOpenCL(gpuKernel, gpuCommandQueue, globalWorkSize, localWorkSize, outputBuffer, loadedImage, texture, pixels, time);
	RunOpenCL(cpuKernel, cpuCommandQueue, globalWorkSize, localWorkSize, outputBuffer, loadedImage, texture, pixels, time);
	timer.End();
	timer.Seconds(seconds);
	std::cout << "OpenCL GPU + CPU" << " ran in " << seconds << " seconds" << std::endl << std::endl;
	SDL_UnlockTexture(texture);

	globalWorkSize[0] = loadedImage->w;
	globalWorkSize[1] = loadedImage->h;
	localWorkSize[0] = 16;
	localWorkSize[1] = 16;
	offsetCPU = 0;
	offsetGPU = 0;

	clSetKernelArg(gpuKernel, 2, sizeof(cl_int), &offsetGPU);
	clSetKernelArg(cpuKernel, 2, sizeof(cl_int), &offsetCPU);

	// Run CPU
	SDL_LockTexture(texture, NULL, &pixels, &loadedImage->pitch);
	timer.Start();
	RunOpenCL(cpuKernel, cpuCommandQueue, globalWorkSize, localWorkSize, outputBuffer, loadedImage, texture, pixels, time);
	timer.End();
	timer.Seconds(seconds);
	std::cout << "OpenCL CPU" << " ran in " << seconds << " seconds" << std::endl << std::endl;
	SDL_UnlockTexture(texture);

	// Run GPU
	SDL_LockTexture(texture, NULL, &pixels, &loadedImage->pitch);
	timer.Start();
	RunOpenCL(gpuKernel, gpuCommandQueue, globalWorkSize, localWorkSize, outputBuffer, loadedImage, texture, pixels, time);
	timer.End();
	timer.Seconds(seconds);
	std::cout << "OpenCL GPU" << " ran in " << seconds << " seconds" << std::endl << std::endl;
	SDL_UnlockTexture(texture);

	SDL_Event e;
	bool quit = false;
	while (!quit){
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT){
				quit = true;
			}
		}
		//Render the scene
		SDL_RenderClear(renderer);
		renderTexture(texture, renderer, 0, 0);
		SDL_RenderPresent(renderer);
	}

	SDL_FreeSurface(loadedImage);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	free(buffer);

	return 0;
}
