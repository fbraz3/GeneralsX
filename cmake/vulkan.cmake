# Vulkan SDK Configuration for Phase 09 Graphics Backend
# Optional - will skip if SDK not found

# Helper function to find Vulkan SDK
function(find_vulkan_sdk)
    set(_VULKAN_SDK_PATH "")
    
    if(WIN32)
        # Windows 32-bit VC6
        if(DEFINED ENV{VULKAN_SDK})
            set(_VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
        # Try common installation paths
        elseif(EXISTS "C:/VulkanSDK")
            file(GLOB VULKAN_VERSIONS "C:/VulkanSDK/*")
            if(VULKAN_VERSIONS)
                list(GET VULKAN_VERSIONS -1 _VULKAN_SDK_PATH)
            endif()
        endif()
        
    elseif(APPLE)
        # macOS
        if(DEFINED ENV{VULKAN_SDK})
            set(_VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
        # Try Homebrew location
        elseif(EXISTS "/usr/local/Cellar/vulkan-sdk")
            set(_VULKAN_SDK_PATH "/usr/local/Cellar/vulkan-sdk")
        endif()
        
    else()
        # Linux
        if(DEFINED ENV{VULKAN_SDK})
            set(_VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
        endif()
    endif()
    
    set(VULKAN_SDK_PATH "${_VULKAN_SDK_PATH}" PARENT_SCOPE)
endfunction()

# Find Vulkan SDK
find_vulkan_sdk()

if(NOT VULKAN_SDK_PATH)
    message(WARNING "Vulkan SDK not found - Phase 09 graphics backend will not be compiled")
    message(WARNING "To install: https://vulkan.lunarg.com/sdk/home")
    message(WARNING "Or run: ${CMAKE_SOURCE_DIR}/scripts/setup_vulkan_sdk.bat")
    set(VULKAN_FOUND FALSE)
    return()
endif()

message(STATUS "Vulkan SDK found at: ${VULKAN_SDK_PATH}")

# Determine include/lib directories based on platform
if(WIN32)
    set(VULKAN_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include")
    set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/Lib")
elseif(APPLE)
    set(VULKAN_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
    set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/lib")
else()
    set(VULKAN_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
    set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/lib")
endif()

# Verify Vulkan headers exist
if(NOT EXISTS "${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h")
    message(WARNING "Vulkan headers not found at ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h")
    set(VULKAN_FOUND FALSE)
    return()
endif()

# Create Vulkan interface library target
add_library(vulkan_interface INTERFACE)

target_include_directories(vulkan_interface INTERFACE
    "${VULKAN_INCLUDE_DIR}"
)

# Define RTS_HAS_VULKAN when SDK is available
target_compile_definitions(vulkan_interface INTERFACE RTS_HAS_VULKAN)

# Find and link Vulkan library
if(WIN32)
    find_library(VULKAN_LIBRARY NAMES vulkan PATHS "${VULKAN_LIB_DIR}")
elseif(APPLE)
    find_library(VULKAN_LIBRARY NAMES vulkan PATHS "${VULKAN_LIB_DIR}")
else()
    find_library(VULKAN_LIBRARY NAMES vulkan)
endif()

if(NOT VULKAN_LIBRARY)
    message(WARNING "Vulkan library not found")
    set(VULKAN_FOUND FALSE)
    return()
endif()

message(STATUS "Vulkan library: ${VULKAN_LIBRARY}")
target_link_libraries(vulkan_interface INTERFACE ${VULKAN_LIBRARY})

# Link SDL2 if available (for surface creation)
if(TARGET SDL2::SDL2)
    target_link_libraries(vulkan_interface INTERFACE SDL2::SDL2)
endif()

# Export targets
add_library(Vulkan::Vulkan ALIAS vulkan_interface)

set(VULKAN_FOUND TRUE)
message(STATUS "Vulkan SDK configured successfully")
message(STATUS "  Include: ${VULKAN_INCLUDE_DIR}")
message(STATUS "  Library: ${VULKAN_LIB_DIR}")

