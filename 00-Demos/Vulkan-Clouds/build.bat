del *.exe
del *.obj
del *.txt
del *.res
del *.spv

cls

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader.vert.spv shader.vert 

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader.frag.spv shader.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\include vk.cpp clockUtils\Clock.cpp

rc.exe vk.rc

link.exe vk.obj Clock.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe