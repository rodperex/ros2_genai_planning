cmake_minimum_required(VERSION 3.8)
project(genai_planner)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)
find_package(ros2openai_interfaces REQUIRED)
find_package(genai_planning_interfaces REQUIRED)
find_package(knowledge_graph_interfaces REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(dependencies
  rclcpp
  std_srvs
  ros2openai_interfaces
  genai_planning_interfaces
  knowledge_graph_interfaces
)

include_directories(
include
)

add_library(${PROJECT_NAME} SHARED
  src/genai_planner/GenAIPlanner.cpp
)
ament_target_dependencies(${PROJECT_NAME} ${dependencies})

install(DIRECTORY include/
  DESTINATION include/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_include_directories(include)
ament_export_dependencies(${dependencies})
ament_export_libraries(${PROJECT_NAME})
ament_package()
