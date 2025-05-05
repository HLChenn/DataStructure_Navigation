/**
 * @file app.cpp
 * 
 * @note
 *      用于启动web服务，最终在此web应用中进行整体功能的汇总
 */

#include <drogon/drogon.h>
#include <memory>
#include "gen_graph.h"

/**
 * 启动 drogon 服务器, 提供三个路由
 *     -  "/": 返回/public下的index.html
 *     - "/getJsonG": 调用 graph 对象 (每一个session对应一个graph) 的 getJsonG()方法, 详见 gen_graph.cpp
 *     - "/aStar": 待实现的最短路径实现
 */
int main()
{
    drogon::app().loadConfigFile("config.json");
    
    drogon::app()

        // route: /
        .registerHandler(
            "/",
            [](const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {

                callback(drogon::HttpResponse::newFileResponse("public/index.html"));

            },
            {drogon::Get})

        // route: /aStar
        .registerHandler(
            "/getJsonG",
            [](const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {

                auto session = req->session();
                if (!session->find("graph")) {
                    session->insert("graph", std::make_shared<Graph>(POINT_NUM));
                }
                auto graph = session->get<std::shared_ptr<Graph>>("graph");
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
                resp->setBody(graph -> getJsonG());

                callback(resp);
            },
            {drogon::Get})
        
        // route: /aStar
        .registerHandler(
            "/aStar",
            [](const drogon::HttpRequestPtr &req,

                std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setBody("/aStar to be done");
                callback(resp);

            },
            {drogon::Get})
        
        // route to 404
        .setCustom404Page(
            drogon::HttpResponse::newFileResponse("public/404.html")
        );

    drogon::app().run();
    return 0;
}
