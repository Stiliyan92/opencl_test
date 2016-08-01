#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include"OpenCLass.hpp"
#include<fstream>
void OpenCLass::deleteHelper()
{
	if (context != 0)
		clReleaseContext(context);
	while (dev[currentPlatform].numOfDevices--)
	{
		if (commandQueues[dev[currentPlatform].numOfDevices] != nullptr)
			clReleaseCommandQueue(commandQueues[dev[currentPlatform].numOfDevices]);
	}
	if (platforms != nullptr)
	{
		delete[] platforms;
	}

	if (dev != nullptr)
	{

		for (unsigned i = 0; i < numOfPlatforms; ++i)
		{
			if (dev[i].device != nullptr)
				delete[] dev[i].device;
		}
		delete[] dev;
	}
	if (commandQueues != nullptr)
	{
		delete[] commandQueues;
	}
}
void OpenCLass::copyHelper(const OpenCLass & rhs)
{
	this->numOfPlatforms = rhs.numOfPlatforms;
	this->platforms = new cl_platform_id[numOfPlatforms];
	this->currentPlatform = rhs.currentPlatform;
	std::memcpy(this->platforms, rhs.platforms, sizeof(cl_platform_id)*numOfPlatforms);
	this->dev = new Devices[numOfPlatforms];
	for (unsigned i = 0; i < this->numOfPlatforms; ++i)
	{
		this->dev[i].numOfDevices = rhs.dev[i].numOfDevices;
		this->dev[i].device = new cl_device_id[dev[i].numOfDevices];
		std::memcpy(this->dev[i].device, rhs.dev[i].device, (sizeof(cl_device_id))*(dev[i].numOfDevices));
	}
	this->context = rhs.context;
	for (unsigned i = 0; i < rhs.dev[currentPlatform].numOfDevices; ++i)
	{
		this->commandQueues[i] = rhs.commandQueues[i];
	}
}
void OpenCLass::printDevHelper(const cl_int platformNumber, const cl_int deviceNumber,  const cl_device_info par, char** buf)
{
	size_t size;
	retCode = CL_SUCCESS;
	retCode = clGetDeviceInfo(dev[platformNumber].device[deviceNumber], par, 0, NULL, &size);
	if (retCode != CL_SUCCESS)
	{
		std::cout << "Device is not valid\n";
		exit(1);
	}
	*buf = new char[size + 1];
	retCode = clGetDeviceInfo(dev[platformNumber].device[deviceNumber], par, size, *buf, NULL);
	if (retCode != CL_SUCCESS)
	{
		std::cout << "Device is not valid\n";
		exit(1);
	}
}
OpenCLass& OpenCLass::operator=(const OpenCLass & rhs)
{
	if (this != &rhs)
	{
		deleteHelper();
		copyHelper(rhs);
	}
	return *this;
}
void OpenCLass::printPlatformName(int pltfrmNum)
{
	char* info = nullptr;
	size_t size;
	retCode = clGetPlatformInfo(platforms[pltfrmNum], CL_PLATFORM_NAME, NULL, info, &size); // get size of profile char array
	info = new char[size+1];
	retCode = clGetPlatformInfo(platforms[pltfrmNum], CL_PLATFORM_NAME, size, info, NULL); // get profile char array
	std::cout << "Platform " << pltfrmNum << " is " << info << std::endl;

}
void OpenCLass::printAllDevices()
{	
	std::cout << "There are " << getDevicesCount() << " devices.Where do you want to output information about all of them?\n[1].standard output\n[2].file\n";
	short k = 0;
	while( k != 1 && k != 2 )
		std::cin >> k;
	if( k == 2 )
	{	
		std::ofstream truncFile("Detailed_device_information.txt", std::ios::out | std::ios::trunc);
		truncFile.close();
		std::ofstream deviceInfo("Detailed_device_information.txt", std::ios::out | std::ios::ate);
		for (cl_uint i = 0; i < dev[currentPlatform].numOfDevices; ++i)
		{	
		printDeviceInformation(0, i ,deviceInfo);
		}
		deviceInfo.close();
		std::cout << "Device information is saved in 'Detailed_device_information.txt' file\n";
	}
	else
	{
		for (cl_uint i = 0; i < dev[currentPlatform].numOfDevices; ++i)
                {
                printDeviceInformation(0, i ,std::cout);
                std::cout << std::endl;
		}
	}
}
void OpenCLass::printPlatformInformation(int num)
{	
	if ( num == -1 )
		num = currentPlatform;
	size_t size; //to get size of char buffer
	char * profiles[5];
	for (int i = 0; i < 5; ++i)
	{
		profiles[i] = nullptr;
	}
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_PROFILE, NULL, profiles[0], &size); // get size of profile char array
	profiles[0] = new char[size+1];
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_PROFILE, size, profiles[0], NULL); // get profile char array
	std::cout << std::endl << profiles[0] << std::endl;
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_VERSION, NULL, profiles[1], &size); // get size of profile char array
	profiles[1] = new char[size+1];
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_VERSION, size, profiles[1], NULL); // get profile char array
	std::cout << profiles[1] << std::endl;
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_NAME, NULL, profiles[2], &size); // get size of profile char array
	profiles[2] = new char[size+1];
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_NAME, size, profiles[2], NULL); // get profile char array
	std::cout << profiles[2] << std::endl;
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_VENDOR, NULL, profiles[3], &size); // get size of profile char array
	profiles[3] = new char[size+1];
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_VENDOR, size, profiles[3], NULL); // get profile char array
	std::cout << profiles[3] << std::endl;
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_EXTENSIONS, NULL, profiles[4], &size); // get size of profile char array
	profiles[4] = new char[size+1];
	retCode = clGetPlatformInfo(platforms[num], CL_PLATFORM_EXTENSIONS, size, profiles[4], NULL); // get profile char array
	std::cout << profiles[4] << std::endl << std:: endl;
	for (int i = 0; i < 5; ++i)
	{
		delete[] profiles[i];
	}
}
int OpenCLass::selectPlatform()
{
	int key = -1;
	std::cout << "A list of available platforms will be printed:\n\n";
	for (int i = 0; i < numOfPlatforms; ++i)
	{
		std::cout << "[" << i << "]:";
		printPlatformName(i);
	}
	while (key < 0 || key >= numOfPlatforms)
	{
		std::cout << "PLEASE SELECT PLATFORM NUMBER\n\n";
		std::cout << "[-1]If you want to return to previous menu enter \n\n";
		std::cin >> key;
		if ( key == -1 )
			return -1;
	}
	currentPlatform = key;
	return key;
}
void OpenCLass::printDeviceInformation(cl_int platformNumber, cl_int deviceNumber, std::ostream& out)
{
	if (this->dev == nullptr || this->dev[platformNumber].device == nullptr)
	{
		std::cout << "\nNo initialized any devices in that platform\n";
		return;
	}
	if (this->dev[platformNumber].numOfDevices <= deviceNumber)
	{
		std::cout << "\nDevice with that number " << deviceNumber << " in this platform " << platformNumber << " does not exist!\n";
		return;
	}
	printDeviceNameAndVendor(platformNumber , deviceNumber, out );
	char * info = nullptr;
	char * deviceName = nullptr;
//	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_NAME, &deviceName);
	cl_uint buf = 0;
	size_t longVal = 0;
	cl_device_type type;

	retCode = CL_SUCCESS;
	printDevHelper(platformNumber, deviceNumber, CL_DRIVER_VERSION, &info);
	out << "Device driver version: " << info << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PROFILE, &info);
	out <<  "Supported OpenCL features by device: " << info << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_VERSION, &info);
	out << "Supported OpenCL version by device: " << info << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_TYPE, type);
	out  << "Device type: ";
	if (type == 4)
		out << "GPU\n";
	else if (type == 2)
		out << "CPU\n";
	else if (type == 8)
		out << "ACCELERATOR\n";
	else
		out << "DEFAULT\n";

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_VENDOR_ID, buf);
	out << "Unique device vendor identifier " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_COMPUTE_UNITS, buf);
	out << "Maximum compute units count for this device  is " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_CLOCK_FREQUENCY, buf);
	out << "Maximum clock frequency for this device is " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_ADDRESS_BITS, buf);
	out << "Device address in bits(32 or 64): " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_MEM_ALLOC_SIZE, longVal);
	out << "Max size of memory object allocation in MB: " << longVal / 1048576 << std::endl; // 1048576 = 1024 * 1024

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_GLOBAL_MEM_SIZE, longVal);
	out << "Size of global device memory in MB: " << longVal / 1048576 << std::endl; // 1048576 = 1024 * 1024

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_PARAMETER_SIZE, longVal);
	out << "Max size in bytes of the arguments that can be passed to a kernel: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, buf);
	out << "Size of global memory cache line in bytes: " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, longVal);
	out << "Size of global memory cache in bytes: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_ERROR_CORRECTION_SUPPORT, buf);
	out << "Eror correction support: " << (buf != 0 ? "true\n" : "false\n");

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_LOCAL_MEM_SIZE, longVal);
	out << "Size of local memory arena in bytes: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, longVal);
	out << "Max size in bytes of a constant buffer allocation: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, buf);
	out << "Maximum work items dimension: " << buf << std::endl;

	size_t longArr[3] = { 0, 0, 0 };
	retCode = clGetDeviceInfo(dev[platformNumber].device[deviceNumber], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &longVal); 
	clGetDeviceInfo(dev[platformNumber].device[deviceNumber], CL_DEVICE_MAX_WORK_ITEM_SIZES, longVal, longArr, &longVal);
	out << "Maximum number of work-items that can be specified in each dimension of the work-group: [" << longArr[0] << "] [" << longArr[1] << "] [ " << longArr[2] << "]\n";

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_WORK_GROUP_SIZE, longVal);
	out << "Maximum work items in work group: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_IMAGE_SUPPORT, buf);
	out << "Device image support: " << (buf != 0 ? "true\n" : "false\n");

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_READ_IMAGE_ARGS, buf);
	out << "Max number of simultaneous image objects that can be read by a kernel: " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, buf);
	out << "Max number of simultaneous image objects that can be written to by a kernel: " << buf << std::endl;
	
	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_IMAGE2D_MAX_WIDTH, longVal);
	out << "Max width of 2D image in pixels: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_IMAGE2D_MAX_HEIGHT, longVal);
	out << "Max height of 2D image in pixels: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_IMAGE3D_MAX_DEPTH, longVal);
	out << "Max depth of 3D image in pixels: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_IMAGE3D_MAX_HEIGHT, longVal);
	out << "Max height of 3D image in pixels: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_IMAGE3D_MAX_WIDTH, longVal);
	out << "Max width of 3D image in pixels: " << longVal << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_MAX_SAMPLERS, buf);
	out << "Maximum number of samplers that can be used in a kernel: " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, buf);
	out << "Preferred char vector width " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, buf);
	out << "Preferred short vector width " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, buf);
	out << "Preferred int vector width " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, buf);
	out << "Preferred long vector width " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, buf);
	out << "Preferred float vector width " << buf << std::endl;

	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, buf);
	out << "Preferred double vector width " << buf << std::endl << std::endl << std::endl;
}
void OpenCLass::printDeviceNameAndVendor(cl_int platformNumber, cl_int deviceNumber, std::ostream& out)
{
	if ( this->dev == nullptr || this->dev[platformNumber].device == nullptr )
	{
		std::cout << "Wrong platform number or device number\n";
		return;
	}
	if (this->dev[platformNumber].numOfDevices <= deviceNumber)
	{
		std::cout << "Device with that number " << deviceNumber << " in this platform " << platformNumber << " does not exist!\n";
		return;
	}
	char * info = nullptr;
	out << "Platform " << platformNumber << " Device " << deviceNumber << std::endl;
	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_NAME, &info);
	out << "Selected Device Name: " << info << std::endl;
	delete[] info;
	printDevHelper(platformNumber, deviceNumber, CL_DEVICE_VENDOR, &info);
	out << "Selected Device Vendor: " << info << std::endl << std::endl;
	delete[] info;
}
void OpenCLass::selectDifferentDevices()
{
	if (platforms == nullptr)
	{
		std::cout << "There are no initialized platforms\nTrying to initialize platforms...\n";
		if (!initPlatforms())
		{
			exit(4);
		}
		std::cout << "Success.\n";
	}
	if (dev[currentPlatform].device == nullptr)
	{
		std::cout << "There are no initialized devices in that platform\n";
	}
	char yn = 'f';
	while (yn != 'y' && yn != 'Y' && yn != 'n' && yn != 'N')
	{ 
		std::cout << "Do you want to use another platform?\n[yY] Yes\n[nN] No \n";
		std::cin >> yn;
	}

	if (yn == 'y' || yn == 'Y')
	{
		currentPlatform = selectPlatform();
	}

	if (dev[currentPlatform].device != nullptr)
	{
		delete[] dev[currentPlatform].device;
	}
	if (commandQueues != nullptr)
	{
		delete[] commandQueues;
	}
	
	int retState = initDevices(currentPlatform);
	if (retState == 1)
	{
		exit(1);
	}
	while (retState == 2)
	{
		retState = initDevices(currentPlatform);
	}
	if (!initContext())
	{
		exit(2);
	}
	if (!initQueues())
	{
		exit(3);
	}

}
cl_program OpenCLass::createProgram(const char * fileName)
{
	cl_program program;
	if (fileName == nullptr)
		return nullptr;
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

	retCode = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (retCode != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		for (cl_uint i = 0; i < dev[currentPlatform].numOfDevices; ++i)
		{
			clGetProgramBuildInfo(program, dev[currentPlatform].device[i], CL_PROGRAM_BUILD_LOG,
				sizeof(buildLog), buildLog, NULL);

			std::cerr << "Error in kernel: " << std::endl;
			std::cerr << buildLog;
		}
		clReleaseProgram(program);
		return nullptr;
	}
	return program;
}
bool OpenCLass::initContext()
{
	{
		context = clCreateContext(NULL, dev[currentPlatform].numOfDevices, dev[currentPlatform].device, NULL, NULL, &retCode);
		if (retCode != CL_SUCCESS) {
			std::cout << "Couldn't create context\n";
			return false;
		}
		return true;
	}
}
int OpenCLass::initDevices(cl_int platformNumber)
{
	cl_uint num1, num2, num3, num4;
	if (platformNumber == -1)
	{
		platformNumber = selectPlatform();
		if (platformNumber == -1)
		{
			std::cout << "You've chosen to return back without initializing a device\n";
			return 3;
		}
		std::cout << "Please select the device type you want to use\n";
	}
	retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_GPU, 0, NULL, &num1);
	if (num1 > 0)
		std::cout << "[1] GPU\n";
	retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_CPU, 0, NULL, &num2);
	if (num2 > 0)
		std::cout << "[2] CPU\n";
	retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &num3);
	if (num3 > 0)
		std::cout << "[3] Dedicated OpenCL accelerator\n";
	retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_ALL, 0, NULL, &num4);
	if (num4 > 0)
		std::cout << "[4] All available devices\n";
	int key = 0;
	if (num1 < 1 && num2 < 1 && num3 < 1 && num4 < 1)
	{
		std::cout << "There are no OPENCL available devices\n";
		return 1;
	}
	while (key < 1 || key > 4)
	{
		
		std::cout << "Please choose correct number:\n";
		std::cin >> key;
	}
	switch (key)
	{
	case 1:
		dev[platformNumber].device = new cl_device_id[num1];
		retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_GPU, num1, dev[platformNumber].device, NULL); dev[platformNumber].numOfDevices = num1; break;
	case 2:
		dev[platformNumber].device = new cl_device_id[num2];
		retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_CPU, num2, dev[platformNumber].device, NULL); dev[platformNumber].numOfDevices = num2; break;
	case 3:
		dev[platformNumber].device = new cl_device_id[num3];
		retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_ACCELERATOR, num3, dev[platformNumber].device, NULL); dev[platformNumber].numOfDevices = num3; break;
	case 4:
		dev[platformNumber].device = new cl_device_id[num4];
		retCode = clGetDeviceIDs(platforms[platformNumber], CL_DEVICE_TYPE_ALL, num4, dev[platformNumber].device, NULL); dev[platformNumber].numOfDevices = num4; break;
	}
	if (retCode == CL_INVALID_VALUE)
	{
		std::cout << "Wrong number is chosen.There are no devices available with that number.\n";
		return 2;
	}
	return 0;

}
bool OpenCLass::initPlatforms()
{
	retCode = clGetPlatformIDs(0, nullptr, &numOfPlatforms);
	if (retCode == CL_INVALID_VALUE)
	{
		std::cout << "ERROR: Couldn't identify a platform\n";
		return false;
	}
	std::cout << numOfPlatforms << " platforms found.initializing.\n";
	platforms = new cl_platform_id[numOfPlatforms];
	retCode = clGetPlatformIDs(numOfPlatforms, platforms, &numOfPlatforms);
	dev = new Devices[numOfPlatforms];
	currentPlatform = 0;
	printPlatformName(0);
	std::cout << "Platform 0 is loaded for use\n";
	if (retCode == CL_INVALID_VALUE)
	{
		std::cout << "ERROR: num_entries parameter is equal to zero and platforms parameter is not NULL or num_platforms parameter and platforms parameter are NULL\n";
		return false;
	}
	return true;
}
bool OpenCLass::initQueues()
{
	{
		commandQueues = new cl_command_queue[dev[currentPlatform].numOfDevices];
		for (cl_uint i = 0; i < dev[currentPlatform].numOfDevices; ++i)
		{

			commandQueues[i] = clCreateCommandQueue(context, dev[currentPlatform].device[i], CL_QUEUE_PROFILING_ENABLE, &retCode);
			if (commandQueues[i] == nullptr)
			{
				std::cerr << "Failed to create commandQueue for device " << i << std::endl;
				return false;
			}
		}
		return true;
	}
}
void OpenCLass::init()
{
	bool retState = true;
	int errorCode;
	retState = initPlatforms();
	if (retState == false)
	{
		exit(4);
	}
	if( numOfPlatforms != 1)
	{
		std::cout << "Do you want to select another platform?\n[y]Yes\n[n]No\n";
		char key;
		std::cin >> key;
		while ( key != 'n' && key != 'N' && key != 'y' && key != 'Y')
		{
			std::cout << "[y]Yes or [n]No?\n";
			std::cin >> key;
		}
		if (key == 'Y' || key == 'y')
		{	
			selectPlatform();
		}
	}
	errorCode = initDevices(currentPlatform);
	if (errorCode == 1)
	{
		exit(1);
	}
	while (errorCode == 2)
	{
		errorCode = initDevices(currentPlatform);
	}
	retState = initContext();
	if (retState == false)
	{
		exit(2);
	}
	retState = initQueues();
	if (retState == false)
		exit(3);
}
