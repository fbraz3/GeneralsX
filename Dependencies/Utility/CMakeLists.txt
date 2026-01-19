add_library(core_utility INTERFACE)

target_include_directories(core_utility INTERFACE 
    "."
)

if(IS_VS6_BUILD)
    target_include_directories(core_utility INTERFACE "Compat")
endif()
