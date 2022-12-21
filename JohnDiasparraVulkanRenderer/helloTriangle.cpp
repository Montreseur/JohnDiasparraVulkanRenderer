#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// For reporting and propagating errors.
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

const uint32_t winResX = 800;
const uint32_t winResY = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


#ifdef NDEBUG // Standard cpp library means "not debug".
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


class HelloTriangleApplication {

	public:
		// Application Life-Cycle
		void run() {
			initWindow();
			initVulkan();
			mainLoop();
			cleanup();
		}

	private:
		GLFWwindow* window;

		VkInstance instance; // Vulkan Instance is the connection between an application and the vulkan library.
		VkDebugUtilsMessengerEXT debugMessenger;

		void initWindow() {
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tells GLFW not to create in the OpenGL context.
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disables resizable window.

			window = glfwCreateWindow(winResX, winResY, "Vulkan Render Window", nullptr, nullptr);
		}

		void initVulkan() {
			createInstance();
			setupDebugMessenger();
		}

		void createInstance() {

			if (enableValidationLayers && !checkValidationLayerSupport()) {
				throw std::runtime_error("Validation layers requested, but not available.");
			}

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

			auto extensions = getRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();
			
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();

				populateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else {
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}

			//validateVulkanExtensions();

			/*
				GLFW Instance Creation
			*/

			//validateGLFWExtentions(glfwExtensionCount, glfwExtensions);

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
			if (enableValidationLayers) {
				DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			}

			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(window);
			glfwTerminate();
		}

		void validateVulkanExtensions() {
			uint32_t extensionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> extensions(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
			std::cout << "Vulkan Extensions Available:\n";

			for (const auto& extension : extensions) {
				std::cout << "\t" << extension.extensionName << "\n";
			}
		}

		void validateGLFWExtentions(uint32_t extCount, const char** ext  ) {
			std::cout << "GLFW Extensions Available: \n";

			for (auto i = 0; i < extCount; ++i) {
				std::cout << "\t" << ext[i] << "\n";
			}
		}

		bool checkValidationLayerSupport() {
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
			
			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}
				if (!layerFound) {
					return false;
				}
			}
		}

		std::vector<const char*> getRequiredExtensions() {
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (enableValidationLayers) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			return extensions;
		}

		/*
			Debug Callback Function
			-Uses PFN_vkDebugUtilsMessengerCallbackEXT prototype.
			-VKAPI_ATTR and VKAPI_CALL ensure that function has the right signature for Vulkan to call it.
		*/

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageSeverityFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, // Struct containing the details of the message itself (pMessage, pObjects, objectCount).
			void* pUserData) { // Parameter is a * specified to pass data to.

			std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";

			// Compare message severity to determine severity.
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
				std::cout << "Error/Bug Prone indication detected in Vulkan Layer Callback.";
			}
			return VK_FALSE;
		}

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			// Specify types of severities you want to be notified about.
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			// Filters which types of messages your callback is notified about.
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			// Specifies the pointer to the callback function.
			createInfo.pfnUserCallback = debugCallback;
			// Optional for passing data from the callback to the application.
			createInfo.pUserData = nullptr;
		}

		void setupDebugMessenger() {
			if (!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			populateDebugMessengerCreateInfo(createInfo);

			if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
				throw std::runtime_error("Failed to setup Debug Messenger.");
			}
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