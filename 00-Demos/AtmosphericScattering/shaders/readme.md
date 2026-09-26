Shader Compile Commands!

// sky dome
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o sky.vert.spv sky.vert
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o sky.frag.spv sky.frag

// ground sphere
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ground.vert.spv ground.vert
C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o ground.frag.spv ground.frag

// the .spv outputs must sit next to vk.cpp (project root), not under shaders/,
// since createShaders() reads them via a relative path from the exe's working directory.
