//
// Created by Diego S. Seabra on 08/03/22.
//

#ifndef VULKAN_ENGINE_STRUCTS_H
#define VULKAN_ENGINE_STRUCTS_H

#include <string>

struct WindowSize
{
    uint32_t width;
    uint32_t height;
};

struct EngineConfig
{
    std::string gameTitle;
    WindowSize windowSize;
    WindowSize minWindowSize;
    bool saveLogToFile;
};

#endif //VULKAN_ENGINE_STRUCTS_H
