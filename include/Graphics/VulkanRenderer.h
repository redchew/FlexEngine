#pragma once
#if COMPILE_VULKAN

#include "Renderer.h"

#include <functional>
#include <vector>

struct GameContext;
class Window;

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanVertex
{
	static VkVertexInputBindingDescription GetVertPosColTexBindingDescription();
	static VkVertexInputBindingDescription GetVertPosColBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> GetVertPosColTexAttributeDescriptions();
	static std::array<VkVertexInputAttributeDescription, 2> GetVertPosColAttributeDescriptions();
	//bool operator==(const VulkanVertex& other) const;
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

template <typename T>
class VDeleter
{
public:
	VDeleter();

	VDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef);
	VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef);
	VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef);
	~VDeleter();

	const T* operator &() const;
	T* replace();
	operator T() const;
	void operator=(T rhs);

	template<typename V>
	bool operator==(V rhs);

private:
	T object{ VK_NULL_HANDLE };
	std::function<void(T)> deleter;

	void cleanup();
};


class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer(GameContext& gameContext);
	virtual ~VulkanRenderer();
	
	virtual void PostInitialize() override;

	virtual glm::uint Initialize(const GameContext& gameContext, std::vector<VertexPosCol>* vertices) override;
	virtual glm::uint Initialize(const GameContext& gameContext, std::vector<VertexPosCol>* vertices,
		std::vector<glm::uint>* indices) override;
	
	virtual void SetClearColor(float r, float g, float b) override;

	virtual void Draw(const GameContext& gameContext, glm::uint renderID) override;

	virtual void SetVSyncEnabled(bool enableVSync) override;
	virtual void Clear(int flags, const GameContext& gameContext) override;
	virtual void SwapBuffers(const GameContext& gameContext) override;

	virtual void UpdateTransformMatrix(const GameContext& gameContext, glm::uint renderID, const glm::mat4x4& model) override;

	virtual int GetShaderUniformLocation(glm::uint program, const std::string uniformName) override;
	virtual void SetUniform1f(glm::uint location, float val) override;

	virtual void DescribeShaderVariable(glm::uint renderID, glm::uint program, const std::string& variableName, int size,
		Renderer::Type renderType, bool normalized, int stride, void* pointer) override;

	virtual void Destroy(glm::uint renderID) override;

private:
	//static GLuint BufferTargetToGLTarget(BufferTarget bufferTarget);
	//static GLenum TypeToGLType(Type type);
	//static GLenum UsageFlagToGLUsageFlag(UsageFlag usage);
	//static GLenum ModeToGLMode(Mode mode);

	void CreateInstance();
	void SetupDebugCallback();
	void CreateSurface(Window* window);
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain(Window* window);
	void CreateImageViews();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateCommandPool();
	void CreateDepthResources();
	void CreateFramebuffers();
	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();
	void LoadModel(const std::string& filePath);

	void CreateVertexBuffer(glm::uint renderID);
	void CreateIndexBuffer(glm::uint renderID);
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateDescriptorSet();
	void CreateCommandBuffers();
	void CreateSemaphores();

	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VDeleter<VkImageView>& imageView);
	void RecreateSwapChain(Window* window);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VDeleter<VkImage>& image, VDeleter<VkDeviceMemory>& imageMemory);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void DrawFrame(Window* window);
	void CreateShaderModule(const std::vector<char>& code, VDeleter<VkShaderModule>& shaderModule);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D ChooseSwapExtent(Window* window, const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	void UpdateUniformBuffer(const GameContext& gameContext, const UniformBufferObject& ubo);

	static std::vector<char> ReadFile(const std::string& filename);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, 
		VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, 
		const char* msg, void* userData);

	struct RenderObject
	{
		RenderObject(const VDeleter<VkDevice>& device)
		{
			vertexBuffer = VDeleter<VkBuffer>(device, vkDestroyBuffer);
			vertexBufferMemory = VDeleter<VkBuffer>(device, vkFreeMemory);
			indexBuffer = VDeleter<VkBuffer>(device, vkDestroyBuffer);
			indexBufferMemory = VDeleter<VkBuffer>(device, vkFreeMemory);
		}

		glm::uint renderID;

		glm::uint VAO;
		glm::uint VBO;
		glm::uint IBO;

		VDeleter<VkBuffer> vertexBuffer; //{ device, vkDestroyBuffer }
		VDeleter<VkDeviceMemory> vertexBufferMemory; // { device, vkFreeMemory };
		VDeleter<VkBuffer> indexBuffer; // { device, vkDestroyBuffer };
		VDeleter<VkDeviceMemory> indexBufferMemory; // { device, vkFreeMemory };
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VertexPosCol>* vertices = nullptr;

		bool indexed = false;
		std::vector<glm::uint>* indices = nullptr;

		glm::uint MVP;
	};

	RenderObject* GetRenderObject(int renderID);

	// TODO: use sorted data type (map)
	std::vector<RenderObject*> m_RenderObjects;

	bool m_VSyncEnabled;



	const std::string MODEL_PATH = "resources/models/chalet.obj";
	const std::string MODEL_TEXTURE_PATH = "resources/textures/chalet.jpg";

	const std::vector<const char*> m_ValidationLayers =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	const std::vector<const char*> m_DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool m_EnableValidationLayers = false;
#else
	const bool m_EnableValidationLayers = true;
#endif

	VDeleter<VkInstance> m_Instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> m_Callback{ m_Instance, DestroyDebugReportCallbackEXT };
	VDeleter<VkSurfaceKHR> m_Surface{ m_Instance, vkDestroySurfaceKHR };

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VDeleter<VkDevice> m_Device{ vkDestroyDevice };

	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	VDeleter<VkSwapchainKHR> m_SwapChain{ m_Device, vkDestroySwapchainKHR };
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VDeleter<VkImageView>> m_SwapChainImageViews;
	std::vector<VDeleter<VkFramebuffer>> m_SwapChainFramebuffers;

	VDeleter<VkRenderPass> m_RenderPass{ m_Device, vkDestroyRenderPass };
	VDeleter<VkDescriptorSetLayout> m_DescriptorSetLayout{ m_Device, vkDestroyDescriptorSetLayout };
	VDeleter<VkPipelineLayout> m_PipelineLayout{ m_Device, vkDestroyPipelineLayout };
	VDeleter<VkPipeline> m_GraphicsPipeline{ m_Device, vkDestroyPipeline };

	VDeleter<VkCommandPool> m_CommandPool{ m_Device, vkDestroyCommandPool };

	VDeleter<VkImage> m_TextureImage{ m_Device, vkDestroyImage };
	VDeleter<VkDeviceMemory> m_TextureImageMemory{ m_Device, vkFreeMemory };
	VDeleter<VkImageView> m_TextureImageView{ m_Device, vkDestroyImageView };
	VDeleter<VkSampler> m_TextureSampler{ m_Device, vkDestroySampler };

	VDeleter<VkImage> m_DepthImage{ m_Device, vkDestroyImage };
	VDeleter<VkDeviceMemory> m_DepthImageMemory{ m_Device, vkFreeMemory };
	VDeleter<VkImageView> m_DepthImageView{ m_Device, vkDestroyImageView };

	//std::vector<VertexPosCol> m_Vertices;
	//std::vector<uint32_t> m_Indices;

	VDeleter<VkBuffer> m_UniformStagingBuffer{ m_Device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_UniformStagingBufferMemory{ m_Device, vkFreeMemory };
	VDeleter<VkBuffer> m_UniformBuffer{ m_Device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_UniformBufferMemory{ m_Device, vkFreeMemory };

	VDeleter<VkDescriptorPool> m_DescriptorPool{ m_Device, vkDestroyDescriptorPool };
	VkDescriptorSet m_DescriptorSet;

	VDeleter<VkSemaphore> m_ImageAvailableSemaphore{ m_Device, vkDestroySemaphore };
	VDeleter<VkSemaphore> m_RenderFinishedSemaphore{ m_Device, vkDestroySemaphore };

	VkClearColorValue m_ClearColor;

	VulkanRenderer(const VulkanRenderer&) = delete;
	VulkanRenderer& operator=(const VulkanRenderer&) = delete;
};

#endif // COMPILE_VULKAN