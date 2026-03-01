// header & macros for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vk.h"
#include "Teapot.h"

int fboWidth = FBO_WIDTH;
int fboHeight = FBO_HEIGHT;

// Surface Format
VkFormat vkFormat_color_fbo = VK_FORMAT_UNDEFINED;

// Fbo Image related Variables
VkImage vkImage_fbo = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_fbo = VK_NULL_HANDLE;
VkImageView vkImageView_fbo = VK_NULL_HANDLE;
VkSampler vkSampler_fbo = VK_NULL_HANDLE;

// For Depth Image
VkFormat vkFormat_depth_fbo = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth_fbo = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth_fbo = VK_NULL_HANDLE;
VkImageView vkImageView_depth_fbo = VK_NULL_HANDLE;

// Command Buffer
VkCommandBuffer vkCommandBuffer_fbo = VK_NULL_HANDLE;

// Render Pass
VkRenderPass vkRenderPass_fbo = VK_NULL_HANDLE;

// Frame Buffer
VkFramebuffer vkFramebuffer_fbo = VK_NULL_HANDLE;

// Fences & Semaphore
VkSemaphore vkSemaphore_fbo = VK_NULL_HANDLE;

// Build Command Buffers
VkClearColorValue vkClearColorValue_fbo;
VkClearDepthStencilValue vkClearDepthStencilValue_fbo;

// Render Variables
BOOL bInitialized_fbo = FALSE;

// Vertex Buffers
float *pPositions = NULL;
float *pNormals = NULL;
float *pTexCoords = NULL;
unsigned int *pElements = NULL;

unsigned int numFaceIndices = 0;
unsigned int numElements = 0;
unsigned int numVerts = 0;

// Position
VertexData vertexData_position_fbo;
// Normal
VertexData vertexData_normal_fbo;
// Texture
VertexData vertexData_texcoord_fbo;
// Indices
VertexData vertexData_elements_fbo;

// Uniform Related Declarations
struct MyUniformData_fbo {
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    // Light Related Uniform
    float lightAmbient[4]; // ambient
    float lightDiffuse[4]; // diffuse
    float lightSpecular[4]; // specular
    float lightPosition[4]; // light position
    // For Material
    float materialAmbient[4]; // ambient
    float materialDiffuse[4]; // diffuse
    float materialSpecular[4]; // specular
    float materialShininess; // shininess
    //keyboard controlled
    int lKeyPressed;
    int textureEnabled;
};

UniformData uniformData_fbo;

// Shader Variables
VkShaderModule vkShaderModule_vertex_fbo = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_fbo = VK_NULL_HANDLE;

// Descriptor Set Layout
VkDescriptorSetLayout vkDescriptorSetLayout_fbo = VK_NULL_HANDLE;

// Pipeline Layout
VkPipelineLayout vkPipelineLayout_fbo = VK_NULL_HANDLE;

// Descriptor Pool
VkDescriptorPool vkDescriptorPool_fbo = VK_NULL_HANDLE;

// Descriptor Set
VkDescriptorSet vkDescriptorSet_fbo = VK_NULL_HANDLE;

// Pipeline
VkViewport vkViewport_fbo;
VkRect2D vkRect2D_scissor_fbo;
VkPipeline vkPipeline_fbo = VK_NULL_HANDLE;

// Texture Related Variables
VkImage vkImage_texture_fbo = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture_fbo = VK_NULL_HANDLE;
VkImageView vkImageView_texture_fbo = VK_NULL_HANDLE;
// Texture Sampler
VkSampler vkSampler_texture_fbo = VK_NULL_HANDLE;

float angleTeapot = 0.0f;

//! FBO RELATED FUNCTIONS

VkResult resize_fbo(int fbo_width, int fbo_height) {
    // Function declarations
    VkResult createSwapchainImagesAndImageViews_fbo(void);
    VkResult createCommandBuffers_fbo(void);
    VkResult createPipelineLayout_fbo(void);
    VkResult createPipeline_fbo(void);
    VkResult createFramebuffer_fbo(void);
    VkResult createRenderPass_fbo(void);
    VkResult buildCommandBuffer_fbo(void);

    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Code
    if(fbo_height <= 0)
        fbo_height = 1;

    // If control comes here before initialization is done, then return false
    if(bInitialized_fbo == FALSE) {
        fprintf(fptr, "resize_fbo(): initialization is not completed or failed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // As recreation of swapchain is required, we are going to repeat many steps of initialization again
    // hence set bInitialized_fbo to FALSE
    bInitialized_fbo = FALSE; // this will prevent display() function to execute before resize() is done

    // Set Global Width & Height
    fboWidth = fbo_width;
    fboHeight = fbo_height;

    // Check if vkSwapchainKHR is NULL, if it is NULL then we cannot proceed
    if(vkSwapchainKHR == VK_NULL_HANDLE) {
        fprintf(fptr, "resize_fbo(): vkSwapchainKHR is NULL cannot proceed!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // Destroy Frame Buffers
    if(vkFramebuffer_fbo) {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_fbo, NULL);
        vkFramebuffer_fbo = VK_NULL_HANDLE;
    }

    // Destroy  Command Buffers
    if(vkCommandBuffer_fbo) {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_fbo);
        vkCommandBuffer_fbo = VK_NULL_HANDLE;
    }

    // Destroy Pipeline
    if(vkPipeline_fbo) {
        vkDestroyPipeline(vkDevice, vkPipeline_fbo, NULL);
        vkPipeline_fbo = VK_NULL_HANDLE;
    }

    // Destroy Pipeline Layout
    if(vkPipelineLayout_fbo) {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout_fbo, NULL);
        vkPipelineLayout_fbo = VK_NULL_HANDLE;
    }

    // Destroy Render Pass
    if(vkRenderPass_fbo) {
        vkDestroyRenderPass(vkDevice, vkRenderPass_fbo, NULL);
        vkRenderPass_fbo = VK_NULL_HANDLE;
    }

    // destroy depth stencil image view
    if(vkImageView_depth_fbo) {
        vkDestroyImageView(vkDevice, vkImageView_depth_fbo, NULL);
        vkImageView_depth_fbo = VK_NULL_HANDLE;
    }

    // destroy depth stencil image
    if(vkImage_depth_fbo) {
        vkDestroyImage(vkDevice, vkImage_depth_fbo, NULL);
        vkImage_depth_fbo = VK_NULL_HANDLE;
    }

    // destroy depth stencil memory
    if(vkDeviceMemory_depth_fbo) {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth_fbo, NULL);
        vkDeviceMemory_depth_fbo = VK_NULL_HANDLE;
    }

    if(vkSampler_fbo){
        vkDestroySampler(vkDevice, vkSampler_fbo, NULL);
        vkSampler_fbo = VK_NULL_HANDLE;
    }

    if(vkImageView_fbo){
        vkDestroyImageView(vkDevice, vkImageView_fbo, NULL);
        vkImageView_fbo = VK_NULL_HANDLE;
    }

    if(vkDeviceMemory_fbo){
        vkFreeMemory(vkDevice, vkDeviceMemory_fbo, NULL);
        vkDeviceMemory_fbo = VK_NULL_HANDLE;
    }

    if(vkImage_fbo){
        vkDestroyImage(vkDevice, vkImage_fbo, NULL);
        vkImage_fbo = VK_NULL_HANDLE;
    }

    // RECREATE FOR RESIZE

    // Create Swapchain Images & Image Views
    vkResult = createSwapchainImagesAndImageViews_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): createSwapchainImagesAndImageViews_fbo() Failed!.\n");
        return (vkResult);
    }

    // Create Render Pass
    vkResult = createRenderPass_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): createRenderPass_fbo() Failed!.\n");
        return (vkResult);
    }

    // Create Pipeline Layout
    vkResult = createPipelineLayout_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): createPipelineLayout_fbo() Failed!.\n");
        return (vkResult);
    }

    // Create Pipeline
    vkResult = createPipeline_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): createPipeline_fbo() Failed!.\n");
        return (vkResult);
    }

    // Create Framebuffers
    vkResult = createFramebuffer_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): createFramebuffer_fbo() Failed!.\n");
        return (vkResult);
    }
    
    // Create Command Buffer
    vkResult = createCommandBuffers_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): createCommandBuffers_fbo() Failed!.\n");
        return (vkResult);
    }

    // Build Command Buffers
    vkResult = buildCommandBuffer_fbo();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "resize_fbo(): buildCommandBuffer_fbo() Failed!.\n");
        return (vkResult);
    }

    // add extra new line for better readability
    fprintf(fptr, "\n\n");

    bInitialized_fbo = TRUE;

    return (vkResult);
}

void update_fbo(void) {

    // code
    angleTeapot += 0.5f;
    if(angleTeapot >= 360.0f) {
        angleTeapot = 0.0f;
    }
}

void uninitialize_fbo(void){

    // Destroy Semaphore
    if(vkSemaphore_fbo) {
        vkDestroySemaphore(vkDevice, vkSemaphore_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroySemaphore() for Render Complete Succeed!\n");
        vkSemaphore_fbo = VK_NULL_HANDLE;
    }

    // Destroy Frame Buffers
    if(vkFramebuffer_fbo) {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyFramebuffer() Succeed!.\n");
        vkFramebuffer_fbo = VK_NULL_HANDLE;
    }

    // Destroy Pipeline
    if(vkPipeline_fbo) {
        vkDestroyPipeline(vkDevice, vkPipeline_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyPipeline() Succeed!\n");
        vkPipeline_fbo = VK_NULL_HANDLE;
    }

    // Destroy Render Pass
    if(vkRenderPass_fbo) {
        vkDestroyRenderPass(vkDevice, vkRenderPass_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyRenderPass() Succeed!\n");
        vkRenderPass_fbo = VK_NULL_HANDLE;
    }

    // Destroy Descriptor Pool
    // When descriptor pool is destroyed, all the descriptor sets created from it are destroyed internally
    // so we  don't need to destroy descriptor set explicitly 
    if(vkDescriptorPool_fbo) {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDescriptorPool_fbo & vkDescriptorSet_fbo Destroy Succeed!\n");
        vkDescriptorPool_fbo = VK_NULL_HANDLE;
        vkDescriptorSet_fbo = VK_NULL_HANDLE;
    }

    // Destroy Pipeline Layout
    if(vkPipelineLayout_fbo) {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyPipelineLayout() Succeed!\n");
        vkPipelineLayout_fbo = VK_NULL_HANDLE;
    }

    // Destroy Descriptor Set Layout
    if(vkDescriptorSetLayout_fbo) {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyDescriptorSetLayout() Succeed!\n");
        vkDescriptorSetLayout_fbo = VK_NULL_HANDLE;
    }

    // Destroy Shader
    if(vkShaderModule_fragment_fbo) {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyShaderModule() Succeed for Fragment Shader!\n");
        vkShaderModule_fragment_fbo = VK_NULL_HANDLE;
    }

    if(vkShaderModule_vertex_fbo) {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyShaderModule() Succeed for Vertex Shader!\n");
        vkShaderModule_vertex_fbo = VK_NULL_HANDLE;
    }

    // Destroy Uniform Buffer
    if(uniformData_fbo.vkDeviceMemory) {
        vkFreeMemory(vkDevice, uniformData_fbo.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Uniform Buffer!\n");
        uniformData_fbo.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if(uniformData_fbo.vkBuffer) {
        vkDestroyBuffer(vkDevice, uniformData_fbo.vkBuffer, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyBuffer() Succeed for Uniform Buffer!\n");
        uniformData_fbo.vkBuffer = VK_NULL_HANDLE;
    }

    // Destroy Texture Sampler
    if(vkSampler_texture_fbo) {
        vkDestroySampler(vkDevice, vkSampler_texture_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroySampler() Succeed for Texture Sampler!\n");
        vkSampler_texture_fbo = VK_NULL_HANDLE;
    }

    // Destroy Texture Image View
    if(vkImageView_texture_fbo) {
        vkDestroyImageView(vkDevice, vkImageView_texture_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyImageView() Succeed for Texture Image View!\n");
        vkImageView_texture_fbo = VK_NULL_HANDLE;
    }

    // Destroy Texture Image Memory
    if(vkDeviceMemory_texture_fbo) {
        vkFreeMemory(vkDevice, vkDeviceMemory_texture_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Texture Image Memory!\n");
        vkDeviceMemory_texture_fbo = VK_NULL_HANDLE;
    }

    // Destroy Texture Image
    if(vkImage_texture_fbo) {
        vkDestroyImage(vkDevice, vkImage_texture_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyImage() Succeed for Texture Image!\n");
        vkImage_texture_fbo = VK_NULL_HANDLE;
    }

    // Free allocated memory for model data
    if(pElements) {
        free(pElements);
        pElements = NULL;
        fprintf(fptr, "uninitialize_fbo(): freed pElements!.\n");
    }

    if(pTexCoords) {
        free(pTexCoords);
        pTexCoords = NULL;
        fprintf(fptr, "uninitialize_fbo(): freed pTexCoords!.\n");
    }

    if(pNormals) {
        free(pNormals);
        pNormals = NULL;
        fprintf(fptr, "uninitialize_fbo(): freed pNormals!.\n");
    }

    if(pPositions) {
        free(pPositions);
        pPositions = NULL;
        fprintf(fptr, "uninitialize_fbo(): freed pPositions!.\n");
    }

    // Destroy Vertex Buffer Position
    if(vertexData_position_fbo.vkDeviceMemory) {
        vkFreeMemory(vkDevice, vertexData_position_fbo.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Vertex Buffer for Position!\n");
        vertexData_position_fbo.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if(vertexData_position_fbo.vkBuffer) {
        vkDestroyBuffer(vkDevice, vertexData_position_fbo.vkBuffer, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyBuffer() Succeed for Vertex Buffer for Position!\n");
        vertexData_position_fbo.vkBuffer = VK_NULL_HANDLE;
    }

    // Destroy Vertex Buffer Normal
    if(vertexData_normal_fbo.vkDeviceMemory) {
        vkFreeMemory(vkDevice, vertexData_normal_fbo.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Vertex Buffer for Normal!\n");
        vertexData_normal_fbo.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if(vertexData_normal_fbo.vkBuffer) {
        vkDestroyBuffer(vkDevice, vertexData_normal_fbo.vkBuffer, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyBuffer() Succeed for Vertex Buffer for Normal!\n");
        vertexData_normal_fbo.vkBuffer = VK_NULL_HANDLE;
    }

    // Destroy Index Buffer
    if(vertexData_elements_fbo.vkDeviceMemory) {
        vkFreeMemory(vkDevice, vertexData_elements_fbo.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Index Buffer!\n");
        vertexData_elements_fbo.vkDeviceMemory = VK_NULL_HANDLE;
    }
    if(vertexData_elements_fbo.vkBuffer) {
        vkDestroyBuffer(vkDevice, vertexData_elements_fbo.vkBuffer, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyBuffer() Succeed for Index Buffer!\n");
        vertexData_elements_fbo.vkBuffer = VK_NULL_HANDLE;
    }

    if(vertexData_texcoord_fbo.vkDeviceMemory) {
        vkFreeMemory(vkDevice, vertexData_texcoord_fbo.vkDeviceMemory, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Vertex Buffer for Texcoord!\n");
        vertexData_texcoord_fbo.vkDeviceMemory = VK_NULL_HANDLE;
    }

    if(vertexData_texcoord_fbo.vkBuffer) {
        vkDestroyBuffer(vkDevice, vertexData_texcoord_fbo.vkBuffer, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyBuffer() Succeed for Vertex Buffer for Texcoord!\n");
        vertexData_texcoord_fbo.vkBuffer = VK_NULL_HANDLE;
    }

    // Destroy  Command Buffer
    if(vkCommandBuffer_fbo) {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_fbo);
        fprintf(fptr, "uninitialize_fbo(): vkFreeCommandBuffers() Succeed!\n");
        vkCommandBuffer_fbo = VK_NULL_HANDLE;
    }

    // destroy depth stencil image view
    if(vkImageView_depth_fbo) {
        vkDestroyImageView(vkDevice, vkImageView_depth_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyImageView() Succeed for Depth Stencil Image View!\n");
        vkImageView_depth_fbo = VK_NULL_HANDLE;
    }

    // destroy depth stencil image
    if(vkImage_depth_fbo) {
        vkDestroyImage(vkDevice, vkImage_depth_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyImage() Succeed for Depth Stencil Image!\n");
        vkImage_depth_fbo = VK_NULL_HANDLE;
    }

    // destroy depth stencil memory
    if(vkDeviceMemory_depth_fbo) {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for Depth Stencil Memory!\n");
        vkDeviceMemory_depth_fbo = VK_NULL_HANDLE;
    }

    // Destroy fbo 
    if(vkSampler_fbo){
        vkDestroySampler(vkDevice, vkSampler_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroySampler() Succeed for FBO Sampler!\n");
        vkSampler_fbo = VK_NULL_HANDLE;
    }

    if(vkImageView_fbo){
        vkDestroyImageView(vkDevice, vkImageView_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyImageView() Succeed for FBO Image View!\n");
        vkImageView_fbo = VK_NULL_HANDLE;
    }

    if(vkDeviceMemory_fbo){
        vkFreeMemory(vkDevice, vkDeviceMemory_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkFreeMemory() Succeed for FBO Image Memory!\n");
        vkDeviceMemory_fbo = VK_NULL_HANDLE;
    }

    if(vkImage_fbo){
        vkDestroyImage(vkDevice, vkImage_fbo, NULL);
        fprintf(fptr, "uninitialize_fbo(): vkDestroyImage() Succeed for FBO Image!\n");
        vkImage_fbo = VK_NULL_HANDLE;
    }
}

// Conceptually this is the most important function!

VkResult createSwapchainImagesAndImageViews_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // For FBO Image and Image View
    vkFormat_color_fbo = vkFormat_color;
    vkFormat_depth_fbo = vkFormat_depth;

    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // 2D Image
    vkImageCreateInfo.format = vkFormat_color_fbo; // format of image
    vkImageCreateInfo.extent.width = fboWidth; 
    vkImageCreateInfo.extent.height = fboHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Now we are going to use this image as color attachment and also as sampled image in shader
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Initial Layout is Undefined as we will be transferring data to it

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImage() Failed for FBO Image!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImage() Successful for FBO Image!.\n");
    }

    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));

    vkGetImageMemoryRequirements(vkDevice, vkImage_fbo, &vkMemoryRequirements_image);

    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL;
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0; // this will be set in next step

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1) {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_image, NULL, &vkDeviceMemory_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkAllocateMemory() Failed for FBO Image!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkAllocateMemory() Successful for FBO Image!.\n");
    }

    vkResult = vkBindImageMemory(vkDevice, vkImage_fbo, vkDeviceMemory_fbo, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkBindImageMemory() Failed for FBO Image!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkBindImageMemory() Successful for FBO Image!.\n");
    }

    // For Depth Image
    // Create Image View for FBO Image
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color_fbo; // Same format as Image
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_fbo;

    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkImageView_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImageView() Failed for FBO Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImageView() Successful for FBO Image!.\n");
    }

    // Create Sampler for FBO Image
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void*)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));

    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR; // Linear Filtering
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Linear Mipmapping
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Clamp Texture
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE; // Disable Anisotropy
    vkSamplerCreateInfo.maxAnisotropy = 16.0f; // Maximum Anisotropy
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // Border Color
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // Normalized Coordinates
    vkSamplerCreateInfo.compareEnable = VK_FALSE; // Disable Comparison
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    vkResult = vkCreateSampler(vkDevice, &vkSamplerCreateInfo, NULL, &vkSampler_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateSampler() Failed for FBO Sampler!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateSampler() Successful for FBO Sampler!.\n");
    }

    // For depth image initialize VkImageCreateInfo
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = vkFormat_depth_fbo;
    vkImageCreateInfo.extent.width = winWidth;
    vkImageCreateInfo.extent.height = winHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_depth_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImage() Failed for Depth Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImage() Successful for Depth Image!.\n");
    }

    // Memory Requirements for Depth Image
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetImageMemoryRequirements(vkDevice, vkImage_depth_fbo, &vkMemoryRequirements);

    // Step 6
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_depth_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkAllocateMemory() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkAllocateMemory() Successful!.\n");
    }

    // Step 10
    vkResult = vkBindImageMemory(vkDevice, vkImage_depth_fbo, vkDeviceMemory_depth_fbo, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkBindDev() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkBindDev() Successful!.\n");
    }

    // Create Image View for Depth Image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_depth_fbo;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_depth_fbo;

    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkImageView_depth_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImageView() Failed for Depth Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSwapchainImagesAndImageViews_fbo(): vkCreateImageView() Successful for Depth Image!.\n");
    }

    return (vkResult);
}

VkResult createCommandBuffers_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: Init and Allocate VkCommandBufferAllocateInfo
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    // Step 3: Allocat eeach command buffer in loop with allocateInfo struct
    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createCommandBuffers_fbo(): vkCreatvkAllocateCommandBufferseImageView() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createCommandBuffers_fbo(): vkAllocateCommandBuffers() Successful!.\n");
    }

    return (vkResult);
}

VkResult createVertexBuffer_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    void addTriangle(float[3][3], float[3][3], float[3][2]);

    // Add Teapot Model Data
    // calculate no of face indices
    numFaceIndices = sizeof(face_indicies) / sizeof(face_indicies[0]);

    // Position
    pPositions = (float *)malloc(sizeof(float) * 3 * numFaceIndices);
    // Normal
    pNormals = (float *)malloc(sizeof(float) * 3 * numFaceIndices);
    // TexCoord
    pTexCoords = (float *)malloc(sizeof(float) * 2 * numFaceIndices);
    // Elements
    pElements = (unsigned int *)malloc(sizeof(unsigned int) * 3 * numFaceIndices);

    // Declare tmp array to hol dtriangel vertices
    float vert[3][3];
    float norm[3][3];
    float tex[3][2];

    for(unsigned int i = 0; i < numFaceIndices; i++) {
        for(int j = 0; j < 3; j++) {
            vert[j][0] = vertices[face_indicies[i][j + 0]][0];
            vert[j][1] = vertices[face_indicies[i][j + 0]][1];
            vert[j][2] = vertices[face_indicies[i][j + 0]][2];

            norm[j][0] = normals[face_indicies[i][j + 3]][0];
            norm[j][1] = normals[face_indicies[i][j + 3]][1];
            norm[j][2] = normals[face_indicies[i][j + 3]][2];

            tex[j][0] = textures[face_indicies[i][j + 6]][0];
            tex[j][1] = textures[face_indicies[i][j + 6]][1];
        }

        addTriangle(vert, norm, tex);
    }

    // VertexData for Triangle Position
    // Step 2
    memset((void*)&vertexData_position_fbo, 0, sizeof(VertexData));

    // Step 3
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = sizeof(float) * 3 * numVerts;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position_fbo.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkCreateBuffer() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkCreateBuffer() Successful for Position!.\n");
    }

    // Step 5
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_fbo.vkBuffer, &vkMemoryRequirements);

    // Step 6
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_fbo.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkAllocateMemory() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkAllocateMemory() Successful for Position!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_fbo.vkBuffer, vertexData_position_fbo.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkBindBufferMemory() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkBindBufferMemory() Successful for Position!.\n");
    }

    // Step 11
    void *data = NULL;

    vkResult = vkMapMemory(
        vkDevice,
        vertexData_position_fbo.vkDeviceMemory,
        0,
        vkMemoryAllocateInfo.allocationSize,
        0,
        &data
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkMapMemory() Failed for Position!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkMapMemory() Successful for Position!.\n");
    }

    // Step 12
    memcpy(data, pPositions, sizeof(float) * 3 * numVerts);

    // Step 13
    vkUnmapMemory(vkDevice, vertexData_position_fbo.vkDeviceMemory);


    // For Normal
    memset((void*)&vertexData_normal_fbo, 0, sizeof(VertexData));

    // Step 3
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = sizeof(float) * 3 * numVerts;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_normal_fbo.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkCreateBuffer() Failed for Normals!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkCreateBuffer() Successful for Normals!.\n");
    }

    // Step 5
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_fbo.vkBuffer, &vkMemoryRequirements);

    // Step 6
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_fbo.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkAllocateMemory() Failed for Normals!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkAllocateMemory() Successful for Normals!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_fbo.vkBuffer, vertexData_normal_fbo.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkBindBufferMemory() Failed for Normals!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkBindBufferMemory() Successful for Normals!.\n");
    }

    // Step 11
    data = NULL;

    vkResult = vkMapMemory(
        vkDevice,
        vertexData_normal_fbo.vkDeviceMemory,
        0,
        vkMemoryAllocateInfo.allocationSize,
        0,
        &data
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkMapMemory() Failed for Normals!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkMapMemory() Successful for Normals!.\n");
    }

    // Step 12
    memcpy(data, pNormals, sizeof(float) * 3 * numVerts);

    // Step 13
    vkUnmapMemory(vkDevice, vertexData_normal_fbo.vkDeviceMemory);

    // For Texcoord
    // For Normal
    memset((void*)&vertexData_texcoord_fbo, 0, sizeof(VertexData));

    // Step 3
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = sizeof(float) * 2 * numVerts;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_texcoord_fbo.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkCreateBuffer() Failed for Texcoords!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkCreateBuffer() Successful for Texcoords!.\n");
    }

    // Step 5
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_fbo.vkBuffer, &vkMemoryRequirements);

    // Step 6
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_fbo.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkAllocateMemory() Failed for Texcoords!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkAllocateMemory() Successful for Texcoords!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_fbo.vkBuffer, vertexData_texcoord_fbo.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkBindBufferMemory() Failed for Texcoords!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkBindBufferMemory() Successful for Texcoords!.\n");
    }

    // Step 11
    data = NULL;

    vkResult = vkMapMemory(
        vkDevice,
        vertexData_texcoord_fbo.vkDeviceMemory,
        0,
        vkMemoryAllocateInfo.allocationSize,
        0,
        &data
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createVertexBuffer_fbo(): vkMapMemory() Failed for Texcoords!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createVertexBuffer_fbo(): vkMapMemory() Successful for Texcoords!.\n");
    }

    // Step 12
    memcpy(data, pTexCoords, sizeof(float) * 2 * numVerts);

    // Step 13
    vkUnmapMemory(vkDevice, vertexData_texcoord_fbo.vkDeviceMemory);

    return(vkResult);
}

VkResult createIndexBuffer_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Step 2
    memset((void*)&vertexData_elements_fbo, 0, sizeof(VertexData));

    // Step 3
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = sizeof(unsigned int) * numElements;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_elements_fbo.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createIndexBuffer_fbo(): vkCreateBuffer() Failed for Index Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createIndexBuffer_fbo(): vkCreateBuffer() Successful for Index Buffer!.\n");
    }

    // Step 5
    VkMemoryRequirements vkMemoryRequirements;
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_elements_fbo.vkBuffer, &vkMemoryRequirements);

    // Step 6
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_elements_fbo.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createIndexBuffer_fbo(): vkAllocateMemory() Failed for Index Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createIndexBuffer_fbo(): vkAllocateMemory() Successful for Index Buffer!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice, vertexData_elements_fbo.vkBuffer, vertexData_elements_fbo.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createIndexBuffer_fbo(): vkBindBufferMemory() Failed for Index Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createIndexBuffer_fbo(): vkBindBufferMemory() Successful for Index Buffer!.\n");
    }

    // Step 11
    void *data = NULL;

    vkResult = vkMapMemory(
        vkDevice,
        vertexData_elements_fbo.vkDeviceMemory,
        0,
        vkMemoryAllocateInfo.allocationSize,
        0,
        &data
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createIndexBuffer_fbo(): vkMapMemory() Failed for Index Buffer!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createIndexBuffer_fbo(): vkMapMemory() Successful for Index Buffer!.\n");
    }

    // Step 12
    memcpy(data, pElements, sizeof(unsigned int) * numElements);

    // Step 13
    vkUnmapMemory(vkDevice, vertexData_elements_fbo.vkDeviceMemory);

    return (vkResult);
}

VkResult createTexture(const char*  textureFileName) {
// variables
    VkResult vkResult = VK_SUCCESS;

    //Code!
    // Step 1: Load Texture Image Information
    FILE *fp = NULL;
    fp = fopen(textureFileName, "rb");
    if(fp == NULL) {
        fprintf(fptr, "createTexture(): Failed to open texture file: %s\n", textureFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    
    uint8_t *image_data = NULL;
    int texture_width, texture_height, texture_channels;

    image_data = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if(image_data == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0) {
        fprintf(fptr, "createTexture(): Failed to load texture image data from stbi_load_from_file for file: %s\n", textureFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fclose(fp);
        return(vkResult);
    }

    VkDeviceSize image_size = texture_width * texture_height * 4; // 4 channels (RGBA)

    fprintf(fptr, "createTexture(): Texture Image Loaded Successfully! Width: %d, Height: %d, Channels: %d, Size: %llu bytes\n", 
        texture_width, texture_height, texture_channels, (unsigned long long)image_size);

    // Step 2: Create Staging Buffer
    VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    
    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = image_size;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vkBuffer_stagingBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateBuffer() Failed for Staging Buffer!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateBuffer() Successful for Staging Buffer!.\n");
    }

    // Get Memory Requirements for Staging Buffer
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);
    
    // Allocate Memory for Staging Buffer
    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1) {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Failed for Staging Buffer!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Successful for Staging Buffer!.\n");
    }

    vkBindBufferMemory(vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBindBufferMemory() Failed for Staging Buffer!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBindBufferMemory() Successful for Staging Buffer!.\n");
    }

    void * data = NULL;

    vkResult = vkMapMemory(
        vkDevice,
        vkDeviceMemory_stagingBuffer,
        0,
        image_size,
        0,
        &data
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkMapMemory() Failed for Staging Buffer!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkMapMemory() Successful for Staging Buffer!.\n");
    }

    memcpy(data, image_data, image_size);

    vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer);
    
    // As Copying of Image Data is done, we can free Image Data from stbi
    stbi_image_free(image_data);
    image_data = NULL;
    fprintf(fptr, "createTexture(): Image Data Copied to Staging Buffer Successful & Freed stbi Image Data!.\n");
    fclose(fp);

    // Step 3: Create VKImage for Texture
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // 2D Image
    vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // RGBA format [To have portability on mobile and desktop we are using UNORM else we can use VK_FORMAT_R8G8B8A8_SRGB]
    vkImageCreateInfo.extent.width = texture_width; 
    vkImageCreateInfo.extent.height = texture_height;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Initial Layout is Undefined as we will be transferring data to it

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_texture_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateImage() Failed for Texture Image!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateImage() Successful for Texture Image!.\n");
    }

    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));

    vkGetImageMemoryRequirements(vkDevice, vkImage_texture_fbo, &vkMemoryRequirements_image);

    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL;
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0; // this will be set in next step

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1) {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_image, NULL, &vkDeviceMemory_texture_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Failed for Texture Image!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateMemory() Successful for Texture Image!.\n");
    }

    vkResult = vkBindImageMemory(vkDevice, vkImage_texture_fbo, vkDeviceMemory_texture_fbo, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBindImageMemory() Failed for Texture Image!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBindImageMemory() Successful for Texture Image!.\n");
    }

    // Step 4: Create Image Transition Layout
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_transition_image_layout;
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;

    VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Failed for Transition Image Layout!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Successful for Transition Image Layout!.\n");
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_transition_image_layout;
        memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));

    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // we will submit this command buffer only once

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Failed for Transition Image Layout!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Successful for Transition Image Layout!.\n");
    }

    VkPipelineStageFlags vkPipelineStageFlags_source = 0;
    VkPipelineStageFlags vkPipelineStageFlags_destination = 0;

    VkImageMemoryBarrier vkImageMemoryBarrier;
    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));

    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = vkImage_texture_fbo;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;

    if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        fprintf(fptr, "createTexture(): Unsuppored Texture Layout Transition!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }

    vkCmdPipelineBarrier(
        vkCommandBuffer_transition_image_layout,
        vkPipelineStageFlags_source,
        vkPipelineStageFlags_destination,
        0, // No flags
        0, NULL, // No memory barriers
        0, NULL, // No buffer barriers
        1, &vkImageMemoryBarrier // Image Barrier
    );

    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Failed for Transition Image Layout!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Successful for Transition Image Layout!.\n");
    }

    VkSubmitInfo vkSubmitInfo_image_transition_layout;
    memset((void*)&vkSubmitInfo_image_transition_layout, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo_image_transition_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_image_transition_layout.pNext = NULL;
    vkSubmitInfo_image_transition_layout.commandBufferCount = 1;
    vkSubmitInfo_image_transition_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;

    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_image_transition_layout, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Failed for Transition Image Layout!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Successful for Transition Image Layout!.\n");
    }

    vkResult = vkQueueWaitIdle(vkQueue);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Failed for Transition Image Layout!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Successful for Transition Image Layout!.\n");
    }

    if(vkCommandBuffer_transition_image_layout){ 
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
        fprintf(fptr, "createTexture(): vkFreeCommandBuffers() Successful for Transition Image Layout!.\n");
        vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    }

    // Step 5: Copy Staging Buffer to Texture Image
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_buffer_to_image_copy;
    memset((void*)&vkCommandBufferAllocateInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandBufferCount = 1;

    VkCommandBuffer vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_buffer_to_image_copy, &vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Failed for Buffer to Image Copy!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Successful for Buffer to Image Copy!.\n");
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_buffer_to_image_copy;
    memset((void*)&vkCommandBufferBeginInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferBeginInfo));

    vkCommandBufferBeginInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferBeginInfo_buffer_to_image_copy.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // we will submit this command buffer only once

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_buffer_to_image_copy, &vkCommandBufferBeginInfo_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Failed for Buffer to Image Copy!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Successful for Buffer to Image Copy!.\n");
    }

    VkBufferImageCopy vkBufferImageCopy;
    memset((void*)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));
    // this is not used a sstandard structure
    vkBufferImageCopy.bufferOffset = 0;
    vkBufferImageCopy.bufferRowLength = 0;
    vkBufferImageCopy.bufferImageHeight = 0;
    vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopy.imageSubresource.mipLevel = 0;
    vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
    vkBufferImageCopy.imageSubresource.layerCount = 1;
    vkBufferImageCopy.imageOffset.x = 0;
    vkBufferImageCopy.imageOffset.y = 0;
    vkBufferImageCopy.imageOffset.z = 0;
    vkBufferImageCopy.imageExtent.width = texture_width;
    vkBufferImageCopy.imageExtent.height = texture_height;
    vkBufferImageCopy.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        vkCommandBuffer_buffer_to_image_copy,
        vkBuffer_stagingBuffer,
        vkImage_texture_fbo,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, // Number of regions
        &vkBufferImageCopy
    );

    vkResult = vkEndCommandBuffer(vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Failed for Buffer to Image Copy!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Successful for Buffer to Image Copy!.\n");
    }

    VkSubmitInfo vkSubmitInfo_image_buffer_to_image_copy;
    memset((void*)&vkSubmitInfo_image_buffer_to_image_copy, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo_image_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_image_buffer_to_image_copy.pNext = NULL;
    vkSubmitInfo_image_buffer_to_image_copy.commandBufferCount = 1;
    vkSubmitInfo_image_buffer_to_image_copy.pCommandBuffers = &vkCommandBuffer_buffer_to_image_copy;

    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_image_buffer_to_image_copy, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Failed for Buffer to Image Copy!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Successful for Buffer to Image Copy!.\n");
    }

    vkResult = vkQueueWaitIdle(vkQueue);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Failed for Buffer to Image Copy!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Successful for Buffer to Image Copy!.\n");
    }

    if(vkCommandBuffer_buffer_to_image_copy){ 
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_buffer_to_image_copy);
        fprintf(fptr, "createTexture(): vkFreeCommandBuffers() Successful for Buffer to Image Copy!.\n");
        vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    }

    // Step 6: Image Layout Transition to Shader Read Only
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;

    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Failed for Transition Image Layout for Shaders!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkAllocateCommandBuffers() Successful for Transition Image Layout for Shaders!.\n");
    }

    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));

    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // we will submit this command buffer only once

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Failed for Transition Image Layout for Shaders!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkBeginCommandBuffer() Successful for Transition Image Layout for Shaders!.\n");
    }

    vkPipelineStageFlags_source = 0;
    vkPipelineStageFlags_destination = 0;

    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));

    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = vkImage_texture_fbo;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;

    if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        fprintf(fptr, "createTexture(): Unsuppored Texture Layout Transition for Shaders!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }

    vkCmdPipelineBarrier(
        vkCommandBuffer_transition_image_layout,
        vkPipelineStageFlags_source,
        vkPipelineStageFlags_destination,
        0, // No flags
        0, NULL, // No memory barriers
        0, NULL, // No buffer barriers
        1, &vkImageMemoryBarrier // Image Barrier
    );

    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Failed for Transition Image Layout for Shaders!.\n");
        return(vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkEndCommandBuffer() Successful for Transition Image Layout for Shaders!.\n");
    }

    memset((void*)&vkSubmitInfo_image_transition_layout, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo_image_transition_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_image_transition_layout.pNext = NULL;
    vkSubmitInfo_image_transition_layout.commandBufferCount = 1;
    vkSubmitInfo_image_transition_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;

    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_image_transition_layout, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Failed for Transition Image Layout for Shaders!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkQueueSubmit() Successful for Transition Image Layout for Shaders!.\n");
    }

    vkResult = vkQueueWaitIdle(vkQueue);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Failed for Transition Image Layout for Shaders!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkQueueWaitIdle() Successful for Transition Image Layout for Shaders!.\n");
    }

    if(vkCommandBuffer_transition_image_layout){ 
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
        fprintf(fptr, "createTexture(): vkFreeCommandBuffers() Successful for Transition Image Layout for Shaders!.\n");
        vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    }

    // Step 7: Remove Local Staging Buffer
    if(vkBuffer_stagingBuffer) {
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL);
        fprintf(fptr, "createTexture(): vkFreeMemory() Successful for Staging Buffer!.\n");
        vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    }
    if(vkBuffer_stagingBuffer) {
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL);
        fprintf(fptr, "createTexture(): vkDestroyBuffer() Successful for Staging Buffer!.\n");
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    }

    // Step 8: Create Image View for Texture
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Same format as Image
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_texture_fbo;

    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkImageView_texture_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateImageView() Failed for Texture Image!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateImageView() Successful for Texture Image!.\n");
    }

    // Step 9: Create Sampler for Texture
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void*)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));

    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR; // Linear Filtering
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Linear Mipmapping
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // Repeat Texture
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE; // Disable Anisotropy
    vkSamplerCreateInfo.maxAnisotropy = 16.0f; // Maximum Anisotropy
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // Border Color
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // Normalized Coordinates
    vkSamplerCreateInfo.compareEnable = VK_FALSE; // Disable Comparison
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    vkResult = vkCreateSampler(vkDevice, &vkSamplerCreateInfo, NULL, &vkSampler_texture_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createTexture(): vkCreateSampler() Failed for Texture Sampler!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createTexture(): vkCreateSampler() Successful for Texture Sampler!.\n");
    }

    return(vkResult);
}

VkResult createUniformBuffer_fbo(void) {
    // functions
    VkResult updateUniformBuffer(void);

    // variables
    VkResult vkResult = VK_SUCCESS;

    memset((void*)&uniformData_fbo, 0, sizeof(UniformData));

    // Step 3
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // No flags, Valid Flags are used in scattered buffer
    vkBufferCreateInfo.size = sizeof(struct MyUniformData_fbo);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    // Setp 4
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData_fbo.vkBuffer);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer_fbo(): vkCreateBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer_fbo(): vkCreateBuffer() Successful!.\n");
    }

    // Step 5
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, uniformData_fbo.vkBuffer, &vkMemoryRequirements);

    // Step 6
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0; // this will be set in next step

    // Step A 
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) {
        // Step B
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1) {
            // Step C
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Step D
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        // Step E
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //Setp 9
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_fbo.vkDeviceMemory);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer_fbo(): vkAllocateMemory() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer_fbo(): vkAllocateMemory() Successful!.\n");
    }

    // Step 10
    vkResult = vkBindBufferMemory(vkDevice, uniformData_fbo.vkBuffer, uniformData_fbo.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer_fbo(): vkBindBufferMemory() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer_fbo(): vkBindBufferMemory() Successful!.\n");
    }

    // call updateUniformBuffer() to fill the uniform buffer with data
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createUniformBuffer_fbo(): updateUniformBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createUniformBuffer_fbo(): updateUniformBuffer() Successful!.\n");
    }

    return (vkResult);
}

VkResult updateUniformBuffer_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    struct MyUniformData_fbo myUniformData;
    memset((void*)&myUniformData, 0, sizeof(struct MyUniformData_fbo));

    myUniformData.modelMatrix = glm::mat4(1.0f);
    glm::mat4 translateMat = glm::mat4(1.0f);
    glm::mat4 rotateMat = glm::mat4(1.0f);
    
    translateMat *= glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0.0f, 0.0f, -2.0f)
    );

    rotateMat *= glm::rotate(
        glm::mat4(1.0f),
        glm::radians(angleTeapot),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    myUniformData.modelMatrix = translateMat * rotateMat;

    myUniformData.viewMatrix = glm::mat4(1.0f);
    myUniformData.projectionMatrix = glm::mat4(1.0f);

    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);

    // Conceptual Change For FBO R2T - Use FBO Width and Height instead of Window Width and Height
    perspectiveProjectionMatrix = glm::perspective(
        glm::radians(45.0f),
        (float)fboWidth / (float)fboHeight,
        0.1f,
        100.0f
    );

    perspectiveProjectionMatrix[1][1] *= -1.0f; // Invert Y axis for Vulkan

    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    // Update Lighting Related Uniform
    myUniformData.lightAmbient[0] = 0.4f;
    myUniformData.lightAmbient[1] = 0.4f;
    myUniformData.lightAmbient[2] = 0.4f;
    myUniformData.lightAmbient[3] = 1.0f;

    myUniformData.lightDiffuse[0] = 1.0f;
    myUniformData.lightDiffuse[1] = 1.0f;
    myUniformData.lightDiffuse[2] = 1.0f;
    myUniformData.lightDiffuse[3] = 1.0f;

    myUniformData.lightSpecular[0] = 1.0f;
    myUniformData.lightSpecular[1] = 1.0f;
    myUniformData.lightSpecular[2] = 1.0f;
    myUniformData.lightSpecular[3] = 1.0f;

    myUniformData.lightPosition[0] = 100.0f;
    myUniformData.lightPosition[1] = 100.0f;
    myUniformData.lightPosition[2] = 100.0f;
    myUniformData.lightPosition[3] = 1.0f;

    myUniformData.materialAmbient[0] = 0.9f;
    myUniformData.materialAmbient[1] = 0.5f;
    myUniformData.materialAmbient[2] = 0.3f;
    myUniformData.materialAmbient[3] = 1.0f;

    myUniformData.materialDiffuse[0] = 0.9f;
    myUniformData.materialDiffuse[1] = 0.5f;
    myUniformData.materialDiffuse[2] = 0.3f;
    myUniformData.materialDiffuse[3] = 1.0f;

    myUniformData.materialSpecular[0] = 0.8f;
    myUniformData.materialSpecular[1] = 0.8f;
    myUniformData.materialSpecular[2] = 0.8f;
    myUniformData.materialSpecular[3] = 1.0f;

    myUniformData.materialShininess = 128.0f;

    if(bLight) {
        myUniformData.lKeyPressed = 1;
    } else {
        myUniformData.lKeyPressed = 0;
    }

    if(bTexture) {
        myUniformData.textureEnabled = 1;
    } else {
        myUniformData.textureEnabled = 0;
    }

    void *data = NULL;

    vkResult = vkMapMemory(
        vkDevice,
        uniformData_fbo.vkDeviceMemory,
        0,
        sizeof(struct MyUniformData_fbo),
        0,
        &data
    );
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "updateUniformBuffer_fbo(): vkMapMemory() Failed!.\n");
        return (vkResult);
    }

    memcpy(data, &myUniformData, sizeof(struct MyUniformData_fbo));

    vkUnmapMemory(vkDevice, uniformData_fbo.vkDeviceMemory);

    // Free Data / Set it to NULL
    data = NULL;

    return (vkResult);
}

VkResult createShaders_fbo(void) {

    // variables
    VkResult vkResult = VK_SUCCESS;

    // for vertex shader
    const char* szFileName = "shader_teapot.vert.spv";
    FILE *fp = NULL;
    size_t fileSize = 0;

    fp = fopen(szFileName, "rb");
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders_fbo(): fopen() failed to open Vertex Shader spir-v file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders_fbo(): fopen() succeed to open Vertex Shader spir-v file!.\n");
    }

    fseek(fp, 0l, SEEK_END);
    fileSize = ftell(fp);
    if(fileSize == 0) {
        fprintf(fptr, "createShaders_fbo(): ftell() gave file size 0.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } 
    fseek(fp, 0l, SEEK_SET);

    char *shaderData = (char*)malloc(fileSize * sizeof(char));

    size_t retVal = fread(shaderData, fileSize, 1, fp);
    if(retVal != 1) {
        fprintf(fptr, "createShaders_fbo(): fread() failed to read Vertex Shader file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders_fbo(): fread() succeed to read Vertex Shader file!.\n");
    }
    fclose(fp);
    fp = NULL;

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = fileSize;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders_fbo(): vkCreateShaderModule() for Vertex Shader Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders_fbo(): vkCreateShaderModule() for Vertex Shader Successful!.\n");
    }

    if(shaderData) {
        free(shaderData);
        shaderData = NULL;
    }
    fprintf(fptr, "createShaders_fbo(): Vertex Shader Module Created Successful!.\n");

    // for fragment shader
    szFileName = "shader_teapot.frag.spv";
    fileSize = 0;

    fp = fopen(szFileName, "rb");
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders_fbo(): fopen() failed to open Fragment Shader spir-v file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders_fbo(): fopen() succeed to open Fragment Shader spir-v file!.\n");
    }

    fseek(fp, 0l, SEEK_END);
    fileSize = ftell(fp);
    if(fileSize == 0) {
        fprintf(fptr, "createShaders_fbo(): ftell() gave file size: 0.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    fseek(fp, 0l, SEEK_SET);

    shaderData = (char*)malloc(fileSize * sizeof(char));

    retVal = fread(shaderData, fileSize, 1, fp);
    if(retVal != 1) {
        fprintf(fptr, "createShaders_fbo(): fread() failed to read Fragment Shader file!.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders_fbo(): fread() succeed to read Fragment Shader file!.\n");
    }
    fclose(fp);
    fp = NULL;

    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = fileSize;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createShaders_fbo(): vkCreateShaderModule() for Fragment Shader Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createShaders_fbo(): vkCreateShaderModule() for Fragment Shader Successful!.\n");
    }

    if(shaderData) {
        free(shaderData);
        shaderData = NULL;
    }
    fprintf(fptr, "createShaders_fbo(): Fragment Shader Module Created Successful!.\n");

    return (vkResult);
}

VkResult createDescriptorSetLayout_fbo(void) {
   // Variables
    VkResult vkResult = VK_SUCCESS;

    // Initialize Descriptor Set Binding
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[2];
    memset((void*)vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));

    // 1st element is for uniform
    vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding_array[0].binding = 0; // this 0 is  the binding index, we will use this index in shader
    vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1; 
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // this binding will be used in vertex shader
    vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL; // we don't have any immutable samplers for now

    // 2nd element is for texture image & sampler
    vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[1].binding = 1; // this 0 is  the binding index, we will use this index in shader
    vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1; 
    vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // this binding will be used in fragment shader
    vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL; // we don't have any immutable samplers for now

    //Create Descriptor Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = _ARRAYSIZE(vkDescriptorSetLayoutBinding_array); // we will atleast have one binding
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array; // we will atleast have one binding
    
    // Create Descriptor Set Layout
    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createDescriptorSetLayout_fbo(): vkCreateDescriptorSetLayout() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createDescriptorSetLayout_fbo(): vkCreateDescriptorSetLayout() Successful!.\n");
    }

    return (vkResult);
}

VkResult createPipelineLayout_fbo(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Pipeline Layout Create Info
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1; // we have only one descriptor set layout
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout_fbo;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0; // no push constant range for now
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    // Create Pipeline Layout
    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createPipelineLayout_fbo(): vkCreatePipelineLayout() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createPipelineLayout_fbo(): vkCreatePipelineLayout() Successful!.\n");
    }

    return (vkResult);
}

VkResult createDescriptorPool_fbo(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Descriptor Pool Create Info
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    memset((void*)vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));

    // for mvp uniforms
    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1; // we have only one uniform buffer

    // for texture sampler
    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize_array[1].descriptorCount = 1; // we have only one texture sampler

    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.maxSets = 2;
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;

    // Create Descriptor Pool
    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createDescriptorPool_fbo(): vkCreateDescriptorPool() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createDescriptorPool_fbo(): vkCreateDescriptorPool() Successful!.\n");
    }

    return (vkResult);
}

VkResult createDescriptorSet_fbo(void) { 
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Descriptor Set Allocate Info
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool_fbo;
    // though we have two descriptors one for mvp uniform and another for texture sampler, both are in onesame  descriptor set hence we will keep the count as 1 
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1; // we have only one descriptor set
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout_fbo; // we have only one descriptor set layout

    // Allocate Descriptor Set 
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createDescriptorSet_fbo(): vkAllocateDescriptorSets() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createDescriptorSet_fbo(): vkAllocateDescriptorSets() Successful!.\n");
    }

    // Describe whether we want buffer or image as uniform
    // we want buffer as uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));

    // for mvp uniform
    vkDescriptorBufferInfo.buffer = uniformData_fbo.vkBuffer; // this is the buffer we want to use as uniform
    vkDescriptorBufferInfo.offset = 0; // offset is 0
    vkDescriptorBufferInfo.range = sizeof(struct MyUniformData_fbo); // range is size of uniform

    // for texture sampler
    VkDescriptorImageInfo vkDescriptorImageInfo;
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));

    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_fbo;
    vkDescriptorImageInfo.sampler = vkSampler_texture_fbo;

    // Now update the descriptor set with the buffer directly to the shader
    // we will write to the shader
    // for above two structures we are making it an array of two
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].pNext = NULL;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_fbo; // this is the descriptor set we want to update
    vkWriteDescriptorSet_array[0].dstArrayElement = 0; // we have only one descriptor set, so array element is 0
    vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are only gonna write one descriptor set
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL; // we'll use this during texture
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // using for tiling of texture but we're not using it now
    vkWriteDescriptorSet_array[0].dstBinding = 0; // this is the binding index we used in descriptor set layout & shader

    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].pNext = NULL;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_fbo; // this is the descriptor set we want to update
    vkWriteDescriptorSet_array[1].dstArrayElement = 0; // we have only one descriptor set, so array element is 0
    vkWriteDescriptorSet_array[1].descriptorCount = 1; // we are only gonna write one descriptor set
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo; // we'll use this during texture
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL; // using for tiling of texture but we're not using it now
    vkWriteDescriptorSet_array[1].dstBinding = 1; // this is the binding index we used in descriptor set layout & shader

    // Update Descriptor Set
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL); 
    // we have only one descriptor set to update, so count is 1
    // last two parameters are for copy descriptor sets, which are used while copying

    fprintf(fptr, "createDescriptorSet_fbo(): vkUpdateDescriptorSets() Successful!.\n");

    return (vkResult);
}

VkResult createRenderPass_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Code
    //Step 1: Create Attachment Description stcture array
    VkAttachmentDescription vkAttachmentDescription_array[2];
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format =  vkFormat_color_fbo;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // this is changed from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for FBO R2T
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // For Depth Attachment
    vkAttachmentDescription_array[1].flags = 0;
    vkAttachmentDescription_array[1].format =  vkFormat_depth_fbo;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Step 2: Create Attachment Reference Structure
    VkAttachmentReference vkAttachmentReference_color;
    memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference_color.attachment = 0; // From the array of attachment description, refer to 0th index, oth will be color attachment
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 

    // Create Attachment Reference for Depth Attachment
    VkAttachmentReference vkAttachmentReference_depth;
    memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference_depth.attachment = 1; // From the array of attachment description, refer to 1st index, 1st will be depth attachment
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Depth Attachment Layout

    // STep 3: create sub pass description strcture
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1; // this count should be count of vkAttachmentReference array
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth; // this is the depth attachment reference;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;

    // Step 4: Render Pass Create Info
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;
    

    // Step 5: Create Render Pass
    vkResult = vkCreateRenderPass(
        vkDevice,
        &vkRenderPassCreateInfo,
        NULL,
        &vkRenderPass_fbo
    );

    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createRenderPass_fbo(): vkCreateRenderPass() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createRenderPass_fbo(): vkCreateRenderPass() Successful!.\n");
    }

    return (vkResult);
}

VkResult createPipeline_fbo(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Vertex Input Binding Description [ Vertex Input State]
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[3];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    vkVertexInputBindingDescription_array[0].binding = AMK_ATTRIBUTE_POSITION; // 0th binding index for position
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vkVertexInputBindingDescription_array[1].binding = AMK_ATTRIBUTE_NORMAL; // 1st binding index for normal
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vkVertexInputBindingDescription_array[2].binding = AMK_ATTRIBUTE_TEXCOORD; // 2nd binding index for texcoord
    vkVertexInputBindingDescription_array[2].stride = sizeof(float) * 2;
    vkVertexInputBindingDescription_array[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[3];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    // Position Attribute
    vkVertexInputAttributeDescription_array[0].binding = AMK_ATTRIBUTE_POSITION;
    vkVertexInputAttributeDescription_array[0].location = AMK_ATTRIBUTE_POSITION;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;

    vkVertexInputAttributeDescription_array[1].binding = AMK_ATTRIBUTE_NORMAL;
    vkVertexInputAttributeDescription_array[1].location = AMK_ATTRIBUTE_NORMAL;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;

    vkVertexInputAttributeDescription_array[2].binding = AMK_ATTRIBUTE_TEXCOORD;
    vkVertexInputAttributeDescription_array[2].location = AMK_ATTRIBUTE_TEXCOORD;
    vkVertexInputAttributeDescription_array[2].format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexInputAttributeDescription_array[2].offset = 0;
    
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    // Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization State
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; // No culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

    // Color Blending State
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;


    // Viewport Scissor State
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;

    // Set the viewport/s
    vkPipelineViewportStateCreateInfo.viewportCount = 1;

    // CONCEPTUAL CHANGE: Use FBO Width and Height instead of swapchain width and height
    memset((void*)&vkViewport_fbo, 0, sizeof(VkViewport));
    vkViewport_fbo.x = 0;
    vkViewport_fbo.y = 0;
    vkViewport_fbo.width = (float)fboWidth;
    vkViewport_fbo.height = (float)fboHeight;
    vkViewport_fbo.minDepth = 0.0f;
    vkViewport_fbo.maxDepth = 1.0f;

    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport_fbo;

    // Set the scissor rect/s
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    
    memset((void*)&vkRect2D_scissor_fbo, 0, sizeof(VkRect2D));
    vkRect2D_scissor_fbo.offset.x = 0;
    vkRect2D_scissor_fbo.offset.y = 0;
    vkRect2D_scissor_fbo.extent.width = fboWidth;
    vkRect2D_scissor_fbo.extent.height = fboHeight;

    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor_fbo;

    // Depth Stencil State
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));

    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // front and back are same

    // Dynamic State
    // We don't have any dynamic state;

    // Multisample State
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


    // Shader Stage State
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

    // Vertex Shader Stage
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_fbo;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

    // Fragment Shader Stage
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_fbo;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;


    // Tessellation State
    // We don't have tessellation shaders so we can skip this state


    // Pipelines are created in a pipeline cache, we will create Pipeline cache object
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;

    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createPipeline_fbo(): vkCreatePipelineCache() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createPipeline_fbo(): vkCreatePipelineCache() Successful!.\n");
    }

    // Create Graphics Pipeline
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;   
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // we don't have dynamic state
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout_fbo;
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass_fbo;
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline handle
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0; // no base pipeline index

    // Create Graphics Pipeline
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createPipeline_fbo(): vkCreateGraphicsPipelines() Failed!.\n");
        // Destroy Pipeline Cache
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        return (vkResult);
    } else {
        fprintf(fptr, "createPipeline_fbo(): vkCreateGraphicsPipelines() Successful!.\n");
    }

    // Destroy Pipeline Cache
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;

    return (vkResult);
}

VkResult createFramebuffer_fbo(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Step 1: create VkImageView array for color and depth attachments
    VkImageView vkImageView_attachments_array[2];
    memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

    // Step 2: Create VkFrameBufferCreateInfo structure
    VkFramebufferCreateInfo vkFrameBufferCreateInfo;
    memset((void*)&vkFrameBufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

    vkFrameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFrameBufferCreateInfo.flags = 0;
    vkFrameBufferCreateInfo.pNext = NULL;
    vkFrameBufferCreateInfo.renderPass = vkRenderPass_fbo;
    vkFrameBufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
    vkFrameBufferCreateInfo.pAttachments = vkImageView_attachments_array;
    vkFrameBufferCreateInfo.width = fboWidth;
    vkFrameBufferCreateInfo.height = fboHeight;
    vkFrameBufferCreateInfo.layers = 1; // VALIDATION USE CASE 2: Comment this line to see the error

    vkImageView_attachments_array[0] = vkImageView_fbo;
    vkImageView_attachments_array[1] = vkImageView_depth_fbo; // this is the depth attachment image view

    vkResult = vkCreateFramebuffer(vkDevice, &vkFrameBufferCreateInfo, NULL, &vkFramebuffer_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createFramebuffer_fbo(): vkCreateFramebuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createFramebuffer_fbo(): vkCreateFramebuffer() Successful!.\n");
    }

    return (vkResult);
}

VkResult createSemaphore_fbo(void) {
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Create Semaphore info
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;
    vkSemaphoreCreateInfo.flags = 0; // it's reserved must be zero

    // By defualt if no type is specified, binary semaphore is created!

    // create semaphore for backbuffer
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "createSemaphore_fbo(): vkCreateSemaphore() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "createSemaphore_fbo(): vkCreateSemaphore() Successful!.\n");
    }

    return (vkResult);
}

VkResult buildCommandBuffer_fbo(void) {
    // variables
    VkResult vkResult = VK_SUCCESS;

    // Reset Command Buffers
    vkResult = vkResetCommandBuffer(vkCommandBuffer_fbo, 0); 
    // adding 0 here means sdon't release resources allocated by command pool
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "buildCommandBuffer_fbo(): vkResetCommandBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "buildCommandBuffer_fbo(): vkResetCommandBuffer() Successful!.\n");
    }

    // set VkCommandBufferBeginInfo
    VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
    memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

    vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo.pNext = NULL;
    vkCommandBufferBeginInfo.flags = 0; 
    // Zero indicates that we'll use primary command buffer and also specifying that we are not
    // using this buffer simultaniously between multiple threads

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_fbo, &vkCommandBufferBeginInfo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "buildCommandBuffer_fbo(): vkBeginCommandBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "buildCommandBuffer_fbo(): vkBeginCommandBuffer() Successful!.\n");
    }

    // Set Clear Values
    VkClearValue vkClearValue_array[2];
    memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

    vkClearValue_array[0].color = vkClearColorValue_fbo;
    vkClearValue_array[1].depthStencil = vkClearDepthStencilValue_fbo;

    // Render pass begin info
    VkRenderPassBeginInfo vkRenderPassBeginInfo;
    memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

    vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vkRenderPassBeginInfo.pNext = NULL;
    vkRenderPassBeginInfo.renderPass = vkRenderPass_fbo;
    vkRenderPassBeginInfo.renderArea.offset.x = 0;
    vkRenderPassBeginInfo.renderArea.offset.y = 0;
    vkRenderPassBeginInfo.renderArea.extent.width = fboWidth;
    vkRenderPassBeginInfo.renderArea.extent.height = fboHeight;
    vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
    vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
    vkRenderPassBeginInfo.framebuffer = vkFramebuffer_fbo;

    // begin render pass
    vkCmdBeginRenderPass(vkCommandBuffer_fbo, &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    // content of this pass are subpass and part of primary command buffers so inline

    // Bind with the pipeline
    vkCmdBindPipeline(vkCommandBuffer_fbo, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline_fbo);

    // Bind Descriptor Set
    vkCmdBindDescriptorSets(
        vkCommandBuffer_fbo,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vkPipelineLayout_fbo,
        0, 1,
        &vkDescriptorSet_fbo, // this is the descriptor set we want to bind
        0, NULL
    );

    // Bind with the vertex buffer
    VkDeviceSize vkDeviceSize_offset_position_array[1];
    memset((void*)vkDeviceSize_offset_position_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position_array));

    vkCmdBindVertexBuffers(
        vkCommandBuffer_fbo, 
        AMK_ATTRIBUTE_POSITION, 1,
        &vertexData_position_fbo.vkBuffer,
        vkDeviceSize_offset_position_array
    );

    VkDeviceSize vkDeviceSize_offset_normal_array[1];
    memset((void*)vkDeviceSize_offset_normal_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal_array));

    vkCmdBindVertexBuffers(
        vkCommandBuffer_fbo, 
        AMK_ATTRIBUTE_NORMAL, 1,
        &vertexData_normal_fbo.vkBuffer,
        vkDeviceSize_offset_normal_array
    );

    VkDeviceSize vkDeviceSize_offset_texcoord_array[1];
    memset((void*)vkDeviceSize_offset_texcoord_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord_array));

    vkCmdBindVertexBuffers(
        vkCommandBuffer_fbo, 
        AMK_ATTRIBUTE_TEXCOORD, 1,
        &vertexData_texcoord_fbo.vkBuffer,
        vkDeviceSize_offset_texcoord_array
    );

        // Bind our index buffer
    vkCmdBindIndexBuffer(
        vkCommandBuffer_fbo,
        vertexData_elements_fbo.vkBuffer,
        0, // offset is 0
        VK_INDEX_TYPE_UINT32 // we are using 16 bit uint index matches with declared array in vertexData_position_index
    );

    // Here we should call vulkan drawing functions!
    // Draw Indexed
    vkCmdDrawIndexed(
        vkCommandBuffer_fbo,
        numElements, // number of indices to draw
        1, // number of instances to draw
        0, // first index to draw from index buffer
        0, // if we want to add offset to starting index of vertex buffer, we can add here
        1  // nth instance, but we have only one instance so 1 
    );

    // End Render Pass
    vkCmdEndRenderPass(vkCommandBuffer_fbo);

    // End command buffer recording
    vkResult = vkEndCommandBuffer(vkCommandBuffer_fbo);
    if(vkResult != VK_SUCCESS) {
        fprintf(fptr, "buildCommandBuffer_fbo(): vkEndCommandBuffer() Failed!.\n");
        return (vkResult);
    } else {
        fprintf(fptr, "buildCommandBuffer_fbo(): vkEndCommandBuffer() Successful!.\n");
    }

    return (vkResult);
}


// Teapot Reaalted Functions
void addTriangle(float single_vertex[3][3], float single_normal[3][3], float single_texCoord[3][2]) {
	// function declarations
	BOOL closeEnough(const float, const float, const float);
	void normalizeVector(float[3]);

	// code
	unsigned int maxElements = numFaceIndices * 3;
	const float e = 0.00001f; // How small a difference to equate

	// First thing we do is make sure the normals are unit length!
	// It's almost always a good idea to work with pre-normalized normals
	normalizeVector(single_normal[0]);
	normalizeVector(single_normal[1]);
	normalizeVector(single_normal[2]);

	// Search for match - triangle consists of three verts
	for (unsigned int i = 0; i < 3; i++) {
		unsigned int j = 0;
		for (j = 0; j < numVerts; j++) {
			// If the vertex positions are the same
			if (closeEnough(pPositions[j * 3], single_vertex[i][0], e) &&
				closeEnough(pPositions[(j * 3) + 1], single_vertex[i][1], e) &&
				closeEnough(pPositions[(j * 3) + 2], single_vertex[i][2], e) &&

				// AND the Normal is the same...
				closeEnough(pNormals[j * 3], single_normal[i][0], e) &&
				closeEnough(pNormals[(j * 3) + 1], single_normal[i][1], e) &&
				closeEnough(pNormals[(j * 3) + 2], single_normal[i][2], e) &&

				// And Texture is the same...
				closeEnough(pTexCoords[j * 2], single_texCoord[i][0], e) &&
				closeEnough(pTexCoords[(j * 2) + 1], single_texCoord[i][1], e))
			{
				// Then add the index only
				pElements[numElements] = j;
				numElements++;
				break;
			}
		}

		// No match for this vertex, add to end of list
		if (j == numVerts && numVerts < maxElements && numElements < maxElements) {
			pPositions[numVerts * 3] = single_vertex[i][0];
			pPositions[(numVerts * 3) + 1] = single_vertex[i][1];
			pPositions[(numVerts * 3) + 2] = single_vertex[i][2];

			pNormals[numVerts * 3] = single_normal[i][0];
			pNormals[(numVerts * 3) + 1] = single_normal[i][1];
			pNormals[(numVerts * 3) + 2] = single_normal[i][2];

			pTexCoords[numVerts * 2] = single_texCoord[i][0];
			pTexCoords[(numVerts * 2) + 1] = single_texCoord[i][1];

			pElements[numElements] = numVerts;
			numElements++;
			numVerts++;
		}
	}
}

void normalizeVector(float u[3]) {
	// function declarations
	void scaleVector(float[3], const float);
	float getVectorLength(const float[3]);

	// code
	scaleVector(u, 1.0f / getVectorLength(u));
}

void scaleVector(float v[3], const float scale) {
	// code
	v[0] *= scale;
	v[1] *= scale;
	v[2] *= scale;
}

float getVectorLength(const float u[3]) {
	// function declarations
	float getVectorLengthSquared(const float[3]);

	// code
	return(sqrtf(getVectorLengthSquared(u)));
}

float getVectorLengthSquared(const float u[3]) {
	// code
	return((u[0] * u[0]) + (u[1] * u[1]) + (u[2] * u[2]));
}

BOOL closeEnough(const float fCandidate, const float fCompare, const float fEpsilon) {
	// code
	return((fabs(fCandidate - fCompare) < fEpsilon));
}