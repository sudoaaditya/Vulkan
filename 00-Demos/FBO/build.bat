del *.exe
del *.obj
del *.txt
del *.spv

cls

@REM C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader.vert.spv shaders-cube/shader.vert 

@REM C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader.frag.spv shaders-cube/shader.frag

@REM C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader_teapot.vert.spv shaders-teapot/shader.vert 

@REM C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader_teapot.frag.spv shaders-teapot/shader.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\include vk.cpp fbo.cpp

rc.exe vk.rc

link.exe vk.obj fbo.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe