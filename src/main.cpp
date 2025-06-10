#include "window.h"
#include "renderer.h"

// GLOBALS


int main(int argc, char **argv) 
{
	LeWindow window;
	LeRenderer renderer;

	LeWindowCreateInfo windowCI{ "Lucida Game", 640, 480 };
	leWindowCreate(windowCI, window);
	leRendererCreate(window, renderer);

	uint32_t frameInFlight = 0;
	while (!window.shouldClose)
	{
		leWindowPoolEvents(window);

		// 1. Wait for previous frame to finish
		vkWaitForFences(renderer.device, 1, &renderer.inFlightFences[frameInFlight], VK_TRUE, UINT64_MAX);
		vkResetFences(renderer.device, 1, &renderer.inFlightFences[frameInFlight]);

		// 2. Acquire an image from the swapchain
		uint32_t imageIndex;
		vkAcquireNextImageKHR(renderer.device, renderer.swapchain, UINT64_MAX, renderer.imageAvailableSemaphores[frameInFlight], VK_NULL_HANDLE, &imageIndex);

		// 3. Record a command buffer which draws the scene onto that image
		vkResetCommandBuffer(renderer.commandBuffers[frameInFlight], 0);

		VkCommandBufferBeginInfo cmdBeginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};

		vkBeginCommandBuffer(renderer.commandBuffers[frameInFlight], &cmdBeginInfo);

		VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 1.0f}} };

		VkRenderPassBeginInfo renderBeginInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderer.renderPass,
			.framebuffer = renderer.framebuffers[imageIndex],
			.renderArea = {
				.offset = {0, 0},
				.extent = renderer.surfaceExtent
			},
			.clearValueCount = 1,
			.pClearValues = &clearColor
		};

		vkCmdBeginRenderPass(renderer.commandBuffers[frameInFlight], &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdEndRenderPass(renderer.commandBuffers[frameInFlight]);

		vkEndCommandBuffer(renderer.commandBuffers[frameInFlight]);

		// 4. Submit the recorded command buffer
		VkSemaphore waitSemaphores[] = { renderer.imageAvailableSemaphores[frameInFlight] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { renderer.renderFinishedSemaphores[frameInFlight] };

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &renderer.commandBuffers[frameInFlight],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signalSemaphores
		};

		vkQueueSubmit(renderer.graphicsQueue, 1, &submitInfo, renderer.inFlightFences[frameInFlight]);

		// 5. Present the swapchain image
		VkSwapchainKHR swapchains[] = { renderer.swapchain };
		
		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signalSemaphores,
			.swapchainCount = 1,
			.pSwapchains = swapchains,
			.pImageIndices = &imageIndex
		};

		vkQueuePresentKHR(renderer.presentQueue, &presentInfo);

		frameInFlight = (frameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	leRendererDestroy(renderer);
	leWindowDestroy(window);
	return 0; 
}