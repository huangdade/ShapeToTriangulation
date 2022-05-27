
project(ShapeToTriangulation)

# OCCT
find_package(OpenCASCADE REQUIRED)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE ShapeToTriangulation.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
        ${OpenCASCADE_INCLUDE_DIR}
)


