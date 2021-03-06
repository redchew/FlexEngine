#include "stdafx.hpp"
#if COMPILE_VULKAN

#include "Graphics/Vulkan/VulkanBuffer.hpp"

#include "Graphics/Vulkan/VulkanDevice.hpp"
#include "Graphics/Vulkan/VulkanInitializers.hpp"

namespace flex
{
	namespace vk
	{
		VulkanBuffer::VulkanBuffer(VulkanDevice* device) :
			m_Device(device),
			m_Buffer(VDeleter<VkBuffer>(device->m_LogicalDevice, vkDestroyBuffer)),
			m_Memory(VDeleter<VkDeviceMemory>(device->m_LogicalDevice, vkFreeMemory))
		{
		}

		VkResult VulkanBuffer::Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		{
			VkBufferCreateInfo bufferInfo = vks::bufferCreateInfo(usage, size);
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK_RESULT(vkCreateBuffer(m_Device->m_LogicalDevice, &bufferInfo, nullptr, m_Buffer.replace()));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_Device->m_LogicalDevice, m_Buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = vks::memoryAllocateInfo(memRequirements.size);
			allocInfo.memoryTypeIndex = FindMemoryType(m_Device, memRequirements.memoryTypeBits, properties);

			VK_CHECK_RESULT(vkAllocateMemory(m_Device->m_LogicalDevice, &allocInfo, nullptr, m_Memory.replace()));

			// Create the memory backing up the buffer handle
			m_Alignment = memRequirements.alignment;
			m_Size = allocInfo.allocationSize;
			m_UsageFlags = usage;
			m_MemoryPropertyFlags = properties;

			return Bind();
		}

		void VulkanBuffer::Destroy()
		{
			m_Buffer.replace();
			m_Memory.replace();
		}

		VkResult VulkanBuffer::Bind()
		{
			return vkBindBufferMemory(m_Device->m_LogicalDevice, m_Buffer, m_Memory, 0);
		}

		VkResult VulkanBuffer::Map(VkDeviceSize size)
		{
			return vkMapMemory(m_Device->m_LogicalDevice, m_Memory, 0, size, 0, &m_Mapped);
		}

		void VulkanBuffer::Unmap()
		{
			if (m_Mapped)
			{
				vkUnmapMemory(m_Device->m_LogicalDevice, m_Memory);
				m_Mapped = nullptr;
			}
		}
	} // namespace vk
} // namespace flex

#endif // COMPILE_VULKAN