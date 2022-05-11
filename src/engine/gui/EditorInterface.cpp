//
// Created by Diego S. Seabra on 10/05/22.
//

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
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

}

void CEditorInterface::Shutdown()
{
    delete mImplementation;
}

CEditorInterfaceImpl::CEditorInterfaceImpl()
{
    CLogger::Info("Initializing editor interface");

    CreateImGuiContext();
    BindImGuiToVulkan();

}

CEditorInterfaceImpl::~CEditorInterfaceImpl()
{
    CLogger::Info("Shutting down editor interface");

    // TODO: Enable imgui-vulkan binding shutdown
//    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CEditorInterfaceImpl::CreateImGuiContext()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    ImGui_ImplGlfw_InitForVulkan(CWindow::GetWindow(), true);
}

void CEditorInterfaceImpl::BindImGuiToVulkan()
{
    ImGui_ImplVulkan_InitInfo imguiInfo{};
    imguiInfo.Instance = CVulkanRenderer::GetInstance();
    imguiInfo.PhysicalDevice = CVulkanRenderer::GetPhysicalDevice();
    imguiInfo.Device = CVulkanRenderer::GetLogicalDevice();
    imguiInfo.Allocator = nullptr; // not using an allocator right now
    imguiInfo.Subpass = 0; // REVIEW: Do we need a subpass here?
    imguiInfo.MinImageCount = 2;
    imguiInfo.ImageCount = 3;

    // TODO: Implement initialization of ImGui inside vulkan
//    ImGui_ImplVulkan_Init(&imguiInfo, nullptr);
}