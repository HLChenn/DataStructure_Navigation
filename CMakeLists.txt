cmake_minimum_required(VERSION 3.10)
project(SCUT_MapNavigation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_BUILD_TYPE Release)
# -------------------------- 依赖加载 --------------------------
find_package(Drogon REQUIRED)   # 1.9.10
find_package(GTest REQUIRED)    # 1.17.0
find_package(CGAL REQUIRED)     # 6.0.1
find_package(nlohmann_json REQUIRED) # 3.12.0

# -------------------------- 主程序配置 --------------------------
add_executable(SCUT_MapNavigation
    src/app.cpp
    src/gen_graph.cpp
)
target_include_directories(SCUT_MapNavigation PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(SCUT_MapNavigation PRIVATE
    Drogon::Drogon
    CGAL::CGAL
    pthread
    nlohmann_json::nlohmann_json
)

# -------------------------- 测试配置 --------------------------
enable_testing()
add_executable(UnitTests
    test/test.cpp
    src/gen_graph.cpp
)
target_include_directories(UnitTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
# 此处的 libs 是要测试的 .cpp 文件的 libs，不需要完全和
target_link_libraries(UnitTests PRIVATE
    GTest::GTest
    GTest::Main
    pthread
    CGAL::CGAL
)

gtest_discover_tests(
    UnitTests
    EXTRA_ARGS "--gtest_output=xml:${PROJECT_SOURCE_DIR}/test/UnitTests.xml"
    # XML_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/test/test_res/
    # PROPERTIES LABELS "Core"
)