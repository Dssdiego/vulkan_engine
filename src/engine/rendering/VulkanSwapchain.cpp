//
// Created by Diego S. Seabra on 09/07/22.
//

#include "VulkanSwapchain.h"
#include "VulkanDevice.h"

// TODO: Refactor the code so that we don't use raw pointers. Instead we want to use smart pointers
//       See more here: https://stackoverflow.com/questions/106508/what-is-a-smart-pointer-and-when-should-i-use-one
VulkanSwapChainImpl* mVulkanSwapChainImpl = nullptr;

//
// Initialization/Destruction
//

void VulkanSwapchain::Init()
{
    mVulkanSwapChainImpl = new VulkanSwapChainImpl;
}

void VulkanSwapchain::Shutdown()
{
    delete mVulkanSwapChainImpl;
}

//
// Implementation
//

VulkanSwapChainImpl::VulkanSwapChainImpl()
{
    CreateSwapChain();
    CreateImageViews();
//    CreateRenderPass(); // TODO
//    CreateDepthResources(); // TODO
//    CreateFramebuffers(); // TODO
//    CreateSyncObjects(); // TODO
}

VulkanSwapChainImpl::~VulkanSwapChainImpl()
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(VulkanDevice::GetDevice(), imageView, nullptr);
    }
    swapChainImages.clear();

    if (swapChain != nullptr)
    {
        vkDestroySwapchainKHR(VulkanDevice::GetDevice(), swapChain, nullptr);
        swapChain = nullptr;
    }

    // TODO: Destroy depth images (image view, image and free memory)

    // TODO: Destroy swapchain frame buffers
//    for (auto frameBuffer : swapChainFrameBuffers)
//    {
//
//    }

    // TODO: Destroy render pass
//    vkDestroyRenderPass(VulkanDevice::GetDevice(), renderPass, nullptr);

    // TODO: Cleanup synchonization objects (semaphores and fences)
}

void VulkanSwapChainImpl::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = VulkanDevice::GetSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; // one more image to have "room" for more processing
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        // making sure we don't exceed the maximum number of images in the swap chain
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    std::cout << "Minimum image count in the swap chain: " << imageCount << std::endl;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = VulkanDevice::GetSurface(); // tying our surface to the swap chain
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // the amount of layers each image consists of (always 1 unless we are developing a stereoscopic 3D app)

    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT - This is a color image we're rendering into
    // VK_IMAGE_USAGE_TRANSFER_SRC_BIT - We'll be copying the image somewhere (screenshot, postProcess)
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    uint32_t queueFamilyIndices[] = { VulkanDevice::GetGraphicsQueueFamilyIdx(), VulkanDevice::GetPresentQueueFamilyIdx() };

    // specifying how to handle swap chain images that will be used across multiple queue families (graphics and presentation)
    // in our case, we'll be drawing on the images in the swap chain from the graphics queue and then
    //    submitting them on the presentation queue
    // REVIEW: Is this correct?
    if (VulkanDevice::GetGraphicsQueueFamilyIdx() != VulkanDevice::GetPresentQueueFamilyIdx())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // optional
        createInfo.pQueueFamilyIndices = nullptr; // optional
    }

    // we don't want any pre-transformation on the images for now
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    // compositeAlpha = blending with other windows in the window system. So we will ignore the alpha channel (aka opaque)
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // we don't care for pixels that are obscured (ex.: another window in front of our own)
    createInfo.oldSwapchain = VK_NULL_HANDLE; // for now we assume that we'll be creating only one swap chain.
    // NOTE: Only one swap chain is not the best option because we'd like the screen to be resized, so in the future we'll probably recreate the swap chain ;)

    // effectively creating the swap chain
    VK_CHECK(vkCreateSwapchainKHR(VulkanDevice::GetDevice(), &createInfo, nullptr, &swapChain));

    // retrieve swap chain images from the logical device
    vkGetSwapchainImagesKHR(VulkanDevice::GetDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(VulkanDevice::GetDevice(), swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    Logger::Debug("Swap chain created");
}

void VulkanSwapChainImpl::CreateImageViews()
{
    // resizing the list to fit all of the image views
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    std::cout << "# of image views created: " << swapChainImageViews.size() << std::endl;
}

//
// Helpers
//

VkImageView VulkanSwapChainImpl::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.format = format;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(VulkanDevice::GetDevice(), &viewInfo, nullptr, &imageView));

    return imageView;
}

VkSurfaceFormatKHR VulkanSwapChainImpl::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    VkSurfaceFormatKHR result;

    // if vulkan returned an unknown format, then we just force what we want.
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        result.format = VK_FORMAT_B8G8R8A8_UNORM;
        result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return result;
    }

    // favor 32 bit rgba and srgb nonlinear colorspace
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    // if all else fails, just return what's available
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChainImpl::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    // checking if triple buffering is available
    for (const auto& availablePresentMode: availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            Logger::Debug("Present Mode: MAILBOX");
            return availablePresentMode;
        }
    }

    // if triple buffering is not available, use Vulkan's default swap chain queue (aka VSync)
    Logger::Debug("Present Mode: V-SYNC");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChainImpl::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(Window::GetWindow(), &width, &height);

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actualExtent.width =
                std::max(capabilities.minImageExtent.width,
                         std::min(capabilities.maxImageExtent.width,
                                  actualExtent.width));

        actualExtent.height =
                std::max(capabilities.minImageExtent.height,
                         std::min(capabilities.maxImageExtent.height,
                                  actualExtent.height));

        return actualExtent;
    }
}
