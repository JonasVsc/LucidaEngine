#include "lucida_config.h"

#include "debug/log.h"

// std
#include <fstream>

LucidaConfig::LucidaConfig(const std::string& path)
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

void LucidaConfig::default_config()
{
	m_config = json::parse( R"(
	  {
		"renderer": {
			"validation_layers": true
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

