if not exist obj mkdir obj

del *.exe
del obj\*.obj
del obj\*.res
del *.txt
del *.spv

cls

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o sky.vert.spv shaders/sky.vert

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o sky.frag.spv shaders/sky.frag

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ground.vert.spv shaders/ground.vert

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ground.frag.spv shaders/ground.frag

cl.exe /c /EHsc /Fo"obj\\" /I C:\VulkanSDK\Vulkan\include /I imgui /I imgui\backends vk.cpp clockUtils\Clock.cpp imgui\imgui.cpp imgui\imgui_draw.cpp imgui\imgui_tables.cpp imgui\imgui_widgets.cpp imgui\imgui_demo.cpp imgui\backends\imgui_impl_win32.cpp imgui\backends\imgui_impl_vulkan.cpp

rc.exe /fo obj\vk.res vk.rc

link.exe obj\vk.obj obj\Clock.obj obj\imgui.obj obj\imgui_draw.obj obj\imgui_tables.obj obj\imgui_widgets.obj obj\imgui_demo.obj obj\imgui_impl_win32.obj obj\imgui_impl_vulkan.obj obj\vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib imm32.lib shell32.lib dwmapi.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe
