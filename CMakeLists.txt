cmake_minimum_required(VERSION 3.15)
project(MyGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Raylib ---
# Opción A: si instalaste Raylib con el package manager del sistema
#   Linux:   sudo apt install libraylib-dev   (o equivalente)
#   macOS:   brew install raylib
#   Windows: vcpkg install raylib
find_package(raylib 5.5 REQUIRED)

# --- Opción B (comentada): FetchContent si no tenés Raylib instalado
# include(FetchContent)
# FetchContent_Declare(raylib
#     GIT_REPOSITORY https://github.com/raysan5/raylib.git
#     GIT_TAG        5.5)
# FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME}
    main.cpp
    inputmanager.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Copiar carpeta de assets al directorio de build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/files
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/files
    COMMENT "Copiando assets..."
    VERBATIM
)
