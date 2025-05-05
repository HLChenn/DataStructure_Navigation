#ifndef GEN_GRAPH_H
#define GEN_GRAPH_H

/**
 * @file gen_graph.h
 * @note
 *      作为gen_graph.cpp的头文件，提供数据结构与接口生成地图
 *      提供Graph类作为接口，详见Graph类注释
 */
#include <vector>
#include <algorithm>
#include <thread>
#include <random>
#include <fstream>
#include <gtest/gtest.h>
#include <unordered_map>
#include <string>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <nlohmann/json.hpp>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<size_t, K> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> DT;
typedef DT::Point CPoint;

/**
 * DOT_FILE_DIR: linux下的目录格式，需要指定为自己的全局路由，请自行修改
 * EXTRA_POINT:
 *      用于生成更多的points用于后续截断（1~10万）
 *      对于不同的 POINT_NUM 应该进行调整，否则 .dot 图会不协调（sort的原因）
 */
extern const int GRAPH_RANGE;
extern const std::string DOT_FILE_DIR;
extern const int EXTRA_POINT;
extern const double EXPAND_PROB;
extern const int POINT_NUM;

/* * * * * * * * * * Data Structure  * * * * * * * * * */

struct Point {
    float x;
    float y;

    bool operator<(const Point& other) const;
    bool operator==(const Point& other) const;
};

struct Edge {
    size_t u;
    size_t v;
    float weight;

    bool operator<(const Edge& other) const;
};


/**
 * @brief
 *      生成地图的关键类，注意 public 中的变量和函数即可进行二次开发
 */
class Graph {
public:

    /*
     * @var int vertices
     *      节点数
     * @var std::vector<Point> points
     *      构成图的所有节点
     * @var std::vector<std::vector<size_t>> adj
     *      以邻接表表示的无向边，索引对应 points
     *      比如: adj[0][0] = 1，说明  points[0] 与 points[1] 存在边
     */
    int vertices;
    std::vector<Point> points;
    std::vector<std::vector<size_t>> adj;

    /*
     * @func Graph(int n)
     *      构造函数，直接生成一个指定数量n个顶点的不交叉且连通的地图
     * @func visualize(const std::string& filename)
     *      将Graph对象以.dot文件的格式存储在 filename 中
     *      位于 gen_graph.cpp 中 DOT_FILE_DIR 全局变量指定的文件夹中
     * @func isConnected()
     *      判断图是否连通，true 为连通，返回 false 反之
     * @func getJsonG()
     *       获取前端  cytoscape.js 所需要的element json格式，示例如下:
     *       elements: [
     *          { data: { id: '0' }, position: { x: 200, y: 200 } },
     *          { data: { id: '1' }, position: { x: 400, y: 200 } },
     *          { data: { id: '2' }, position: { x: -100, y: 700 } },
     *          { data: { id: '01', source: '0', target: '1' } },
     *          { data: { id: '20', source: '2', target: '0' } }
     *       ]
     */
    Graph(int n);
    void visualize(const std::string& filename) const;
    bool isConnected();
    std::string getJsonG() const;

private:
    struct UnionFind {
        std::vector<size_t> parent;

        UnionFind(size_t n);
        size_t find(size_t u);
        bool unite(size_t u, size_t v);
    };

    DT dt;
    std::vector<Edge> delaunay_edges;
    std::vector<Edge> mst_edges;

    void buildNetwork(double probability);
    void dedupAdj();
    void buildDelaunayTriangulation();
    void computeMST();
    void genPoints();
    void removeDuplicates();
};

#endif