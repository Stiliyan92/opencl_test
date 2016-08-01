#ifndef OpenCLass_H
#define OpenCLass_H
#include<iostream>
#include<fstream>
#include<sstream>
#include<cstring>
#include<CL/cl.h>


struct Devices
{
	Devices() : device(nullptr), numOfDevices(0)
	{

	}
	cl_device_id* device;
	cl_uint numOfDevices;
};
class OpenCLass
{
public:

	OpenCLass() : platforms(nullptr), dev(nullptr), numOfPlatforms(0), currentPlatform(0), context(nullptr), commandQueues(nullptr)
	{
	}
	~OpenCLass()
	{
		deleteHelper();
	}
	bool initPlatforms();
	void printPlatformName(int);
	void printPlatformInformation(int num = -1);
	int initDevices(cl_int platform = -1);
	void printDeviceNameAndVendor(cl_int platformNumber , cl_int deviceNumber, std::ostream & out = std::cout );
	void printDeviceInformation(cl_int platformNumber = 0, cl_int deviceNumber = 0, std::ostream & out = std::cout);
	void printAllDevices();
	void selectDifferentDevices();
	cl_platform_id * getPlatform()
	{
		return platforms;
	}
	cl_device_id* getDevices()
	{
		return dev[currentPlatform].device;
	}
	cl_uint getDevicesCount()
	{
		return dev[currentPlatform].numOfDevices;
	}
	template<class T>
	void getSpecificDeviceInfo(const cl_int platformNumber, const cl_int deviceNumber, const cl_device_info info, T& value)
	{
		printDevHelper(platformNumber, deviceNumber, info, value);
	}
	bool initContext();
	bool initQueues();
	void init();
	cl_program createProgram(const char* fileName);
	cl_int getPreferredWorkGroupSize(cl_kernel);
	cl_context getContext()
	{
		return context;
	}
	cl_command_queue getCommandQueue(const cl_uint deviceNum)
	{
		if (deviceNum >= dev[currentPlatform].numOfDevices)
		{	
			std::cout << "There is no device with that number.\n";
			return nullptr;
		}
		return commandQueues[deviceNum];
	}
	cl_uint getCurrentPlatform()
	{
		return currentPlatform;
	}
private:
	template< class T>
	void printDevHelper(const cl_int, const cl_int, const cl_device_info, T&);
	void printDevHelper(const cl_int, const cl_int, const cl_device_info, char **);
	int selectPlatform();
	void deleteHelper();
	void copyHelper(const OpenCLass &);
	OpenCLass(const OpenCLass & rhs);
	OpenCLass& operator=(const OpenCLass & rhs);
	cl_platform_id* platforms;
	cl_uint numOfPlatforms;
	cl_uint currentPlatform;
	Devices* dev;
	cl_int retCode;
	cl_context context;
	cl_command_queue * commandQueues;
};
template< class T>
void OpenCLass::printDevHelper(cl_int platformNumber, cl_int deviceNumber, cl_device_info par, T& val)
{
	size_t size;
	char * info;
	retCode = CL_SUCCESS;
	retCode = clGetDeviceInfo(dev[platformNumber].device[deviceNumber], par, 0, NULL, &size);
	if (retCode != CL_SUCCESS)
	{
		std::cout << "Device is not valid\n";
		exit(1);
	}
	info = new char[size + 1];
	retCode = clGetDeviceInfo(dev[platformNumber].device[deviceNumber], par, size, &val, NULL);
	if (retCode != CL_SUCCESS)
	{
		std::cout << "Device is not valid\n";
		exit(1);
	}
}
#endif
