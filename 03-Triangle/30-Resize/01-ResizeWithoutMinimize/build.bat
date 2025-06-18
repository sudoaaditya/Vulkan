del *.exe
del *.obj
del *.txt

cls

cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\include vk.c

rc.exe vk.rc

link.exe vk.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe