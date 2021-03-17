cmake_minimum_required(VERSION 3.5)
project(darknet_ros)

set(CMAKE_CXX_STANDARD
    14
    CACHE STRING "The C++ standard to use"
)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find dependencies
find_package(ament_cmake REQUIRED)
# Core
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
# Message deps
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(perception_interfaces REQUIRED)
# OpenCV stuff
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)

set(INSTALL_BIN_DIR
    bin
    CACHE INTERNAL "" FORCE
)
set(INSTALL_LIB_DIR
    lib
    CACHE INTERNAL "" FORCE
)
add_subdirectory(deps/darknet)
add_subdirectory(deps/DarkHelp)

set(ament_deps
    # Core dependency
    rclcpp
    # Message deps
    std_msgs
    sensor_msgs
    perception_interfaces
    # OpenCV stuff
    cv_bridge
    image_transport
    rclcpp_components
)

add_executable(object_detector src/object_detector.cc)
target_link_libraries(object_detector PRIVATE DarkHelp dark)
target_compile_features(object_detector PRIVATE cxx_std_17)
ament_target_dependencies(object_detector PUBLIC ${ament_deps})

install(TARGETS object_detector DESTINATION lib/${PROJECT_NAME})

ament_package()
