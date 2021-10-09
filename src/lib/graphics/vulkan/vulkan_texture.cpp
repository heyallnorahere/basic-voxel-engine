#include "bve_pch.h"
#include "vulkan_texture.h"
#include "util.h"
#include "vulkan_buffer.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            static std::map<uint32_t, vulkan_texture*> bound_textures;
            void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice device, VkPhysicalDevice physical_device, VkImage& image, VkDeviceMemory& memory) {
                VkImageCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                create_info.imageType = VK_IMAGE_TYPE_2D;
                create_info.extent.width = width;
                create_info.extent.height = height;
                create_info.extent.depth = 1;
                create_info.mipLevels = 1;
                create_info.arrayLayers = 1;
                create_info.format = format;
                create_info.tiling = tiling;
                create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                create_info.usage = usage;
                create_info.samples = VK_SAMPLE_COUNT_1_BIT;
                create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                if (vkCreateImage(device, &create_info, nullptr, &image) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan texture] could not create image");
                }
                VkMemoryRequirements requirements;
                vkGetImageMemoryRequirements(device, image, &requirements);
                VkMemoryAllocateInfo alloc_info;
                util::zero(alloc_info);
                alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                alloc_info.allocationSize = requirements.size;
                alloc_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, properties, physical_device);
                if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan texture] could not allocate image memory");
                }
                vkBindImageMemory(device, image, memory, 0);
            }
            static bool has_stencil_component(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
            void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, ref<vulkan_context> context) {
                VkCommandBuffer command_buffer = context->begin_single_time_commands();
                VkImageMemoryBarrier barrier;
                util::zero(barrier);
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.image = image;
                barrier.oldLayout = old_layout;
                barrier.newLayout = new_layout;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    if (has_stencil_component(format)) {
                        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                } else {
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                }
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                VkPipelineStageFlags source_stage, destination_stage;
                if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                } else {
                    throw std::runtime_error("[vulkan texture] unsupported layout transition");
                }
                vkCmdPipelineBarrier(command_buffer,
                    source_stage, destination_stage,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
                context->end_single_time_commands(command_buffer);
            }
            void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, ref<vulkan_context> context) {
                VkCommandBuffer command_buffer = context->begin_single_time_commands();
                VkBufferImageCopy region;
                util::zero(region);
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { width, height, 1 };
                vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                context->end_single_time_commands(command_buffer);
            }
            VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkDevice device) {
                VkImageViewCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                create_info.image = image;
                create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                create_info.format = format;
                create_info.subresourceRange.aspectMask = aspect_flags;
                create_info.subresourceRange.baseMipLevel = 0;
                create_info.subresourceRange.levelCount = 1;
                create_info.subresourceRange.baseArrayLayer = 0;
                create_info.subresourceRange.layerCount = 1;
                VkImageView image_view;
                if (vkCreateImageView(device, &create_info, nullptr, &image_view) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan texture] could not create image view");
                }
                return image_view;
            }
            std::map<uint32_t, ref<vulkan_texture>> vulkan_texture::get_bound_textures() {
                std::map<uint32_t, ref<vulkan_texture>> refs;
                for (auto [slot, texture] : bound_textures) {
                    refs.insert({ slot, texture });
                }
                return refs;
            }
            vulkan_texture::vulkan_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, ref<vulkan_object_factory> factory) {
                this->m_width = width;
                this->m_height = height;
                this->m_channels = channels;
                this->m_factory = factory;
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                this->m_device = context->get_device();
                this->create(data, context);
            }
            vulkan_texture::~vulkan_texture() {
                std::vector<uint32_t> slots;
                for (auto [slot, texture] : bound_textures) {
                    if (this == texture) {
                        slots.push_back(slot);
                    }
                }
                for (uint32_t slot : slots) {
                    bound_textures.erase(slot);
                }
                vkDestroySampler(this->m_device, this->m_sampler, nullptr);
                vkDestroyImageView(this->m_device, this->m_image_view, nullptr);
                vkDestroyImage(this->m_device, this->m_image, nullptr);
                vkFreeMemory(this->m_device, this->m_memory, nullptr);
            }
            void vulkan_texture::bind(uint32_t slot) {
                bound_textures[slot] = this;
                auto pipeline = this->m_factory->get_current_pipeline();
                if (pipeline) {
                    auto vk_pipeline = pipeline.as<vulkan_pipeline>();
                    auto shader = vk_pipeline->get_shader();
                    if (shader) {
                        auto context = this->m_factory->get_current_context().as<vulkan_context>();
                        size_t image_count = context->get_swapchain_image_count();
                        for (size_t i = 0; i < image_count; i++) {
                            shader->update_descriptor_sets(i);
                        }
                    }
                }
            }
            glm::ivec2 vulkan_texture::get_size() {
                return glm::ivec2(this->m_width, this->m_height);
            }
            int32_t vulkan_texture::get_channels() {
                return this->m_channels;
            }
            ImTextureID vulkan_texture::get_texture_id() {
                return nullptr; // todo: figure out some ImGui thing
            }
            void vulkan_texture::create(const std::vector<uint8_t>& data, ref<vulkan_context> context) {
                VkFormat format;
                this->create_texture_image(data, context, format);
                this->m_image_view = create_image_view(this->m_image, format, VK_IMAGE_ASPECT_COLOR_BIT, this->m_device);
                this->create_sampler(context);
                util::zero(this->m_image_info);
                this->m_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                this->m_image_info.imageView = this->m_image_view;
                this->m_image_info.sampler = this->m_sampler;
            }
            void vulkan_texture::create_texture_image(const std::vector<uint8_t>& data, ref<vulkan_context> context, VkFormat& format) {
                VkPhysicalDevice physical_device = context->get_physical_device();
                size_t buffer_size = (size_t)this->m_width * this->m_height * this->m_channels;
                VkBuffer staging_buffer;
                VkDeviceMemory staging_memory;
                create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    this->m_device, physical_device, staging_buffer, staging_memory);
                void* gpu_data;
                vkMapMemory(this->m_device, staging_memory, 0, buffer_size, 0, &gpu_data);
                memcpy(gpu_data, data.data(), buffer_size);
                vkUnmapMemory(this->m_device, staging_memory);
                switch (this->m_channels) {
                case 1:
                    format = VK_FORMAT_R8_SRGB;
                    break;
                case 2:
                    format = VK_FORMAT_R8G8_SRGB;
                    break;
                case 3:
                    format = VK_FORMAT_R8G8B8_SRGB;
                    break;
                case 4:
                    format = VK_FORMAT_R8G8B8A8_SRGB;
                    break;
                default:
                    throw std::runtime_error("[vulkan texture] invalid channel count");
                }
                uint32_t width = (uint32_t)this->m_width;
                uint32_t height = (uint32_t)this->m_height;
                create_image(width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_device, physical_device, this->m_image, this->m_memory);
                transition_image_layout(this->m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, context);
                copy_buffer_to_image(staging_buffer, this->m_image, width, height, context);
                transition_image_layout(this->m_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, context);
                vkDestroyBuffer(this->m_device, staging_buffer, nullptr);
                vkFreeMemory(this->m_device, staging_memory, nullptr);
            }
            void vulkan_texture::create_sampler(ref<vulkan_context> context) {
                VkPhysicalDevice physical_device = context->get_physical_device();
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(physical_device, &properties);
                VkPhysicalDeviceFeatures features;
                vkGetPhysicalDeviceFeatures(physical_device, &features);
                VkSamplerCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                create_info.magFilter = VK_FILTER_LINEAR;
                create_info.minFilter = VK_FILTER_LINEAR;
                create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                if (features.samplerAnisotropy) {
                    create_info.anisotropyEnable = true;
                    create_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                } else {
                    create_info.anisotropyEnable = false;
                    create_info.maxAnisotropy = 1.f;
                }
                create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                create_info.unnormalizedCoordinates = false;
                create_info.compareEnable = false;
                create_info.compareOp = VK_COMPARE_OP_ALWAYS;
                create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                create_info.mipLodBias = 0.f;
                create_info.minLod = 0.f;
                create_info.maxLod = 0.f;
                if (vkCreateSampler(this->m_device, &create_info, nullptr, &this->m_sampler) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan texture] could not create sampler");
                }
            }
        }
    }
}