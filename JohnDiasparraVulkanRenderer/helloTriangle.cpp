#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// For reporting and propagating errors.
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

class HelloTriangleApplication {

	public:

		const uint32_t winResX = 800;
		const uint32_t winResY = 600;
		GLFWwindow* window;
		
		// Application Life-Cycle
		void run() {
			initWindow();
			initVulkan();
			mainLoop();
			cleanup();
		}

	private:

		VkInstance instance; // Vulkan Instance is the connection between an application and the vulkan library.

		void initWindow() {
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tells GLFW not to create in the OpenGL context.
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disables resizable window.

			window = glfwCreateWindow(winResX, winResY, "Vulkan Render Window", nullptr, nullptr);
		}

		void initVulkan() {
			createInstance();
		}

		void createInstance() {
			VkApplicationInfo appInfo{}; // Struct that contains various app information. 

			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Triangle!";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Gabagool Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			/*
				GLFW Instance Creation
			*/

			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;

			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			createInfo.enabledExtensionCount = glfwExtensionCount;
			createInfo.ppEnabledExtensionNames = glfwExtensions;
			createInfo.enabledLayerCount = 0;

			/*
				Common patterin in vk object creation :
				- Pointer to struct with create info. 
				- Custom allocator callbacks. 
				- Pointer to variable that stores the handle of new objects.
			*/

			if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create vkInstance!\n");
			}

			std::cout << "VkInstance Created Succesfully.\n";
		}

		void mainLoop() {
			// Loops and checks for the window being closed. Main app life-cycle will then run cleanup once the loop is terminated.
			while (!glfwWindowShouldClose(window)) {
				glfwPollEvents();
			}

		}

		void cleanup() {
			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(window);
			glfwTerminate();
		}

};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}