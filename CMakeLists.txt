cmake_minimum_required(VERSION 3.0.0)
project(CapstoneRPi VERSION 0.1.0)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(Image_Recog_H_Files "Cluster.h" "ImageState.h" "PieceRecognition.h" "FSM.h")
set(Image_Recog_CPP_Files "Cluster.cpp" "ImageState.cpp" "PieceRecognition.cpp" "FSM.cpp")

add_executable(CapstoneRPi main.cpp ${Image_Recog_H_Files} ${Image_Recog_CPP_Files})

target_link_libraries( CapstoneRPi ${OpenCV_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
