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
include_directories(src ${VULKAN_SDK}/Include include)

# MODULES
set(WINDOW
	"src/window.cpp"
)

set(RENDERER
	"src/renderer.cpp"
)

add_executable (Lucida
	"src/main.cpp"
	${RENDERER}
	${WINDOW}
)

set(SDL2_LIB
	"${VULKAN_SDK}/Lib/SDL2main.lib"
	"${VULKAN_SDK}/Lib/SDL2.lib"
)

set(VULKAN_LIB
	"${VULKAN_SDK}/Lib/vulkan-1.lib"
)

target_link_libraries(Lucida 
	PRIVATE ${SDL2_LIB} ${VULKAN_LIB}
)