del *.exe
del *.obj
del *.res
del *.txt

cls

cl.exe /c /EHsc /I "..\..\assimp\Include" /I "..\..\glm" /I C:\VulkanSDK\Vulkan\include vk.cpp ModelLoading.cpp

rc.exe vk.rc

link.exe vk.obj ModelLoading.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib /LIBPATH:"..\..\assimp\Lib" user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe