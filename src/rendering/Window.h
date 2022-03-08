//
// Created by Diego S. Seabra on 08/03/22.
//

#ifndef VULKAN_ENGINE_WINDOW_H
#define VULKAN_ENGINE_WINDOW_H

#include <GLFW/glfw3.h>
#include "../common/structs.h"

struct SWindowImpl
{
    SWindowImpl(SEngineConfig* pConfig);
    ~SWindowImpl();

    GLFWwindow* window;
};

class CWindow
{
public:
    static void Init(SEngineConfig* pConfig);
    static void Update();
    static void Shutdown();

    static bool ShouldCloseWindow();

private:
    void LoadCusor();
    void Resize();
};

#endif //VULKAN_ENGINE_WINDOW_H
