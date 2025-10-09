cmake_minimum_required (VERSION 2.6)

#define the project
project (libgpio_rpi4)

#add lib subdirectory to build
add_subdirectory (lib)

#include directories to building path
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/lib)


# add the ejecutable principal
add_executable(server server.c auth_utils.c car_control.c)
#link against required libraries
target_link_libraries(server libgpio_rpi4 microhttpd ssl crypto)

# agregar ejecutable test_gpio
add_executable(test_gpio test_gpio.c)
target_link_libraries(test_gpio libgpio_rpi4)

# agregar ejecutable test_pwm
add_executable(test_pwm test_pwm.c)
target_link_libraries(test_pwm libgpio_rpi4)

#install the binary
install (TARGETS server test_gpio test_pwm DESTINATION bin)

#package source generation
set(MAJOR "0")
set(MINOR "1")
set(PATCH "1")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${MAJOR}.${MINOR}.${PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)