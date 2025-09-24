FetchContent_Declare(
    miles
    GIT_REPOSITORY https://github.com/TheSuperHackers/miles-sdk-stub.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(miles)

# Criar alias Miles::Miles para milesstub
if(TARGET milesstub)
    add_library(Miles::Miles ALIAS milesstub)
endif()
