#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define PI 3.14159265
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "client/vertex.h"
#include "client/entity.h"
#include "client/uniform.h"
#include "client/terrain.h"
#include "client/engine.h"
#include "client/shaderloader.h"
#include <math.h>
#include <chrono>
#include <thread>
int main(){
	Engine *engineInstance = new Engine("Test Engine",1280,720);
	engineInstance->ENABLE_VALIDATION_LAYERS=true;
	engineInstance->initVulkan();
	engineInstance->initTerrain(6);

	glm::vec3 camPos(0,0,0);
	glm::vec2 eyeAngles(0,0);
	glm::vec3 eyeDirection(0,0,0);
	glm::vec3 sideDirection(0,0,0);
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	glfwSetInputMode(engineInstance->window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
	bool FORWARD=false;
	bool BACK=false;
	bool RIGHT=false;
	bool LEFT=false;
	bool UP=false;
	bool DOWN=false;
	bool SPRINT=false;
	float speedMultiplier;
	while(!glfwWindowShouldClose(engineInstance->window)){
		int timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		start = std::chrono::system_clock::now();
		if (timeElapsed<10){
			std::this_thread::sleep_for((std::chrono::duration<double, std::milli>)(10-timeElapsed));
		}
		glfwPollEvents();
		FORWARD=glfwGetKey(engineInstance->window,GLFW_KEY_W)==GLFW_PRESS;
		BACK=glfwGetKey(engineInstance->window,GLFW_KEY_S)==GLFW_PRESS;
		LEFT=glfwGetKey(engineInstance->window,GLFW_KEY_A)==GLFW_PRESS;
		RIGHT=glfwGetKey(engineInstance->window,GLFW_KEY_D)==GLFW_PRESS;
		UP=glfwGetKey(engineInstance->window,GLFW_KEY_SPACE)==GLFW_PRESS;
		DOWN=glfwGetKey(engineInstance->window,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS;
		SPRINT=glfwGetKey(engineInstance->window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS;
		speedMultiplier=(0.1+SPRINT*0.4)*timeElapsed/10;
		double mx;
		double my;

		glfwGetCursorPos(engineInstance->window,&mx,&my);
		eyeAngles.x+=(mx-engineInstance->extent.width/2)*0.001;
		if (not ((eyeAngles.y>=PI/2 and (my-engineInstance->extent.height/2) > 0) or (eyeAngles.y<=-PI/2 and (my-engineInstance->extent.height/2) <0))){
			eyeAngles.y+=(my-engineInstance->extent.height/2)*0.001;
		}
		glfwSetCursorPos(engineInstance->window,engineInstance->extent.width/2,engineInstance->extent.height/2);
		while (eyeAngles.x>=2*PI){
			eyeAngles.x-=2*PI;
		}
		while (eyeAngles.x<=0){
			eyeAngles.x+=2*PI;
		}

		eyeDirection = glm::vec3(cos(eyeAngles.x),-sin(eyeAngles.y),sin(eyeAngles.x));
		sideDirection = glm::vec3(cos(eyeAngles.x+PI/2),0,sin(eyeAngles.x+PI/2));
		float forward_speed=(FORWARD-BACK)*speedMultiplier;
		float side_speed=(RIGHT-LEFT)*speedMultiplier;
		float height_speed=(UP-DOWN)*speedMultiplier;
		camPos+=glm::vec3((float)eyeDirection.x*forward_speed,0,eyeDirection.z*forward_speed);
		camPos+=glm::vec3((float)sideDirection.x*side_speed,0,sideDirection.z*side_speed);
		camPos+=glm::vec3(0,height_speed,0);

		VkCommandBufferAllocateInfo bufferAllocInfo = {};
		bufferAllocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocInfo.commandPool = engineInstance->commandPool;
		bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocInfo.commandBufferCount = (uint32_t) engineInstance->commandBuffers.size();
		vkAllocateCommandBuffers(engineInstance->logicalDevice, &bufferAllocInfo, engineInstance->commandBuffers.data());
		engineInstance->uniformBufferObject.time+=timeElapsed*0.001;
		engineInstance->uniformBufferObject.viewMatrix = glm::lookAt(camPos,camPos+eyeDirection,glm::vec3(0.0f,1.0f,0.0f));
		engineInstance->uniformBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), (float)engineInstance->extent.width/(float)engineInstance->extent.height, 0.1f, 1000.0f);
		engineInstance->uniformBufferObject.projectionMatrix[1][1] *= -1;
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = engineInstance->renderPass;
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = engineInstance->extent;
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = {0.53f, 0.81f, 0.92f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		for(unsigned int i=0;i<engineInstance->imageCount;i++){
			renderPassInfo.framebuffer = engineInstance->Framebuffers[i];

			vkBeginCommandBuffer(engineInstance->commandBuffers[i], &beginInfo);
			vkCmdBeginRenderPass(engineInstance->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(engineInstance->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, engineInstance->graphicsPipeline);
			vkCmdBindDescriptorSets(engineInstance->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, engineInstance->pipelineLayout, 0, 1, &engineInstance->descSets[i], 0, nullptr);

			for (Entity &entity: engineInstance->Entities){
				engineInstance->uniformBufferObject.modelMatrix = glm::mat4(1.0f);
				engineInstance->uniformBufferObject.modelMatrix = glm::translate(engineInstance->uniformBufferObject.modelMatrix, entity.Position);
				engineInstance->uniformBufferObject.modelMatrix = glm::rotate(engineInstance->uniformBufferObject.modelMatrix,entity.Angle.y,glm::vec3(0.0f,1.0f,0.0f));
				engineInstance->uniformBufferObject.modelMatrix = glm::rotate(engineInstance->uniformBufferObject.modelMatrix,entity.Angle.x,glm::vec3(1.0f,0.0f,0.0f));
				engineInstance->uniformBufferObject.modelMatrix = glm::rotate(engineInstance->uniformBufferObject.modelMatrix,entity.Angle.z,glm::vec3(0.0f,0.0f,1.0f));


				void* data;
				vkMapMemory(engineInstance->logicalDevice, engineInstance->uniformBufferMemory, 0, sizeof(engineInstance->uniformBufferObject), 0, &data);
				memcpy(data, &engineInstance->uniformBufferObject, sizeof(engineInstance->uniformBufferObject));
				vkUnmapMemory(engineInstance->logicalDevice, engineInstance->uniformBufferMemory);

				VkBuffer vertexBuffers[] = {entity.vertexBuffer};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(engineInstance->commandBuffers[i], 0, 1, vertexBuffers, offsets);
				if(entity.hasIndexBuffer){
					vkCmdBindIndexBuffer(engineInstance->commandBuffers[i], entity.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(engineInstance->commandBuffers[i], static_cast<uint32_t>(entity.Indices.size()), 1, 0, 0, 0);
				}
				else{
					vkCmdDraw(engineInstance->commandBuffers[i], static_cast<uint32_t>(entity.Vertices.size()), 1, 0, 0);
				}
			}

			vkCmdEndRenderPass(engineInstance->commandBuffers[i]);
			vkEndCommandBuffer(engineInstance->commandBuffers[i]);
		}
		for(unsigned int i=0;i<engineInstance->imageCount;i++){
			vkWaitForFences(engineInstance->logicalDevice, 1, &engineInstance->Fence, VK_TRUE, UINT64_MAX);
			vkResetFences(engineInstance->logicalDevice, 1, &engineInstance->Fence);

			uint32_t imageIndex;
			vkAcquireNextImageKHR(engineInstance->logicalDevice, engineInstance->swapChain, UINT64_MAX, engineInstance->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = {engineInstance->imageAvailableSemaphore};
			VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &engineInstance->commandBuffers[imageIndex];
			VkSemaphore signalSemaphores[] = {engineInstance->renderFinishedSemaphore};
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			if(vkQueueSubmit(engineInstance->graphicsQueue, 1, &submitInfo, engineInstance->Fence)!=VK_SUCCESS){
				std::cerr << "Queue submit Failed" << std::endl;
			}


			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapchains[] = {engineInstance->swapChain};
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapchains;
			presentInfo.pImageIndices = &imageIndex;
			vkQueuePresentKHR(engineInstance->graphicsQueue, &presentInfo);
		}
		vkQueueWaitIdle(engineInstance->graphicsQueue);
		vkFreeCommandBuffers(engineInstance->logicalDevice,engineInstance->commandPool,engineInstance->imageCount,engineInstance->commandBuffers.data());
		vkResetCommandPool(engineInstance->logicalDevice,engineInstance->commandPool,VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);







		 end = std::chrono::system_clock::now();
	}


	delete engineInstance;

}
