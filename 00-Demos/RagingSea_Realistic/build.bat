del *.exe
del *.obj
del *.txt
del *.spv

cls

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader.vert.spv shaders/shader.vert 

C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o shader.frag.spv shaders/shader.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Vulkan\include /I imgui /I imgui\backends vk.cpp clockUtils\Clock.cpp imgui\imgui.cpp imgui\imgui_draw.cpp imgui\imgui_tables.cpp imgui\imgui_widgets.cpp imgui\imgui_demo.cpp imgui\backends\imgui_impl_win32.cpp imgui\backends\imgui_impl_vulkan.cpp

rc.exe vk.rc

link.exe vk.obj Clock.obj imgui.obj imgui_draw.obj imgui_tables.obj imgui_widgets.obj imgui_demo.obj imgui_impl_win32.obj imgui_impl_vulkan.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\lib user32.lib gdi32.lib imm32.lib shell32.lib dwmapi.lib /SUBSYSTEM:WINDOWS /out:"./VK.exe"

VK.exe