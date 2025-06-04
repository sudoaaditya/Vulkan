del *.exe
del *.obj
del *.txt

cls

cl.exe /MD /std:c++17 /c /EHsc /I C:\VulkanSDK\Vulkan\include /I C:\VulkanSDK\Vulkan\include\glslang vk.cpp

rc.exe vk.rc

link.exe vk.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe