cmake_minimum_required(VERSION 3.0.0)
project(CapstoneRPi VERSION 0.1.0)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(Image_Recog_Files "Cluster.h" "Cluster.cpp" "ImageState.h" "ImageState.cpp" "PieceRecognition.h" "PieceRecognition.cpp")

add_executable(CapstoneRPi main.cpp ${Image_Recog_Files})

target_link_libraries( CapstoneRPi ${OpenCV_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
