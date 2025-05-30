#pragma once

// lib
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// std
#include <string>
#include <vector>

class LucidaConfig {
public:

	LucidaConfig(const std::string& path);

	// APP
	std::string get_app_name() { return m_config["app"]["name"]; }
	std::vector<int> get_app_version() { return m_config["app"]["version"].get<std::vector<int>>(); }

	// ENGINE
	std::vector<int> get_lucida_version() { return m_config["lucida"]["version"].get<std::vector<int>>(); }

	// RENDERER
	std::vector<std::string> get_layers() { return m_config["renderer"]["vulkan"]["layers"].get<std::vector<std::string>>(); }
	std::vector<std::string> get_extensions() { return m_config["renderer"]["vulkan"]["extensions"].get<std::vector<std::string>>(); }
	std::vector<int> get_api_version() { return m_config["renderer"]["vulkan"]["version"].get<std::vector<int>>(); }

	// WINDOW
	std::string get_window_title() { return m_config["window"]["title"]; }
	int get_window_width() { return m_config["window"]["width"]; }
	int get_window_height() { return m_config["window"]["height"]; }
	bool is_window_resizable() { return m_config["window"]["resizable"]; }
	bool is_window_fullscreen() { return m_config["window"]["fullscreen"]; }

private:

	void default_config();

	json m_config;
};