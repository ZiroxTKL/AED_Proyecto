cmake_minimum_required(VERSION 3.16)  # ⚠️ 4.2 no es estándar aún
project(AED_Proyecto)

set(CMAKE_CXX_STANDARD 17)

# Ruta a Qt
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/mingw_64")

# 🔥 IMPORTANTE para Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(AED_Proyecto
        main.cpp
        Node.h
        SparseMatrix.cpp
        SparseMatrix.h
        Mainwindow.cpp
        Mainwindow.h
)

# 🔥 ESTO TE FALTA
target_link_libraries(AED_Proyecto Qt6::Widgets)