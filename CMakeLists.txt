cmake_minimum_required(VERSION 3.20)
project(Blanchiniiland)

set(CMAKE_CXX_STANDARD 17)

# Configura las rutas a las bibliotecas y encabezados de SFML
set(SFML_DIR "C:/Program Files/JetBrains/CLion 2023.2.2/bin/mingw/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

add_executable(Blanchiniiland main.cpp)

# Enlaza las bibliotecas de SFML a tu programa
target_link_libraries(Blanchiniiland PRIVATE sfml-graphics sfml-audio sfml-window sfml-network sfml-system)
target_link_libraries(Blanchiniiland PRIVATE ws2_32)
