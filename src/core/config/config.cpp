#include "config.h"

// core
#include "core/log.h"

// std
#include <fstream>

Config::Config(const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		jerr("failed to load user config");
		jwarn("using default config");
		default_config();
		return;
	}

	m_config = json::parse(file);
	file.close();
}

void Config::default_config()
{
	m_config = json::parse( R"(
	  {
		"lucida": {
			"version": [0,0,1]
		},

		"app": {
			"name": "Lucida Application",
			"version": [0,0,1]
		},

		"renderer": {
			"vulkan": {
				"version": [1,0,0],
				"layers": [],
				"extensions": []
			}
		},

		"window": {
			"title": "Lucida Application",
			"width": 640,
			"height": 480,
			"fullscreen": false,
			"resizable": true
		}
	  }
	)");
}

