file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cc include/*.h)

find_package(PCL 1.10 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/io)

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# ================================================================
# add matplotlib-cpp
add_library(path_plan_matplotlib_cpp INTERFACE)
target_include_directories(path_plan_matplotlib_cpp INTERFACE ${PROJECT_SOURCE_DIR}/common)
target_compile_features(path_plan_matplotlib_cpp INTERFACE
  cxx_std_11
)
# TODO: Use `Development.Embed` component when requiring cmake >= 3.18
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
target_link_libraries(path_plan_matplotlib_cpp INTERFACE
  Python3::Python
  Python3::Module
)
find_package(Python3 COMPONENTS NumPy)
if(Python3_NumPy_FOUND)
  target_link_libraries(path_plan_matplotlib_cpp INTERFACE
    Python3::NumPy
  )
else()
  target_compile_definitions(path_plan_matplotlib_cpp INTERFACE WITHOUT_NUMPY)
endif()
install(
  TARGETS path_plan_matplotlib_cpp
  EXPORT install_targets
)
# ================================================================

add_library(path_plan STATIC ${srcs})
target_include_directories(path_plan PUBLIC include)
target_link_libraries(path_plan ${OpenCV_LIBS} ${PCL_LIBRARIES} path_plan_matplotlib_cpp)

add_executable(easy_dijkstra_test test/easy_dijkstra_test.cc)
target_link_libraries(easy_dijkstra_test)

add_executable(dijkstra_test test/dijkstra_test.cc)
target_link_libraries(dijkstra_test path_plan)

add_executable(a_star_test test/a_star_test.cc)
target_link_libraries(a_star_test path_plan)

# INSTALL
install(
  TARGETS easy_dijkstra_test a_star_test
  ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/build/install)
