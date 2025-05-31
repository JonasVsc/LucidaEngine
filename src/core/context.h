#pragma once

class Config;
class Window;
class Device;
class Swapchain;
class Renderer;

struct EngineContext {
	Config* config = nullptr;
	Window* window = nullptr;
	Device* device = nullptr;
	Swapchain* swapchain = nullptr;
};