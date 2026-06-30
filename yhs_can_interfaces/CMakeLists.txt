cmake_minimum_required(VERSION 3.8)
project(yhs_can_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/BmsFlagInfoFb.msg"
  "msg/BmsInfoFb.msg"
  "msg/ChassisInfoFb.msg"
  "msg/CtrlCmd.msg"
  "msg/CtrlFb.msg"
  "msg/IoCmd.msg"
  "msg/IoFb.msg"
  "msg/LrWheelFb.msg"
  "msg/RrWheelFb.msg"
  "msg/OdoFb.msg"
  "msg/VehDiagFb.msg"
  "msg/Ultrasonic.msg"
  DEPENDENCIES std_msgs
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
