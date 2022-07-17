
#include <stdio.h>
#include <vulkan/vulkan.h>


int main()
{
    VkInstance instance;
    
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS)
    {
        printf("Failed to create instance!\n");
        return -1;
    }
    
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

    printf("Device count: %u\n", deviceCount);
    if (deviceCount == 0) { return -2; }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    for (uint32_t i = 0; i<deviceCount; ++i)
    {
        VkPhysicalDeviceFeatures   device_feats;
        VkPhysicalDeviceProperties device_props;
        vkGetPhysicalDeviceFeatures  (devices[i], &device_feats);
        vkGetPhysicalDeviceProperties(devices[i], &device_props);
        printf("Device %d\n", i);
        printf("    Name: %s\n", device_props.deviceName);
        printf("    Api version:         %u.%u.%u\n",
               // VK_API_VERSION_VARIANT(device_props.apiVersion), //  (device_props.apiVersion >> 29),
               /*VK_API_VERSION_MAJOR(device_props.apiVersion),*/    (device_props.apiVersion >> 22) & 0x7F,
               /*VK_API_VERSION_MINOR(device_props.apiVersion),*/    (device_props.apiVersion >> 12) & 0x3FF,
               /*VK_API_VERSION_PATCH(device_props.apiVersion) */    (device_props.apiVersion)       & 0xFFF
        );
        printf("    Driver version:      %u\n", device_props.driverVersion);
        printf("    Max Mem Alloc:       %u\n", device_props.limits.maxMemoryAllocationCount);
        printf("    Max Sampler Alloc:   %u\n", device_props.limits.maxSamplerAllocationCount);
        printf("    Max Image1D:         %u\n", device_props.limits.maxImageDimension1D);
        printf("    Max Image2D:         %u\n", device_props.limits.maxImageDimension2D);
        printf("    Max Image3D:         %u\n", device_props.limits.maxImageDimension3D);
        printf("    Max Compute WG cnt:  %u x %u x %u\n",
               device_props.limits.maxComputeWorkGroupCount[0],
               device_props.limits.maxComputeWorkGroupCount[1],
               device_props.limits.maxComputeWorkGroupCount[2]);
        printf("    Max Compute WG size: %u x %u x %u\n",
               device_props.limits.maxComputeWorkGroupSize[0],
               device_props.limits.maxComputeWorkGroupSize[1],
               device_props.limits.maxComputeWorkGroupSize[2]);
        printf("    Max Compute SharedM: %u\n", device_props.limits.maxComputeSharedMemorySize);
        printf("    Max Framebuf Dims:   %u x %u x %u\n",
               device_props.limits.maxFramebufferWidth,
               device_props.limits.maxFramebufferHeight,
               device_props.limits.maxFramebufferLayers);
        
        
        printf("    Geometry shader:     "); device_feats.geometryShader == VK_TRUE ? printf("yes\n") : printf("no\n");
        printf("    Wide lines:          "); device_feats.wideLines      == VK_TRUE ? printf("yes\n") : printf("no\n");
        printf("    Large points:        "); device_feats.largePoints    == VK_TRUE ? printf("yes\n") : printf("no\n");
    }
    
    vkDestroyInstance(instance, NULL);

    return 0;
}
