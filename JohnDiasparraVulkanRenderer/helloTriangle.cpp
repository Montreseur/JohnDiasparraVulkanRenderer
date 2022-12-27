#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// For reporting and propagating errors.
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vector>
#include <map>
#include <optional>

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
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Implicitly destroyed in cleanup. No manual cleanup needed.
		VkDevice device;
		VkQueue graphicsQueue;

		struct QueueFamilyIndicies {
			std::optional<uint32_t> graphicsFamily;

			bool isComplete() {
				return graphicsFamily.has_value();
			}
		};

		void initWindow() {
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tells GLFW not to create in the OpenGL context.
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disables resizable window.

			window = glfwCreateWindow(winResX, winResY, "Vulkan Render Window", nullptr, nullptr);
		}

		void initVulkan() {
			createInstance();
			setupDebugMessenger();
			pickPhysicalDevice();
			createLogicalDevice();
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
				vkDestroyDevice(device, nullptr);
			}

			vkDestroyInstance(instance, nullptr);
			glfwDestroyWindow(window);
			glfwTerminate();
		}
		void createInstance() {

			if (enableValidationLayers && !checkValidationLayerSupport()) {
				throw std::runtime_error("Validation layers requested, but not available.");
			}

			VkApplicationInfo appInfo{}; // Struct that contains various app information. 

			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Triangle!";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "MtVr Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_3;

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

			validateVulkanExtensions();

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

		void setupDebugMessenger() {
			if (!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			populateDebugMessengerCreateInfo(createInfo);

			if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
				throw std::runtime_error("Failed to setup Debug Messenger.");
			}
		}

		void pickPhysicalDevice() {
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0) {
				throw std::runtime_error("Failed to find a physical device with Vulkan Support.");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (const auto& device : devices) {
				if (isPhysicalDeviceValid(device)) {
					physicalDevice = device;
					debugPhysicalDevice(physicalDevice);
					break;
				}
			}
			if (physicalDevice == VK_NULL_HANDLE) {
				throw std::runtime_error("Failed to find a suitable GPU.");
			}
		}

		bool isPhysicalDeviceValid(VkPhysicalDevice device) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			std::cout << "Device Found: " << deviceProperties.deviceName << "\n";
			isDeviceSuitable(device);
			return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
		}

		void debugPhysicalDevice(VkPhysicalDevice device) {
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			std::cout << "Physial Device Debug: " << "\n";
			std::cout << "\t" << "Allocated Physical Device: " << deviceProperties.deviceName << "\n";

		}

		bool isDeviceSuitable(VkPhysicalDevice device) {
			QueueFamilyIndicies indices = findQueueFamilies(device);
			return indices.isComplete();
		}

		QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device) {
			QueueFamilyIndicies indicies;
			uint32_t queueFamilyCount = 0;

			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
			std::cout << "Device Queue Family Indicies: " << queueFamilies.size() << "\n";
			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indicies.graphicsFamily = i;
				}
				if (indicies.isComplete()) {
					
					break;
				}
				i++;
			}
			return indicies;
		}

		void createLogicalDevice() {
			float queuePriority = 1.0f;
			QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = indicies.graphicsFamily.value();
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			VkPhysicalDeviceFeatures deviceFeatures{};
			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = &queueCreateInfo;
			createInfo.queueCreateInfoCount = 1;
			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = 0;
			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else {
				createInfo.enabledLayerCount = 0;
			}
		
			if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create logical device.");
			}
			vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphicsQueue);
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