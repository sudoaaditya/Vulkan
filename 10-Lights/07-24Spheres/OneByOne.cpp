 /* // First Column
        // First Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 50.0f * (float)winWidth / 800.0f;
        vkViewport.y = 30.0f * (float)winHeight / 600.0f;
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;
        
        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient = glm::vec4(0.0215f, 0.1745f, 0.0215f, 1.0f);
        pushConstantData_material.materialDiffuse = glm::vec4(0.07568f, 0.61424f, 0.07568f, 1.0f);
        pushConstantData_material.materialSpecular = glm::vec4(0.633f, 0.727811f, 0.633f, 1.0f);
        pushConstantData_material.materialShininess =  0.6f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        //Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // First Column
        // second Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 50.0f * (float)winWidth / 800.0f;
        vkViewport.y = 102.0f * (float)winHeight / 600.0f;
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;
        
        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
        pushConstantData_material.materialDiffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
        pushConstantData_material.materialSpecular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
        pushConstantData_material.materialShininess =  0.1f * 12.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );
        
        //Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // First Column
        // Third Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 50.0f * (float)winWidth / 800.0f;
        vkViewport.y = 174.0f * (float)winHeight / 600.0f;
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;
        
        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient = glm::vec4(0.05375f, 0.05f, 0.06625f, 1.0f);
        pushConstantData_material.materialDiffuse = glm::vec4(0.18275f, 0.17f, 0.22525f, 1.0f);
        pushConstantData_material.materialSpecular = glm::vec4(0.332741f, 0.328634f, 0.34643f, 1.0f);
        pushConstantData_material.materialShininess =  0.3f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );
        
        //Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // First Column
        // Fourth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 50.0f * (float)winWidth / 800.0f;
        vkViewport.y = 246.0f * (float)winHeight / 600.0f;
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;
        
        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient = glm::vec4(0.25f, 0.20725f, 0.20725f , 1.0f);
        pushConstantData_material.materialDiffuse = glm::vec4(1.0f, 0.829f, 0.829f , 1.0f);
        pushConstantData_material.materialSpecular = glm::vec4(0.296648f, 0.296648f, 0.296648f , 1.0f);
        pushConstantData_material.materialShininess =  0.088f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );
        
        //Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);


        // First Column
        // Fifth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 50.0f * (float)winWidth / 800.0f;
        vkViewport.y = 318.0f * (float)winHeight / 600.0f;
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;
        
        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
        pushConstantData_material.materialDiffuse = glm::vec4(0.61424f, 0.04136f, 0.04236f, 1.0f);
        pushConstantData_material.materialSpecular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
        pushConstantData_material.materialShininess = 0.6f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );
        
        //Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // First Column
        // Sixth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 50.0f * (float)winWidth / 800.0f;
        vkViewport.y = 390.0f * (float)winHeight / 600.0f;
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;
        
        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient = glm::vec4(0.1f, 0.18725f, 0.1745f, 1.0f);
        pushConstantData_material.materialDiffuse = glm::vec4(0.396f, 0.74151f, 0.69102f, 1.0f);
        pushConstantData_material.materialSpecular = glm::vec4(0.297254f, 0.30829f, 0.306678f, 1.0f);
        pushConstantData_material.materialShininess =  0.1f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );
        
        //Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Second Column
        // First Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 250.0f * (float)winWidth / 800.0f; // column 2
        vkViewport.y = 30.0f * (float)winHeight / 600.0f;           // row 1
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Brass)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
        pushConstantData_material.materialShininess = 0.21794872f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Second Column
        // Second Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 250.0f * (float)winWidth / 800.0f;
        vkViewport.y = 102.0f * (float)winHeight / 600.0f; // row 2
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Bronze)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.2125f, 0.1275f, 0.054f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.714f, 0.4284f, 0.18144f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.393548f, 0.271906f, 0.166721f, 1.0f);
        pushConstantData_material.materialShininess = 0.2f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Second Column
        // Third Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 250.0f * (float)winWidth / 800.0f;
        vkViewport.y = 174.0f * (float)winHeight / 600.0f; // row 3
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Chrome)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
        pushConstantData_material.materialShininess = 0.6f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Second Column
        // Fourth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 250.0f * (float)winWidth / 800.0f;
        vkViewport.y = 246.0f * (float)winHeight / 600.0f; // row 4
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Copper)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
        pushConstantData_material.materialShininess = 0.1f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Second Column
        // Fifth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 250.0f * (float)winWidth / 800.0f;
        vkViewport.y = 318.0f * (float)winHeight / 600.0f; // row 5
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Gold)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
        pushConstantData_material.materialShininess = 0.4f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Second Column
        // Sixth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 250.0f * (float)winWidth / 800.0f;
        vkViewport.y = 390.0f * (float)winHeight / 600.0f; // row 6
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Silver)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.19225f, 0.19225f, 0.19225f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.50754f, 0.50754f, 0.50754f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.508273f, 0.508273f, 0.508273f, 1.0f);
        pushConstantData_material.materialShininess = 0.4f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        //!
        // Third Column
        // First Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 450.0f * (float)winWidth / 800.0f;
        vkViewport.y = 30.0f * (float)winHeight / 600.0f; // row 2
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Black Plastic)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.50f, 0.50f, 0.50f, 1.0f);
        pushConstantData_material.materialShininess = 0.25f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Third Column
        // Second Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 450.0f * (float)winWidth / 800.0f;
        vkViewport.y = 102.0f * (float)winHeight / 600.0f; // row 2
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Cyan Plastic)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.1f, 0.06f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.0f, 0.5098039f, 0.5098039f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.5019608f, 0.5019608f, 0.5019608f, 1.0f);
        pushConstantData_material.materialShininess = 0.25f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Third Column
        // Third Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 450.0f * (float)winWidth / 800.0f;
        vkViewport.y = 174.0f * (float)winHeight / 600.0f; // row 3
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant ( Green Plastic)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.1f, 0.35f, 0.1f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.45f, 0.55f, 0.45f, 1.0f);
        pushConstantData_material.materialShininess = 0.25f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Third Column
        // Fourth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 450.0f * (float)winWidth / 800.0f;
        vkViewport.y = 246.0f * (float)winHeight / 600.0f; // row 4
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Red Plastic)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.7f, 0.6f, 0.6f, 1.0f);
        pushConstantData_material.materialShininess = 0.25f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Third Column
        // Fifth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 450.0f * (float)winWidth / 800.0f;
        vkViewport.y = 318.0f * (float)winHeight / 600.0f; // row 5
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (White Plastic)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.55f, 0.55f, 0.55f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.70f, 0.70f, 0.70f, 1.0f);
        pushConstantData_material.materialShininess = 0.25f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Third Column
        // Sixth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 450.0f * (float)winWidth / 800.0f;
        vkViewport.y = 390.0f * (float)winHeight / 600.0f; // row 6
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Yellow Plastic)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.60f, 0.60f, 0.50f, 1.0f);
        pushConstantData_material.materialShininess = 0.25f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        //!
        // Fourth Column
        // First Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 650.0f * (float)winWidth / 800.0f;
        vkViewport.y = 30.0f * (float)winHeight / 600.0f; // row 2
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Black Rubber)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.02f, 0.02f, 0.02f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
        pushConstantData_material.materialShininess = 0.078125f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Fourth Column
        // Second Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 650.0f * (float)winWidth / 800.0f;
        vkViewport.y = 102.0f * (float)winHeight / 600.0f; // row 2
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Cyan Rubber)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.05f, 0.05f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.4f, 0.5f, 0.5f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.04f, 0.7f, 0.7f, 1.0f);
        pushConstantData_material.materialShininess = 0.078125f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Fourth Column
        // Third Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 650.0f * (float)winWidth / 800.0f;
        vkViewport.y = 174.0f * (float)winHeight / 600.0f; // row 3
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant ( Green Rubber)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.0f, 0.04f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.4f, 0.5f, 0.4f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.04f, 0.5f, 0.04f, 1.0f);
        pushConstantData_material.materialShininess = 0.078125f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Fourth Column
        // Fourth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 650.0f * (float)winWidth / 800.0f;
        vkViewport.y = 246.0f * (float)winHeight / 600.0f; // row 4
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Red Rubber)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.05f, 0.0f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.5f, 0.4f, 0.4f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.7f, 0.04f, 0.04f, 1.0f);
        pushConstantData_material.materialShininess = 0.078125f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Fourth Column
        // Fifth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 650.0f * (float)winWidth / 800.0f;
        vkViewport.y = 318.0f * (float)winHeight / 600.0f; // row 5
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (White Rubber)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
        pushConstantData_material.materialShininess = 0.078125f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0);

        // Fourth Column
        // Sixth Sphere
        memset((void*)&vkViewport, 0, sizeof(VkViewport));
        vkViewport.x = 650.0f * (float)winWidth / 800.0f;
        vkViewport.y = 390.0f * (float)winHeight / 600.0f; // row 6
        vkViewport.width = (float)winWidth / 8.0f;
        vkViewport.height = (float)winHeight / 8.0f;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 1.0f;

        vkCmdSetViewport(vkCommandBuffer_array[i], 0, 1, &vkViewport);

        // Material Push Constant (Yellow Rubber)
        memset((void*)&pushConstantData_material, 0, sizeof(PushConstantData_Material));

        pushConstantData_material.materialAmbient   = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
        pushConstantData_material.materialDiffuse   = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
        pushConstantData_material.materialSpecular  = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
        pushConstantData_material.materialShininess = 0.078125f * 128.0f;

        vkCmdPushConstants(
            vkCommandBuffer_array[i],
            vkPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData_Material),
            &pushConstantData_material
        );

        // Draw
        vkCmdDrawIndexed(vkCommandBuffer_array[i], numElements, 1, 0, 0, 0); */