
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/opencl.h>


int main() {

    cl_int errNum;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platformIds;
    cl_device_id *deviceIds;
    cl_context context = NULL;
    size_t size;
    char info[1000];
    cl_uint i,j;

    size_t info_st[3];
    cl_uint info_uint;
    cl_long info_ulong;
    cl_device_type dtype;
    char c;

    errNum = clGetPlatformIDs(0, NULL, &numPlatforms);
    platformIds = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numPlatforms);
    errNum = clGetPlatformIDs(numPlatforms, platformIds, NULL);

    printf("Number of platforms: %u\n\n", numPlatforms);
    
    for (i=0 ; i<numPlatforms ; i++) {
        errNum = clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, 0, NULL, &size);
        errNum = clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, size, info, NULL);
        printf("Platform %u name: %s\n\n", i, info);

        errNum = clGetDeviceIDs(platformIds[i],CL_DEVICE_TYPE_ALL,0,NULL,&numDevices);
        deviceIds = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
        errNum = clGetDeviceIDs(platformIds[i],CL_DEVICE_TYPE_ALL,numDevices,deviceIds,NULL);
        for (j=0 ; j<numDevices ; j++) {
            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_TYPE,sizeof(cl_device_type),&dtype,&size);
            switch (dtype) {
                case CL_DEVICE_TYPE_CPU :
                    printf("  Device is a CPU\n");
                    break;
                case CL_DEVICE_TYPE_GPU :
                    printf("  Device is a GPU\n");
                    break;
                default :
                    printf("  Device is other\n");
            }

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_NAME,100*sizeof(char),info,&size);
            printf("  Device name         : %s\n", info);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DRIVER_VERSION,100*sizeof(char),info,&size);
            printf("  Driver version      : %s\n", info);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_VERSION,100*sizeof(char),info,&size);
            printf("  Device version      : %s\n", info);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(cl_uint),&info_uint,&size);
            printf("  Max compute units   : %u\n", info_uint);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,sizeof(cl_uint),&info_uint,&size);
            printf("  Max work item dim   : %u\n", info_uint);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_MAX_WORK_ITEM_SIZES,3*sizeof(size_t),info_st,&size);
            printf("  Max work item size  : %u %u %u\n", info_st[0], info_st[1], info_st[2]);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(size_t),&info_st[0],&size);
            printf("  Max work group size : %u\n", info_st[0]);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_MAX_MEM_ALLOC_SIZE,sizeof(cl_ulong),&info_ulong,&size);
            printf("  Max memory alloc    : %ul\n", info_ulong);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_GLOBAL_MEM_SIZE,sizeof(cl_ulong),&info_ulong,&size);
            printf("  Global mem size     : %ul\n", info_ulong);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_LOCAL_MEM_SIZE,sizeof(cl_ulong),&info_ulong,&size);
            printf("  Local mem size      : %ul\n", info_ulong);

            errNum = clGetDeviceInfo(deviceIds[j],CL_DEVICE_MAX_PARAMETER_SIZE,sizeof(size_t),&info_st[0],&size);
            printf("  Max arg to kernel   : %u\n", info_st[0]);
        }

        free(deviceIds);
        printf("\n");
    }

    free(platformIds);

    fflush(stdin);
    //scanf("%c", &c);

    return 1;
}
