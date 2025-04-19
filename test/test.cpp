/**
 * @file test.cpp
 * @note
 *      项目测试样例的入口文件
 *      对于自定义添加的测试文件与样例，需要将其.cpp文件通过 #include 语句进行引入
 *      详见 Test Demo
 */
#include <gtest/gtest.h>
#include <vector>



/* * * * * * * * * * Test Demo * * * * * * * * * */
// #include "demo.cpp"
// std::vector<std::string> tests_to_run = {
//     "correctExample.BasicAssertion"
// };



/* * * * * * * * * * Main Test * * * * * * * * * */
#include "dev/gen_graph.cpp"

std::vector<std::string> tests_to_run = {
    // "GenGraphTest.GenMapSpeed",
    "GenGraphTest.MapShowDot",
    "GenGraphTest.MapRange",
    "GenGraphTest.MapConnected"
};

int runTests(const std::vector<std::string>&);
/**
 * @note
 *      - 运行特定的测试样例: 在 全局变量tests_to_run 中写入样例的名称
 *      - 运行全部的测试样例: 将 全局变量tests_to_run 置空
 *      详见 Test Demo
 */
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return runTests(tests_to_run);
}



/* * * * * * * * * * Helper Function * * * * * * * * * */
int runTests(const std::vector<std::string>& testNames) {
    if (testNames.empty()) {
        return RUN_ALL_TESTS();
    }
    std::ostringstream oss;
    for (size_t i = 0; i < testNames.size(); ++i) {
        oss << testNames[i];
        if (i + 1 < testNames.size()) {
            oss << ":";
        }
    }
    ::testing::GTEST_FLAG(filter) = oss.str();
    return RUN_ALL_TESTS();
    
}