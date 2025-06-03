cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(VULKAN_SDK "C:\\VulkanSDK\\1.4.313.0")

project ("Lucida")

# PACKAGES
add_subdirectory(3rdparty/fmt)
add_subdirectory(3rdparty/json)

include_directories(src ${VULKAN_SDK}/Include)

set(CORE "src/core")

# MODULES
set(CONFIG_SOURCES
	"${CORE}/config/config.cpp"
)

set(WINDOW_SOURCES
	"src/window/window.cpp"
)

set(GRAPHICS_SOURCES
	"src/graphics/renderer.cpp"
	"src/graphics/swapchain.cpp"
	"src/graphics/device.cpp"
)

set(CORE_SOURCES 
	${CONFIG_SOURCES} 
)

add_executable (Lucida 
	"src/lucida.cpp"
	${CORE_SOURCES}
	${WINDOW_SOURCES}
	${GRAPHICS_SOURCES}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Lucida PROPERTY CXX_STANDARD 20)
endif()

set(SDL2_LIB
	"${VULKAN_SDK}/Lib/SDL2main.lib"
	"${VULKAN_SDK}/Lib/SDL2.lib"
)

set(VULKAN_LIB
	"${VULKAN_SDK}/Lib/vulkan-1.lib"
)

set(MISC_LIB
	fmt::fmt nlohmann_json::nlohmann_json
)

target_link_libraries(Lucida 
	PRIVATE ${MISC_LIB} ${SDL2_LIB} ${VULKAN_LIB}
)


# DEPENDENCIES
file(COPY ${CMAKE_SOURCE_DIR}/lucida.json DESTINATION ${CMAKE_BINARY_DIR})