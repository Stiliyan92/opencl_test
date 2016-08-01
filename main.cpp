#include"OpenCLass.hpp"
#include<CL/cl.h>
#include"opencv2/highgui/highgui.hpp"
#include"opencv/cv.h"
#include<string>
#include <cmath>


const int ARRAY_SIZE = 32768;
void printMenu()
{
	std::cout << "PROGRAM MENU\n[0].EXIT\n[1].Print devices information\n[2].Select different devices.\n[3].Print platform information.\n[4].Run Gaussian Blur OPENCL example\n[5].Run sum of two vectors OPENCL.\n[6].Run Greyscale OPENCL example\n";
}
int setImageFormat(cl_uint type, cl_image_format& format)
{
	if (type < 8)
	{
		format.image_channel_order = CL_R;
		switch (type)
		{
			case 0: format.image_channel_data_type = CL_UNORM_INT8; break;
			case 1: format.image_channel_data_type = CL_SNORM_INT8; break;
			case 2: format.image_channel_data_type = CL_UNORM_INT16; break;
			case 3: format.image_channel_data_type = CL_SNORM_INT16; break;
			case 4: format.image_channel_data_type = CL_FLOAT; break;
		}
	}
	else
	{
		format.image_channel_order = CL_RGBA;
		switch (type)
		{
			case 24: format.image_channel_data_type = CL_UNORM_INT8; break;
			case 25: format.image_channel_data_type = CL_SNORM_INT8; break;
			case 26: format.image_channel_data_type = CL_UNORM_INT16; break;
			case 27: format.image_channel_data_type = CL_SNORM_INT16; break;
			case 28: format.image_channel_data_type = CL_FLOAT; break;
			default: return -1;
		}
	}
	return 0;
}
cl_program createProgram(cl_context context, cl_uint numDevices, cl_device_id* devices, const char* fileName)
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
		for (unsigned i = 0; i < numDevices; ++i)
		{
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
				sizeof(buildLog), buildLog, NULL);

			std::cerr << "Error in kernel: " << std::endl;
			std::cerr << buildLog;
		}
		clReleaseProgram(program);
		return NULL;
	}
	return program;
}
void Cleanup(cl_program program, cl_kernel kernel, cl_mem* memObjects, cl_uint memObjCount)
{
	if (memObjects != nullptr)
	{
		for (cl_uint i = 0; i < memObjCount; i++)
		{
			if (memObjects[i] != nullptr)
				clReleaseMemObject(memObjects[i]);
		}
	}
	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

}
size_t chooseLocalWorkSize(cl_kernel kernel, cl_device_id * devices, cl_uint devicesCount, size_t globalWorkSize)
{
	size_t localWorkSize = 0, min = 1000000;
	for (cl_uint i = 0; i < devicesCount; ++i)
	{
		clGetKernelWorkGroupInfo(kernel, devices[i], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &localWorkSize, NULL);
		std::cout << "Hardware specific size of wavefront(AMD) warp(nVIDIA) for device " << i << " is " << localWorkSize << "\n";
		if (min > localWorkSize)
			std::swap(min, localWorkSize);
	}
	std::cout << "The minimum warp(wavefront) for all devices in that context is  " << min << std::endl;
	std::cout << "Global work size is " << globalWorkSize << std::endl;
	std::cout << "Please select localWorkSize:\n";
	std::cin >> localWorkSize;
	return localWorkSize;
}
void measureTime(cl_event * events, cl_uint devCount, OpenCLass& workObj)
{
	cl_ulong startTime;
	cl_ulong endTime;
	for (cl_uint i = 0; i < devCount; ++i)
	{
		clWaitForEvents(1, &events[i]);
		clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
		clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
		cl_ulong totalTime = endTime - startTime;
		workObj.printDeviceNameAndVendor(workObj.getCurrentPlatform(), i, std::cout);
		std::cout << "Start time(in ns): " << startTime << " and in ms: " << startTime / 1000000;
		std::cout << std::endl << "End time(in ns): " << endTime << " and in ms: " << endTime / 1000000;
		std::cout << std::endl << "Total time(in ns): " << totalTime << " and in ms: " << totalTime / 1000000 << std::endl;
	}
}
int addArrays(OpenCLass& OCLass)
{
	cl_int errNum;
	cl_device_id * devices = OCLass.getDevices();
	cl_uint devicesCount = OCLass.getDevicesCount();
	cl_program addArrays = createProgram(OCLass.getContext(), devicesCount, devices, "addArrays.cl");
	if (addArrays == NULL)
	{
		std::cout << "Failed to create program.Exiting.\n";
		Cleanup(addArrays, nullptr, nullptr, 3);
		return 1;
	}
	cl_kernel kernel;
	kernel = clCreateKernel(addArrays, "addArrays_kernel", &errNum);
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Failed to create kernel" << std::endl;
		Cleanup(addArrays, kernel, nullptr, 3);
		return 1;
	}
	int a[ARRAY_SIZE];
	int b[ARRAY_SIZE];
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		a[i] = i;
		b[i] = i;
	}
	cl_mem memObjects[3];
	memObjects[0] = clCreateBuffer(OCLass.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(int) * ARRAY_SIZE, a, NULL);
	memObjects[1] = clCreateBuffer(OCLass.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(int) * ARRAY_SIZE, b, NULL);
	memObjects[2] = clCreateBuffer(OCLass.getContext(), CL_MEM_READ_WRITE,
		sizeof(int) * ARRAY_SIZE, NULL, NULL);

	if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
	{
		std::cerr << "Error creating memory objects." << std::endl;
		Cleanup(addArrays, kernel, memObjects, 3);
		return 1;
	}

	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

	//Selecting number of devices to use
	std::cout << "There are " << devicesCount << " available devices.How many would you like to use?\n";
	cl_uint devCount;
	std::cin >> devCount;

	while (devCount < 1 || devCount > devicesCount)
	{
		std::cout << "Please choose correct number:\n";
		std::cin >> devCount;
	}

	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error setting kernel arguments." << std::endl;
		Cleanup(addArrays, kernel, memObjects, 3);
		return 1;
	}

	size_t globalWorkSize = ARRAY_SIZE;
	size_t localWorkSize = chooseLocalWorkSize(kernel, devices, devicesCount, globalWorkSize);
	
	if (devCount == 1)
	{
		cl_event myEvent;
		int result[ARRAY_SIZE];
		errNum = clEnqueueNDRangeKernel(OCLass.getCommandQueue(0), kernel, 1, 0, &globalWorkSize, &localWorkSize, 0, NULL, &myEvent);
		if (errNum != CL_SUCCESS)
		{
			std::cerr << "Error queuing kernel for execution." << std::endl;
			Cleanup(addArrays, kernel, memObjects, 3);
			return 1;
		}
		errNum = clEnqueueReadBuffer(OCLass.getCommandQueue(0), memObjects[2], CL_TRUE, 0, globalWorkSize * sizeof(int), result, 0, NULL, NULL);
		if (errNum != CL_SUCCESS)
		{
			std::cerr << "Error reading result buffer.\n" << std::endl;
			Cleanup(addArrays, kernel, memObjects, 3);
			return 1;

		}
		for ( cl_uint i = 0; i < ARRAY_SIZE; ++i )
			std::cout << result[i] << " ";
		std::cout << std::endl;
		measureTime(&myEvent, devCount, OCLass);
		std::cout << "Executed program succesfully." << std::endl;
		Cleanup(addArrays, kernel, memObjects, 3);

	}
	else
	{ 
		cl_event * events = new cl_event[devCount];
		size_t global_work_offset = globalWorkSize / devCount;
		// Queue the kernel up for execution across the selected devices
		size_t lowerLimit, upperLimit;
		for (cl_uint i = 0; i < devCount; ++i)
		{
			lowerLimit = (global_work_offset*i);
			upperLimit = global_work_offset*(i + 1);
			if (i == (devCount - 1))
				upperLimit = globalWorkSize;
			errNum = clEnqueueNDRangeKernel(OCLass.getCommandQueue(i), kernel, 1, &lowerLimit, &upperLimit, &localWorkSize, 0, NULL, &events[i]);
			if (errNum != CL_SUCCESS)
			{
				std::cerr << "Error queuing kernel for execution.Error Number: " << errNum << std::endl;
				if(devCount % 2 == 1)
					std::cerr << "Try to use even number of devices.\n";
				Cleanup(addArrays, kernel, memObjects, 3);
				return 1;
			}

		}

		size_t remainder = globalWorkSize%devCount;
		remainder += global_work_offset;
		//initialize read buffers
		int** result = new int*[devCount];

		//read result buffers from gpu
		for (cl_uint i = 0; i < devCount; ++i)
		{
			lowerLimit = global_work_offset*i*sizeof(int);
			if (i < (devCount - 1))
			{
				result[i] = new int[global_work_offset];
				errNum = clEnqueueReadBuffer(OCLass.getCommandQueue(i), memObjects[2], CL_TRUE, lowerLimit, global_work_offset * sizeof(int), result[i], 0, NULL, NULL);
				if (errNum != CL_SUCCESS)
				{
					std::cerr << "Error reading result buffer.\n" << std::endl;
					Cleanup(addArrays, kernel, memObjects, 3);
					return 1;

				}
			}
			else
			{
				result[i] = new int[remainder];
				errNum = clEnqueueReadBuffer(OCLass.getCommandQueue(i), memObjects[2], CL_TRUE, lowerLimit, remainder * sizeof(int), result[i], 0, NULL, NULL);
				if (errNum != CL_SUCCESS)
				{
					std::cerr << "Error reading result buffer.\n" << std::endl;
					Cleanup(addArrays, kernel, memObjects, 3);
					return 1;
				}
			}
		}
		// Output result buffers
		for (cl_uint i = 0; i < devCount ; i++)

		{
			if (i == devCount - 1 && remainder != 0)
				global_work_offset = remainder;
			for (cl_uint j = 0; j < global_work_offset; ++j)
			{
				std::cout << result[i][j] << " ";
			}
		}
		
		measureTime(events, devCount, OCLass);
		std::cout << std::endl;
		std::cout << "Executed program succesfully." << std::endl;
		Cleanup(addArrays, kernel, memObjects, 3);

		//clear dynamic memory
		for (cl_uint i = 0; i < devCount; ++i)
		{
			delete result[i];
		}
		delete[] result;
	}
	
	return 0;
}
float * createBlurMask(float sigma, int * maskSizePointer)
{
	int maskSize = (int)ceil(3.0f*sigma);
	float * mask = new float[(maskSize * 2 + 1)*(maskSize * 2 + 1)];
	float sum = 0.0f;
	for (int a = -maskSize; a < maskSize + 1; a++) {
		for (int b = -maskSize; b < maskSize + 1; b++) {
			float temp = exp(-((float)(a*a + b*b) / (2 * sigma*sigma)));
			sum += temp;
			mask[a + maskSize + (b + maskSize)*(maskSize * 2 + 1)] = temp;
		}
	}
	// Normalize the mask
	for (int i = 0; i < (maskSize * 2 + 1)*(maskSize * 2 + 1); i++)
		mask[i] = mask[i] / sum;

	*maskSizePointer = maskSize;

	return mask;


}
int GaussianBlur(OpenCLass& OCLass)
{
	cl_int errNum;
	cl_device_id * devices = OCLass.getDevices();
	cl_uint devicesCount = OCLass.getDevicesCount();
	std::string imageFile;
	std::cout << "Please, enter image filename:\n";
	std::cin >> imageFile;
	cv::Mat img = cv::imread(imageFile, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
//	cv::Mat img = cv::imread("R9.jpg", CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	if (img.empty())
		
	{
		std::cout << "Couldn't load the image\n";
		return 1;
	}
	cv::Mat transformedImg;
	cv::cvtColor(img, transformedImg, CV_BGR2RGBA);
	//defining image format and desc
	cl_image_format inFormat, outFormat;
	//cl_image_desc desc;
	setImageFormat(transformedImg.type(), inFormat);
	//desc.image_width = transformedImg.cols;
	//desc.image_height = transformedImg.rows;

	size_t globalWorkSize = transformedImg.cols * transformedImg.rows;
	size_t arrSize = globalWorkSize * 4;
	// Compile OpenCL code
	cl_program gaussianBlur = createProgram(OCLass.getContext(), devicesCount, devices, "GaussianBlur.cl");

	cl_mem memoryObjects[3];
	// Create an OpenCL Image / texture and transfer data to the device
	memoryObjects[0] = clCreateImage2D(OCLass.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &inFormat, transformedImg.cols, transformedImg.rows, transformedImg.cols * 4, transformedImg.data, &errNum);
	if (errNum != 0)
	{

		std::cout << "Error creating Image memory object.Error number: " << errNum << std::endl;
		Cleanup(gaussianBlur, NULL, memoryObjects, 1);
		return 1;
	}
	// Create Gaussian mask
	float sigma;
	std::cout << "Please choose sigma(higher number means more blurred effect because of bigger mask)\n";
	std::cin >> sigma;
	int maskSize;
	float * mask = createBlurMask(sigma, &maskSize);

	// Create buffer for mask and transfer it to the device
	memoryObjects[1] = clCreateBuffer(OCLass.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*(maskSize * 2 + 1)*(maskSize * 2 + 1), mask, &errNum);
	if (errNum != 0)
	{
		std::cout << "Error creating mask memory object.Error number: " << errNum << std::endl;
		Cleanup(gaussianBlur, NULL, memoryObjects, 2);
		return 1;
	}
	float * data = new float[arrSize];
	// Create a buffer for the result
	outFormat.image_channel_data_type = CL_FLOAT; 
	outFormat.image_channel_order = CL_RGBA;
	memoryObjects[2] = clCreateImage2D(OCLass.getContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &outFormat, transformedImg.cols, transformedImg.rows, transformedImg.cols * 16, data, &errNum);
	if (errNum != 0)
	{
		std::cout << "Error creating read back output IMAGE memory object.Error number: " << errNum << std::endl;
		Cleanup(gaussianBlur, NULL, memoryObjects, 3);
		return 1;
	}
	
	// Run Gaussian kernel
	cl_kernel kernel = clCreateKernel(gaussianBlur, "gaussian_blur", &errNum);
	if (errNum != 0)
	{
		std::cout << "Error creating kernel.Error number: " << errNum << std::endl;
		Cleanup(gaussianBlur, kernel, memoryObjects, 3);
		return 1;
	}

	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memoryObjects[0]);
	errNum = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memoryObjects[1]);
	errNum = clSetKernelArg(kernel, 2, sizeof(cl_mem), &memoryObjects[2]);
	errNum = clSetKernelArg(kernel, 3, sizeof(int), (float*)&maskSize);
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error setting kernel Arguments.\n" << std::endl;
		Cleanup(gaussianBlur, kernel, memoryObjects, 3);
		return 1;
	}
	cl_event myEvent;
	size_t glWork[2] = { transformedImg.cols, transformedImg.rows };
	size_t lWork[2] = { 8, 8 };
	errNum = clEnqueueNDRangeKernel(OCLass.getCommandQueue(0), kernel, 2, 0, glWork,lWork, 0, NULL, &myEvent);
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error queuing kernel for execution.Error number: " << errNum <<  std::endl;
		Cleanup(gaussianBlur, kernel, memoryObjects, 3);
		return 1;
	}
	// Transfer image back to host
	errNum = clFinish(OCLass.getCommandQueue(0));
	if (errNum != 0)
	{
		std::cout << "Error while waiting for queue to finish work\n";
	
	}
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { transformedImg.cols, transformedImg.rows, 1 };
	/*
	cl::size_t<3> origin;
	origin[0] = 0; origin[1] = 0, origin[2] = 0;
	cl::size_t<3> region;
	region[0] = transformedImg.cols; region[1] = transformedImg.rows; region[2] = 1;
	*/
	errNum = clEnqueueReadImage(OCLass.getCommandQueue(0), memoryObjects[2], CL_TRUE, origin, region, transformedImg.cols*16, 0 , data, 0,  NULL, NULL);
	if(errNum != 0)
	{
		std::cout << "Error reading back result.Error number: " << errNum << "\n";
		return 1;
	}
	measureTime(&myEvent, 1, OCLass);
	for (int i = 0; i < arrSize; ++i)
	{
		data[i] *= 255;
	}
	
	//create cv::Size object for creating a Mat object with proper size
	cv::Size size;
	size.height = transformedImg.rows;
	size.width = transformedImg.cols;
	//create cv::Mat object with data

	cv::Mat outputImageRGBA(size, CV_32FC4, data);

	cv::Mat outputImageBGR;
	//convert to BGR because imwrite doc: "Only 8-bit (or 16-bit unsigned (CV_16U) in case of PNG, JPEG 2000, and TIFF) single-channel or 3-channel (with ‘BGR’ channel order) images can be saved using this function."
	cv::cvtColor(outputImageRGBA, outputImageBGR, CV_RGBA2BGR);
	//save to disc with imwrite
	std::cout << "Please enter file name for blurred image(ending with .jpg or .png or another known image file type extension):\n";
	std::cin >> imageFile;
	std::cout << std::endl << cv::imwrite(imageFile, outputImageBGR) << std::endl;
//	std::cout << std::endl << cv::imwrite("blurredImage.jpg", outputImageBGR) << std::endl;
	Cleanup(gaussianBlur, kernel, memoryObjects, 3);
	delete[] data;
	return 0;
}
int GreyScaleImage(OpenCLass & OCLass)
{
	cl_int errNum;
	cl_device_id * devices = OCLass.getDevices();
	cl_uint devicesCount = OCLass.getDevicesCount();
	std::string imageFile;
	std::cout << "Please, enter image filename:\n";
	std::cin >> imageFile;
	cv::Mat img = cv::imread(imageFile, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	//cv::Mat img = cv::imread("R9.jpg", CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	if (img.empty())

	{
		std::cout << "Couldn't load the image\n";
		return 1;
	}
	cv::Mat transformedImg;
	cv::cvtColor(img, transformedImg, CV_BGR2RGBA);
	//defining image format and desc
	cl_image_format inFormat, outFormat;
	setImageFormat(transformedImg.type(), inFormat);

	size_t globalWorkSize = transformedImg.cols * transformedImg.rows;
	size_t arrSize = globalWorkSize * 4;
	cl_program greyScale = createProgram(OCLass.getContext(), devicesCount, devices, "GreyScale.cl");

	cl_mem memoryObjects[2];
	// Create an OpenCL Image / texture and transfer data to the device
	memoryObjects[0] = clCreateImage2D(OCLass.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &inFormat, transformedImg.cols, transformedImg.rows, transformedImg.cols * 4, transformedImg.data, &errNum);
	if (errNum != 0)
	{

		std::cout << "Error creating Image memory object.Error number: " << errNum << std::endl;
		Cleanup(greyScale, NULL, memoryObjects, 1);
		return 1;
	}
	float * data = new float[globalWorkSize];
	// Create a buffer for the result
	outFormat.image_channel_data_type = CL_FLOAT;
	outFormat.image_channel_order = CL_R;
	memoryObjects[1] = clCreateImage2D(OCLass.getContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &outFormat, transformedImg.cols, transformedImg.rows, transformedImg.cols * 4, data, &errNum);
	if (errNum != 0)
	{
		std::cout << "Error creating read back output IMAGE memory object.Error number: " << errNum << std::endl;
		Cleanup(greyScale, NULL, memoryObjects, 2);
		return 1;
	}

	// Run Gaussian kernel
	cl_kernel kernel = clCreateKernel(greyScale, "greyScale", &errNum);
	if (errNum != 0)
	{
		std::cout << "Error creating kernel.Error number: " << errNum << std::endl;
		Cleanup(greyScale, kernel, memoryObjects, 2);
		return 1;
	}

	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memoryObjects[0]);
	errNum = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memoryObjects[1]);
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error setting kernel Arguments.\n" << std::endl;
		Cleanup(greyScale, kernel, memoryObjects, 2);
		return 1;
	}
	cl_event myEvent;
	size_t glWork[2] = {transformedImg.cols, transformedImg.rows};
	//size_t lWork[2] = { 16, 16 };
	errNum = clEnqueueNDRangeKernel(OCLass.getCommandQueue(0), kernel, 2, 0, glWork, NULL, NULL , NULL, &myEvent);
	if (errNum != CL_SUCCESS)
	{


		std::cerr << "Error queuing kernel for execution." << std::endl;
		Cleanup(greyScale, kernel, memoryObjects, 2);
		return 1;
	}
	// Transfer image back to host
	errNum = clFinish(OCLass.getCommandQueue(0));
	if (errNum != 0)
	{
		std::cout << "Error while waiting for queue to finish work\n";
	}
	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {transformedImg.cols, transformedImg.rows, 1};
	/*
	cl::size_t<3> origin;
	origin[0] = 0; origin[1] = 0, origin[2] = 0;
	cl::size_t<3> region;
	region[0] = transformedImg.cols; region[1] = transformedImg.rows; region[2] = 1;
	*/
	errNum = clEnqueueReadImage(OCLass.getCommandQueue(0), memoryObjects[1], CL_TRUE, origin, region, transformedImg.cols * 4, 0, data, 0, NULL, NULL);
	if (errNum != 0)
	{
		std::cout << "Error reading back result.Error number: " << errNum << "\n";
		return 1;
	}
	measureTime(&myEvent, 1, OCLass);
	for (int i = 0; i < globalWorkSize; ++i)
	{
		data[i] *= 255;
	}

	//create cv::Size object for creating a Mat object with proper size
	cv::Size size;
	size.height = transformedImg.rows;
	size.width = transformedImg.cols;
	//create cv::Mat object with data

	cv::Mat outputImageGreyscale(size, CV_32FC1, data);

	//cv::Mat outputImageBGR;
	//convert to BGR because imwrite doc: "Only 8-bit (or 16-bit unsigned (CV_16U) in case of PNG, JPEG 2000, and TIFF) single-channel or 3-channel (with ‘BGR’ channel order) images can be saved using this function."
	//cv::cvtColor(outputImageGreyscale, outputImageBGR, CV_RGBA2BGR);
	//save to disc with imwrite
	std::cout << "Please enter file name for blurred image(ending with .jpg or .png or another known image file type extension):\n";
	std::cin >> imageFile;
	std::cout << std::endl << cv::imwrite(imageFile, outputImageGreyscale) << std::endl;
//	std::cout << std::endl << cv::imwrite("greyscaled.jpg", outputImageGreyscale) << std::endl;
	Cleanup(greyScale, kernel, memoryObjects, 2);
	delete[] data;
	return 0;
}
int main()
{
	int key = 1;
	OpenCLass workClass;
	workClass.init();
	printMenu();
	std::cin >> key;
	while (key != 0)
	{
		switch (key)
		{
		case 0: break;
		case 1: workClass.printAllDevices(); break;
		case 2: workClass.selectDifferentDevices(); break;
		case 3: workClass.printPlatformInformation();break; 
		case 4: GaussianBlur(workClass); break;
		case 5: addArrays(workClass); break;
		case 6:GreyScaleImage(workClass); break;
		}
			printMenu(); 
			std::cin >> key;
	}
	return 0;
}
/*What if there are multiple devices within the context? clCreateBuffer only takes a context as a parameter, not a command queue associated with a specific device. I've been told on Nvidia forums that clCreateBuffer with CL_MEM_COPY_HOST_PTR will copy the data to ALL devices associated with that context. So if you're only using a single device, you might be better off manually enqueuing a write to this particular device because if a user's machine has several GPUs, he will pay for several memcopies over PCIE. Is this correct?*/
/*Choose first case if you want create buffer and initialize only once in whole program.   Use clEnqueueWriteBuffer if you want to update the buffer values with new data set more than once.*/
