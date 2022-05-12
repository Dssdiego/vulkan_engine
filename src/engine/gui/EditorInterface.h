//
// Created by Diego S. Seabra on 10/05/22.
//

#ifndef VULKAN_ENGINE_EDITORINTERFACE_H
#define VULKAN_ENGINE_EDITORINTERFACE_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

struct CEditorInterfaceImpl
{
    // Constructor/Destructor
    CEditorInterfaceImpl();
    ~CEditorInterfaceImpl();

    // Variables
    VkDescriptorPool imguiPool;

    // Auxiliary Methods
    void CreateDescriptorPool();
    void InitializeImGui();
};

class CEditorInterface
{
public:
    static void Init();
    static void Draw();
    static void Shutdown();
};


#endif //VULKAN_ENGINE_EDITORINTERFACE_H
