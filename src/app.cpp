#include <drogon/drogon.h>
#include <filesystem>

int main()
{
    if (!std::filesystem::exists("views"))
        std::filesystem::create_directory("views");

    drogon::app()
        .setDocumentRoot(std::filesystem::absolute("views").string())
        .registerHandler(
            "/",
            [](const drogon::HttpRequestPtr &,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback)
            {
                drogon::HttpViewData data;
                data.insert("message", "Drogon");
                callback(drogon::HttpResponse::newHttpViewResponse("Index", data));
            },
            {drogon::Get})
        .addListener("127.0.0.1", 5000);

    std::cout << "服务运行在: http://127.0.0.1:5000\n";

    drogon::app().run();
    return 0;
}
