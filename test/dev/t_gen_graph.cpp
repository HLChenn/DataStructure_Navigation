/**
 * @file gen_graph.cpp
 * @note
 *      经过CMakeLists.txt配置，测试运行后结果会被保存到 ${PROJECT_SOURCE_DIR}/test/UniTests.xml
 */
#include "gen_graph.h"
#include <gtest/gtest.h>

const int TEST_POINT_NUM = 1000;    // GRAPH_RANGE 需要在 gen_graph.h 里调节
const int TEST_GRAPH_RANGE = 10000;

/**
 * 1-10万数量级点生成测速，无视 TEST_POINT_NUM
 */
TEST(GenGraphTest, GenMapSpeed) {

    RecordProperty("SEPERATE_GenMapSpeed", " ========================================");
    for (int incr = 10; incr < 110;incr+=10) {
        for (int i = 1; i < 4; i++) {
            int n = 1000 * incr;    // 1000 * (10 ~ 100)

            auto start = std::chrono::high_resolution_clock::now();

            Graph graph(n);

            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start
            ).count();

            EXPECT_EQ((graph.points).size(), n);

            RecordProperty("GenVec_i" + std::to_string(incr)+"-"+std::to_string(i), ms);
        }
    }   
}

/**
 * 可视化1000个点，导出 .dot 文件
 */
TEST(GenGraphTest, MapShowDot) {

    RecordProperty("SEPERATE_MapShowDot", " ========================================");
    for (int i = 1; i < 4; i++) {

        auto start = std::chrono::high_resolution_clock::now();

        Graph graph(1000);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start
            ).count();

        EXPECT_EQ(graph.points.size(), 1000);  // 可以恰好生成
        RecordProperty("MapShowDot_i" + std::to_string(i)+"-"+std::to_string(i), ms);
        graph.visualize("MapShowDot_i"+std::to_string(i));

    }
}

/**
 * 地图生成所有点的范围
 */

TEST(GenGraphTest, MapRange) {
    for (int i = 0; i < 10; i ++) {
        Graph graph(TEST_POINT_NUM);
        std::vector<Point>& ptr2vec = graph.points; // 引用加快速度
        bool all_in_range = std::all_of(
            ptr2vec.begin(), ptr2vec.end(),
            [](const Point& p) {
                return p.x >= 0.0f && p.x <= TEST_GRAPH_RANGE * 1.0f
                    && p.y >= 0.0f && p.y <= TEST_GRAPH_RANGE * 1.0f;
            }
        );
        EXPECT_TRUE(all_in_range);
    }
}

/**
 * 地图是否连通
 */
TEST(GenGraphTest, MapConnected) {
    for (int i = 0; i < 10; i++) {
        Graph graph(TEST_POINT_NUM);
        EXPECT_TRUE(graph.isConnected());
    }
}
