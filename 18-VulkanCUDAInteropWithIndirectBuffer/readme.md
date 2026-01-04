
It is recommended to use two semaphores for Vulkan and two for CUDA—one for waiting and one for signaling.
This requires using CUDA streams that can work with multiple semaphores.
As an alternative, timeline semaphores can be used, but they do not work with older Vulkan versions.

Initialize()
CUDA requires VkPhysical and VkDevice to initialize itself. Unlike, OpenGL interoperability, Vulkan does not have dedicated API's to initialize CUDA. Hence, initialize_cuda() has been called after getDeviceQueue().

+ nvidia-smi -i 0 -c PROHIBITED
    - smi : System Management Interface
    - -i : Device Index (0th device in this case)
    - -c : Compute Mode
- With this command we're making the compute mode for this device prohibited, no cuda application will run after execution of this command