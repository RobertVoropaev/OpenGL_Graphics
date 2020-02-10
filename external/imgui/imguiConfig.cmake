include(CMakeFindDependencyMacro)
find_dependency(glew CONFIG)
find_dependency(glfw3 CONFIG)

include("${CMAKE_CURRENT_LIST_DIR}/imguiTargets.cmake")
