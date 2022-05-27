
#include <iostream>
#include <iomanip>
#include <string>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>


std::string kernel_str = R"kernelstr(

kernel void vec_add (global float* a, global float* b, global float* c)
{
    int i = get_global_id(0);
    c[i] = a[i] + b[i];
}

)kernelstr";

int main()
{
    cl_int errNum;

    float a[10], b[10], c[10];
    for (int i=0 ; i<10 ; i++)
    {
        a[i] = i;
        b[i] = i+i;
    }

    cl_mem a_d, b_d, c_d;

    cl_platform_id platform;
    errNum = clGetPlatformIDs(1, &platform, NULL);

    char info[200];
    errNum = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 200, info, NULL);
    std::cout << "Platform name: " << info << std::endl;

    cl_device_id device;
    errNum = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

    cl_context context;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &errNum);
    if (errNum != CL_SUCCESS) { std::cout << "ERROR: creating context" << std::endl; return -1; }

    cl_program program;
    const char* kc = kernel_str.c_str();
    size_t kclen   = kernel_str.size();
    program = clCreateProgramWithSource(context, 1, &kc, &kclen, &errNum);
    if (errNum != CL_SUCCESS) { std::cout << "ERROR: creating program" << std::endl; return -2; }
    
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    cl_kernel kernel;
    kernel = clCreateKernel(program, "vec_add", &errNum);

    cl_command_queue queue;
    queue = clCreateCommandQueue(context, device, 0, &errNum);


    a_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*10, a, &errNum);
    b_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*10, b, &errNum);
    c_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*10, NULL, &errNum);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &a_d);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &b_d);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &c_d);

    size_t work_units = 10;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, c_d, CL_TRUE, 0, sizeof(float)*10, c, 0, NULL, NULL);

    for (int i=0 ; i<10 ; i++)
    {
        std::cout << std::setw(2) << a[i] << " ";
    }
    std::cout << std::endl;
    
    for (int i=0 ; i<10 ; i++)
    {
        std::cout << std::setw(2) << b[i] << " ";
    }
    std::cout << std::endl;
    
    for (int i=0 ; i<10 ; i++)
    {
        std::cout << std::setw(2) << c[i] << " ";
    }
    std::cout << std::endl;

    clReleaseMemObject(a_d);
    clReleaseMemObject(b_d);
    clReleaseMemObject(c_d);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    return 1;
}
