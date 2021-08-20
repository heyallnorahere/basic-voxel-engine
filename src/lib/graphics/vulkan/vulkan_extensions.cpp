#include "bve_pch.h"
#include "vulkan_extensions.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            VkResult _vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger) {
                auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
                if (func) {
                    return func(instance, create_info, allocator, debug_messenger);
                } else {
                    return VK_ERROR_EXTENSION_NOT_PRESENT;
                }
            }
            void _vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator) {
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
                if (func) {
                    func(instance, debug_messenger, allocator);
                }
            }
        }
    }
}