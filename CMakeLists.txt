set(CMAKE_TOOLCHAIN_FILE ~/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(Boost_NO_WARN_NEW_VERSIONS true)
set(ENV{LD_LIBRARY_PATH} "$ENV{HOME}/vcpkg/installed/x64-linux/lib:$ENV{HOME}/vcpkg/installed/x64-linux/lib")
project(rtimv)

find_package(FFTW3 CONFIG REQUIRED)
find_package(FFTW3f CONFIG REQUIRED)
find_package(FFTW3l CONFIG REQUIRED)
find_package(ZeroMQ CONFIG REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(GSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "forms")

add_library(rtimv 
    src/rtimvGraphicsView.cpp
    src/StretchBox.cpp
    src/StretchCircle.cpp
    src/StretchLine.cpp
)
target_include_directories(rtimv PRIVATE ${Qt5Widgets_INCLUDE_DIRS})
target_compile_options(rtimv PUBLIC "-fPIC")

add_executable(rtimview
    src/rtimvBase.cpp
    src/rtimvMainWindow.cpp
    src/rtimvMain.cpp
    src/rtimvControlPanel.cpp
    src/rtimvStats.cpp
    src/images/shmimImage.cpp
    src/images/fitsImage.cpp
    src/images/mzmqImage.cpp
    forms/rtimvMainWindow.ui
    forms/imviewerControlPanel.ui
    forms/imviewerStats.ui 
)
target_include_directories(rtimview PRIVATE ${Qt5Widgets_INCLUDE_DIRS} src)
target_compile_options(rtimview PUBLIC "-fPIC")
target_link_directories(rtimview PUBLIC "$ENV{HOME}/vcpkg/installed/x64-linux/lib")
target_link_libraries(rtimview PUBLIC 
    libzmq 
    xrif-shared 
    Qt5::Widgets 
    ImageStreamIO 
    rtimv 
    cfitsio 
    z 
    mx
    FFTW3::fftw3 FFTW3::fftw3f FFTW3::fftw3l 
    GSL::gsl GSL::gslcblas 
    ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

install(TARGETS rtimv DESTINATION lib)
install(TARGETS rtimview DESTINATION bin)