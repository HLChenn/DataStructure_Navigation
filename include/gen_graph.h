/**
 * @file gen_graph.h
 * @note
 *      作为gen_graph.cpp的头文件，提供数据结构与接口生成地图
 *      使用了cgal 6.0.1 版本进行三角剖分算法的实现
 */
#include <vector>
#include <algorithm>
#include <thread>
#include <random>
#include <fstream>
#include <gtest/gtest.h>
#include <unordered_map>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<size_t, K> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> DT;
typedef DT::Point CPoint;


/* * * * * * * * * * Data Structure  * * * * * * * * * */
struct Point {
    float x;
    float y;

    bool operator<(const Point& other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }

    bool operator==(const Point& other) const {
        return std::tie(x, y) == std::tie(other.x, other.y);
    }
};

struct Edge {
    size_t u;
    size_t v;
    float weight;

    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};


class Graph {
public:

    int vertices;
    std::vector<Point> points;
    std::vector<std::vector<size_t>> adj;

    Graph(int n);
    void visualize(const std::string& filename) const;
    bool isConnected();

private:
    struct UnionFind {
        std::vector<size_t> parent;

        UnionFind(size_t n) : parent(n) {
            for (size_t i = 0; i < n; ++i)
                parent[i] = i;
        }

        size_t find(size_t u) {
            if (parent[u] != u)
                parent[u] = find(parent[u]);
            return parent[u];
        }

        bool unite(size_t u, size_t v) {
            size_t pu = find(u);
            size_t pv = find(v);
            if (pu == pv) return false;
            parent[pu] = pv;
            return true;
        }
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
