cmake_minimum_required(VERSION 3.16)
project(AED_Proyecto)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/mingw_64")

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
        SparseMatrix.h
        SparseMatrix.h
)

target_link_libraries(AED_Proyecto Qt6::Widgets)