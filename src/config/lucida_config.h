#pragma once

// lib
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// std
#include <string>

class LucidaConfig {
public:

	LucidaConfig(const std::string& path);

	int get_window_width() { return m_config["window"]["width"]; }
	int get_window_height() { return m_config["window"]["height"]; }
	std::string get_window_title() { return m_config["window"]["title"]; }
	bool is_window_resizable() { return m_config["window"]["resizable"]; }
	bool is_window_fullscreen() { return m_config["window"]["fullscreen"]; }

private:

	void default_config();

	json m_config;
};