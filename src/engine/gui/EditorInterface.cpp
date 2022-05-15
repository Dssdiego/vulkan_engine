//
// Created by Diego S. Seabra on 10/05/22.
//

#include "EditorInterface.h"
#include "../profiling/Logger.h"
#include "../rendering/Window.h"
#include "../rendering/VulkanRenderer.h"

// TODO: Refactor the code so that we don't use raw pointers. Instead we want to use smart pointers
//       See more here: https://stackoverflow.com/questions/106508/what-is-a-smart-pointer-and-when-should-i-use-one
CEditorInterfaceImpl* mImplementation = nullptr;

void CEditorInterface::Init()
{
    mImplementation = new CEditorInterfaceImpl;
}

void CEditorInterface::Draw()
{
    // TODO: Implement
//    ImGui_ImplVulkan_NewFrame();
//    ImGui_ImplGlfw_NewFrame();
//    ImGui::NewFrame();

//    ImGui::ShowDemoWindow();
}

void CEditorInterface::Shutdown()
{
    delete mImplementation;
}

CEditorInterfaceImpl::CEditorInterfaceImpl()
{
    CLogger::Info("Initializing editor interface");

    InitializeImGui();
}

CEditorInterfaceImpl::~CEditorInterfaceImpl()
{
    CLogger::Info("Shutting down editor interface");

    // TODO: Enable imgui-vulkan binding shutdown
//    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CEditorInterfaceImpl::InitializeImGui()
{
    // Check Version and create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    ImGui_ImplGlfw_InitForVulkan(CWindow::GetWindow(), true);

    // Setting ImGui to use dark colors
    ImGui::StyleColorsDark();

    // Setup Vulkan init info
    ImGui_ImplVulkan_InitInfo imguiInfo{};
    imguiInfo.Instance = CVulkanRenderer::GetInstance();
    imguiInfo.PhysicalDevice = CVulkanRenderer::GetPhysicalDevice();
    imguiInfo.Device = CVulkanRenderer::GetLogicalDevice();
//    imguiInfo.QueueFamily = 0; // REVIEW: Do we need a queuefamily to be passed here?
    imguiInfo.Queue = CVulkanRenderer::GetGraphicsQueue();
    imguiInfo.DescriptorPool = CVulkanRenderer::GetDescriptorPool();
    imguiInfo.MinImageCount = 3; // FIXME: This should come from our own vulkan renderer
    imguiInfo.ImageCount = 3; // FIXME: This should come from our own vulkan renderer
    imguiInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    imguiInfo.Allocator = nullptr; // not using an allocator right now

    // Init ImGui for Vulkan
    // TODO: Implement
//    ImGui_ImplVulkan_Init(&imguiInfo, CVulkanRenderer::GetRenderPass());

    // TODO: Allocate a command buffer
    // TODO: Record the command buffer
    // TODO: Submit to the GPU
}
