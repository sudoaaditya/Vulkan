del *.exe
del *.obj
del *.txt

cls

rc.exe vk.rc

nvcc.exe -I C:\VulkanSDK\Vulkan\include -L C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib -o VK.exe vk.res vk.cu -diag-suppress 20012 -diag-suppress 20013 -diag-suppress 20014 -diag-suppress 20015 -diag-suppress 2464 -diag-suppress 177 -allow-unsupported-compiler -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH

VK.exe