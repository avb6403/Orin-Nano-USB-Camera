cmake_minimum_required(VERSION 3.5)
project(Orin_Nano_USB_Cam)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add executable
add_executable(main
    main.cpp
    camera_interface.cpp
    gui.cpp
)

# Include directories for header files
target_include_directories(main PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc  # Adjust this path to match your actual directory structure
)

# Link libraries
target_link_libraries(main PRIVATE
    ${OpenCV_LIBRARIES}
)

# Set output directory for binaries
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Optional: Copy necessary files (like images, configuration files) to build directory
# This can be useful for deployment
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../test DESTINATION ${CMAKE_BINARY_DIR}/bin)

# Optional: Install the executable to a specific directory
install(TARGETS main DESTINATION bin)
