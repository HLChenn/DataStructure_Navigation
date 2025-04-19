cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0167 NEW)
project(SCUT_MapNavigation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_BUILD_TYPE Release)
# -------------------------- 依赖加载 --------------------------
find_package(Drogon REQUIRED)
find_package(GTest REQUIRED)
file(GLOB SCP_LIST ${PROJECT_SOURCE_DIR}/views/*.csp)
foreach(cspFile ${SCP_LIST})
    message(STATUS "cspFile: ${cspFile}")
    get_filename_component(classname ${cspFile} NAME_WE)
    message(STATUS "view classname: ${classname}")
    add_custom_command(
        OUTPUT ${classname}.h ${classname}.cc
        COMMAND drogon_ctl create view ${cspFile}
        DEPENDS ${cspFile}
        VERBATIM
    )
    list(APPEND VIEWSRC ${classname}.cc)
endforeach()

find_package(CGAL REQUIRED) 

# -------------------------- 主程序配置 --------------------------
add_library(SCUT_MapNavigationLib STATIC
    src/gen_graph.cpp
)
add_executable(SCUT_MapNavigation
    src/app.cpp
    ${VIEWSRC}
)
target_link_libraries(SCUT_MapNavigation PRIVATE
    SCUT_MapNavigationLib
    Drogon::Drogon 
)
target_include_directories(SCUT_MapNavigationLib PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# -------------------------- 测试配置 --------------------------
enable_testing()
add_executable(UnitTests
    test/test.cpp

)
target_link_libraries(UnitTests PRIVATE
    SCUT_MapNavigationLib
    GTest::GTest
    GTest::Main
    pthread
    CGAL::CGAL
)
target_include_directories(UnitTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

gtest_discover_tests(
    UnitTests
    EXTRA_ARGS "--gtest_output=xml:${PROJECT_SOURCE_DIR}/test/UnitTests.xml"
    # XML_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/test/test_res/
    # PROPERTIES LABELS "Core"
)