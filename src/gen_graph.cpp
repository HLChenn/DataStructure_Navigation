/**
 * @file gen_graph.cpp
 * 
 * @note
 *      用于地图生成算法的具体函数实现，对外部提供Graph类作为接口，详细信息见gen_graph.h
 *      使用mst结合三角剖分算法的实现平面连通图的生成 
 */
#include "gen_graph.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<size_t, K> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> DT;
typedef DT::Point CPoint;

const int GRAPH_RANGE = 10000;
const std::string DOT_FILE_DIR ="/home/kc1zs4/Code/Proj/SCUT/SCUT_MapNavigation/test/dev/dot_visualize/";
const int EXTRA_POINT = 20;
const double EXPAND_PROB = 0.3;
const int POINT_NUM = 500;

/* * * * * * * * * * Point Impl  * * * * * * * * * */
bool Point::operator<(const Point& other) const {
    return std::tie(x, y) < std::tie(other.x, other.y);
}

bool Point::operator==(const Point& other) const {
    return std::tie(x, y) == std::tie(other.x, other.y);
}

/* * * * * * * * * * Edge Impl  * * * * * * * * * */
bool Edge::operator<(const Edge& other) const {
    return weight < other.weight;
}

/* * * * * * * * * * Graph Impl  * * * * * * * * * */
Graph::Graph(int n = POINT_NUM) : vertices(n) {
    genPoints();
    removeDuplicates();
    buildNetwork(EXPAND_PROB);
}

/**
 * 将点与边以.dot文件的格式导出，用于临时的可视化
 */
void Graph::visualize(const std::string& filename) const {
    std::ofstream dot_file(DOT_FILE_DIR + filename + ".dot");
    dot_file << "graph Map {\n";
    for (const auto& p : points) {
        dot_file << "    \"" << p.x << "," << p.y << "\" [pos=\"" 
                << p.x << "," << p.y << "!\"];\n";
    }
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& p = points[i];
        for (const auto& neighbor_idx : adj[i]) {
            const auto& neighbor = points[neighbor_idx];
            if (p < neighbor) {
                dot_file << "    \"" << p.x << "," << p.y << "\" -- \"" 
                        << neighbor.x << "," << neighbor.y << "\";\n";
            }
        }
    }
    dot_file << "}\n";
    dot_file.close();
}


/**
 * DFS 判断是否连通
 */
bool Graph::isConnected() {
    if (points.empty()) return true;

    std::vector<bool> visited(points.size(), false);
    std::vector<size_t> stack;
    stack.reserve(points.size());

    visited[0] = true;
    stack.push_back(0);
    size_t visited_count = 1;

    // DFS
    while (!stack.empty()) {
        size_t u = stack.back();
        stack.pop_back();
        for (size_t v : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                ++visited_count;
                stack.push_back(v);
            }
        }
    }

    return visited_count == points.size();
}

/**
 * 用于将 Graph 对象的 points 和 adj 邻接表使用 JSON 特定的格式传输到前端进行渲染
 *      elements: [
 *         { data: { id: '0' }, position: { x: 200, y: 200 } },
 *         { data: { id: '1' }, position: { x: 400, y: 200 } },
 *         { data: { id: '2' }, position: { x: -100, y: 700 } },
 *         { data: { id: '01', source: '0', target: '1' } },
 *         { data: { id: '20', source: '2', target: '0' } }
 *      ]
 */
std::string Graph::getJsonG() const {
    nlohmann::json result;
    nlohmann::json elements = nlohmann::json::array();

    for (size_t i = 0; i < points.size(); ++i) {
        nlohmann::json node;
        node["data"]["id"] = std::to_string(i);
        node["position"]["x"] = points[i].x;
        node["position"]["y"] = points[i].y;
        elements.push_back(node);
    }

    for (size_t i = 0; i < adj.size(); ++i) {
        for (size_t j : adj[i]) {
            if (j > i) {    // 无向边, 确保每一条边只生成一次
                nlohmann::json edge;
                edge["data"]["id"] = std::to_string(i) + "-" + std::to_string(j);
                edge["data"]["source"] = std::to_string(i);
                edge["data"]["target"] = std::to_string(j);
                elements.push_back(edge);
            }
        }
    }

    result["elements"] = elements;

    return result.dump(2);
}

void Graph::buildNetwork(double probability) {
    adj.clear();
    if (points.empty()) return;

    buildDelaunayTriangulation();
    computeMST();

    adj.resize(points.size());
    for (const auto& edge : mst_edges) {
        adj[edge.u].push_back(edge.v);
        adj[edge.v].push_back(edge.u);
    }

    std::set<std::pair<size_t, size_t>> mst_edge_set;
    for (const auto& e : mst_edges) {
        mst_edge_set.emplace(e.u, e.v);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(probability);

    for (const auto& edge : delaunay_edges) {
        if (mst_edge_set.count({edge.u, edge.v})) continue;
        if (dist(gen)) {
            adj[edge.u].push_back(edge.v);
            adj[edge.v].push_back(edge.u);
        }
    }

    dedupAdj();

}

void Graph::dedupAdj() {
    for (auto& neighbors : adj) {
        std::sort(neighbors.begin(), neighbors.end());
        auto last = std::unique(neighbors.begin(), neighbors.end());
        neighbors.erase(last, neighbors.end());
    }
}

void Graph::buildDelaunayTriangulation() {
    std::vector<std::pair<CPoint, size_t>> cgal_points;
    cgal_points.reserve(points.size());
    for (size_t i = 0; i < points.size(); ++i)
        cgal_points.emplace_back(CPoint(points[i].x, points[i].y), i);

    dt.clear();
    dt.insert(cgal_points.begin(), cgal_points.end());

    delaunay_edges.clear();
    for (auto eit = dt.finite_edges_begin(); eit != dt.finite_edges_end(); ++eit) {
        auto edge = *eit;
        auto v1 = edge.first->vertex((edge.second + 1) % 3);
        auto v2 = edge.first->vertex((edge.second + 2) % 3);
        if (!dt.is_infinite(v1) && !dt.is_infinite(v2)) {
            size_t i1 = v1->info();
            size_t i2 = v2->info();
            if (i1 > i2) std::swap(i1, i2);
            float dx = points[i1].x - points[i2].x;
            float dy = points[i1].y - points[i2].y;
            float weight = sqrt(dx*dx + dy*dy);
            delaunay_edges.push_back({i1, i2, weight});
        }
    }

    std::sort(delaunay_edges.begin(), delaunay_edges.end(),
        [](const Edge& a, const Edge& b) { return std::tie(a.u, a.v) < std::tie(b.u, b.v); });
    auto last = std::unique(delaunay_edges.begin(), delaunay_edges.end(),
        [](const Edge& a, const Edge& b) { return a.u == b.u && a.v == b.v; });
    delaunay_edges.erase(last, delaunay_edges.end());
}

void Graph::computeMST() {
    mst_edges.clear();
    if (delaunay_edges.empty()) return;

    std::vector<Edge> sorted_edges = delaunay_edges;
    std::sort(sorted_edges.begin(), sorted_edges.end());

    UnionFind uf(points.size());
    for (const auto& edge : sorted_edges) {
        if (uf.unite(edge.u, edge.v)) {
            mst_edges.push_back(edge);
            if (mst_edges.size() == points.size() - 1) break;
        }
    }
}

/**
 * 引入 EXtRA_POINT 来准确生成指定数量的顶点
 */
void Graph::genPoints() {
    const int total = vertices + EXTRA_POINT;
    std::vector<Point> result;
    if (total <= 0) return;

    unsigned num_threads = std::min<unsigned>(std::thread::hardware_concurrency(), total);
    std::vector<std::thread> threads(num_threads);
    std::vector<std::vector<Point>> thread_points(num_threads);

    std::random_device rd;
    const unsigned base_seed = rd();

    auto thread_task = [&](unsigned thread_id, int points_to_gen) {
        std::mt19937 gen(base_seed + thread_id);
        std::uniform_int_distribution<int> dist(0, GRAPH_RANGE * 10);
        std::vector<Point> local_points;
        local_points.reserve(points_to_gen);
        
        for (int i = 0; i < points_to_gen; ++i) {
            local_points.push_back({dist(gen) * 0.1f, dist(gen) * 0.1f});
        }
        thread_points[thread_id] = std::move(local_points);
    };

    const int chunk = total / num_threads;
    int remainder = total % num_threads;
    for (unsigned i = 0; i < num_threads; ++i) {
        const int points_to_gen = chunk + (i < remainder ? 1 : 0);
        threads[i] = std::thread(thread_task, i, points_to_gen);
    }

    for (auto& t : threads) t.join();
    
    result.reserve(total);
    for (auto& tp : thread_points) {
        result.insert(result.end(), tp.begin(), tp.end());
    }
    points = std::move(result);
}

/**
 * 1-10万数量级点可以正确通过 EXTRA_POINT 来准确生成
 */
void Graph::removeDuplicates() {
    std::sort(points.begin(), points.end());
    auto last = std::unique(points.begin(), points.end());
    if (static_cast<size_t>(vertices) < points.size()) {
        points.resize(vertices);
    } else {
        points.erase(last, points.end());
    }
}


/* * * * * * * * * * Graph::UnionFind Impl  * * * * * * * * * */
Graph::UnionFind::UnionFind(size_t n): parent(n) {
    for (size_t i = 0; i < n; ++i)
        parent[i] = i;
}

size_t Graph::UnionFind::find(size_t u) {
    if (parent[u] != u)
        parent[u] = find(parent[u]);
    return parent[u];
}

bool Graph::UnionFind::unite(size_t u, size_t v) {
    size_t pu = find(u);
    size_t pv = find(v);
    if (pu == pv) return false;
    parent[pu] = pv;
    return true;
}