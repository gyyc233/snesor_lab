file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cc include/*.h)

find_package(PCL 1.14.1 REQUIRED)
find_package(glog 0.6.0 REQUIRED)
find_package(gflags REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Ceres REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/g2o_test/cmake_modules)
SET(G2O_ROOT /usr/local/include/g2o)
find_package(G2O REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories (${gflags_INCLUDE_DIR})
include_directories (${G2O_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/io)
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/feature_detect) # orb detect

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_library(pose_estimation STATIC ${srcs})

target_include_directories(pose_estimation PUBLIC include)
target_link_libraries(pose_estimation ${OpenCV_LIBS} ${PCL_LIBRARIES} ${G2O_CORE_LIBRARY} ${G2O_STUFF_LIBRARY} Ceres::ceres glog::glog gflags feature_detect)

add_executable(epipolar_constraint_test test/epipolar_constraint_test.cc)
target_link_libraries(epipolar_constraint_test pose_estimation)

# INSTALL
install(
  TARGETS epipolar_constraint_test pose_estimation feature_detect
  ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/build/install)
