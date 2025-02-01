#include <string>
#include "dependencies/crow_all.h"
#include "utils/FileUtils.cpp"
#include "utils/JsonUtils.cpp"

struct CORSHandler
{
    struct context
    {
    };

    void before_handle(crow::request &req, crow::response &res, context &)
    {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }

    void after_handle(crow::request &req, crow::response &res, context &)
    {
        res.add_header("Access-Control-Allow-Origin", "*");
    }
};

const std::string SERVICE_FILE_PATH = "log/all_services.json";
const std::string SERVICE_FILE_PREFIX = "log/";

int main()
{
    FileUtils::createFileIfNotExists(SERVICE_FILE_PATH);

    crow::App<CORSHandler> app;

    CROW_ROUTE(app, "/<path>")
        .methods(crow::HTTPMethod::OPTIONS)([](const crow::request &, std::string)
                                            {
            crow::response res(204);  // No Content
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return res; });

    CROW_ROUTE(app, "/service/register")
        .methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                         {
            try {
                if (req.body.empty()) {
                    return crow::response(400, "Empty JSON payload");
                }
                auto jsonBody = crow::json::load(req.body);
                if (!jsonBody || !jsonBody.has("service_name")) {
                    return crow::response(400, "Invalid JSON format or missing 'service_name'");
                }

                std::string serviceName = jsonBody["service_name"].s();

                std::string servicesArrayString = FileUtils::readFile(SERVICE_FILE_PATH);
                if (servicesArrayString.empty()) {
                    servicesArrayString = "[]";
                }
                json servicesArray = JsonUtils::parseJson(servicesArrayString);

                for (const auto &service : servicesArray) {
                    if (service == serviceName) {
                        return crow::response(400, "Service already exists");
                    }
                }

                servicesArray.push_back(serviceName);
                FileUtils::writeFile(SERVICE_FILE_PATH, JsonUtils::stringifyJson(servicesArray));

                return crow::response(200, "Service registered successfully");
            }
            catch (const std::exception &e) {
                return crow::response(500, e.what());
            } });

    CROW_ROUTE(app, "/service/add")
        .methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                         {
            try {
                if (req.body.empty()) {
                    return crow::response(400, "Empty JSON payload");
                }
                auto jsonBody = crow::json::load(req.body);
                if (!jsonBody || !jsonBody.has("service_name") || !jsonBody.has("message") || !jsonBody.has("log_level")) {
                    return crow::response(400, "Missing required fields: 'service_name', 'message', or 'log_level'");
                }

                std::string serviceName = jsonBody["service_name"].s();
                std::string path = SERVICE_FILE_PREFIX + serviceName + ".json";

                FileUtils::createFileIfNotExists(path);

                std::string servicesArrayString = FileUtils::readFile(path);
                if (servicesArrayString.empty()) {
                    servicesArrayString = "[]";
                }
                json servicesArray = JsonUtils::parseJson(servicesArrayString);

                json jsonDictionary = JsonUtils::emptyJsonDict();
                jsonDictionary["timestamp"] = std::time(nullptr);
                jsonDictionary["message"] = jsonBody["message"].s();
                jsonDictionary["log_level"] = jsonBody["log_level"].s();

                std::string logLevel = jsonDictionary["log_level"].get<std::string>();
                if (logLevel != "INFO" && logLevel != "DEBUG" && logLevel != "ERROR") {
                    return crow::response(400, "Invalid log level");
                }

                servicesArray = JsonUtils::addDictToJsonArray(servicesArray, jsonDictionary);
                FileUtils::writeFile(path, JsonUtils::stringifyJson(servicesArray));

                return crow::response(200, "Log added successfully");
            }
            catch (const std::exception &e) {
                return crow::response(500, e.what());
            } });

    CROW_ROUTE(app, "/service/get")
        .methods(crow::HTTPMethod::GET)([](const crow::request &req)
                                        {
            try {
                std::string servicesArrayString = FileUtils::readFile(SERVICE_FILE_PATH);
                if (servicesArrayString.empty()) {
                    return crow::response(200, "[]");
                }
                std::string applicationJSON = "application/json";
                return crow::response(200, applicationJSON, servicesArrayString);
            }
            catch (const std::exception &e) {
                return crow::response(500, e.what());
            } });

    CROW_ROUTE(app, "/service/get-one")
        .methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                         {
            try {
                if (req.body.empty()) {
                    return crow::response(400, "Empty JSON payload");
                }
                auto jsonBody = crow::json::load(req.body);
                if (!jsonBody || !jsonBody.has("service_name")) {
                    return crow::response(400, "Invalid JSON format or missing 'service_name'");
                }

                std::string serviceName = jsonBody["service_name"].s();
                std::string path = SERVICE_FILE_PREFIX + serviceName + ".json";

                std::string servicesArrayString = FileUtils::readFile(path);
                if (servicesArrayString.empty()) {
                    return crow::response(200, "[]");
                }
                std::string applicationJSON = "application/json";
                return crow::response(200, applicationJSON, servicesArrayString);
            }
            catch (const std::exception &e) {
                return crow::response(500, e.what());
            } });

    CROW_ROUTE(app, "/service/cleanup")
        .methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                         {
            try {
                if (req.body.empty()) {
                    return crow::response(400, "Empty JSON payload");
                }
                auto jsonBody = crow::json::load(req.body);
                if (!jsonBody || !jsonBody.has("service_name")) {
                    return crow::response(400, "Invalid JSON format or missing 'service_name'");
                }

                std::string serviceName = jsonBody["service_name"].s();
                std::string path = SERVICE_FILE_PREFIX + serviceName + ".json";

                if (!std::filesystem::exists(path)) {
                    return crow::response(400, "Service does not exist.");
                }

                FileUtils::writeFile(path, "[]");
                return crow::response(200, "Service log cleaned up successfully");
            }
            catch (const std::exception &e) {
                return crow::response(500, e.what());
            } });

    CROW_ROUTE(app, "/service/remove")
        .methods(crow::HTTPMethod::DELETE)([](const crow::request &req)
                                           {
        try {
            if (req.body.empty()) {
                return crow::response(400, "Empty JSON payload");
            }

            auto jsonBody = crow::json::load(req.body);
            if (!jsonBody.has("service_name")) {
                return crow::response(400, "Missing 'service_name'");
            }

            std::string serviceName = jsonBody["service_name"].s();
            std::string logFilePath = SERVICE_FILE_PREFIX + serviceName + ".json";

            std::string servicesArrayString = FileUtils::readFile(SERVICE_FILE_PATH);
            if (servicesArrayString.empty()) {
                return crow::response(404, "No services found");
            }

            json servicesArray = JsonUtils::parseJson(servicesArrayString);

            auto it = std::find(servicesArray.begin(), servicesArray.end(), serviceName);
            if (it == servicesArray.end()) {
                return crow::response(404, "Service not found");
            }

            servicesArray.erase(it);
            FileUtils::writeFile(SERVICE_FILE_PATH, JsonUtils::stringifyJson(servicesArray));

            if (std::filesystem::exists(logFilePath)) {
                FileUtils::deleteFile(logFilePath);
            }

            return crow::response(200, "Service removed successfully");
        }
        catch (const std::exception &e) {
            return crow::response(500, e.what());
        } });

    CROW_ROUTE(app, "/healthz")
        .methods(crow::HTTPMethod::GET)([](const crow::request &req)
                                        {
            try {
                return crow::response(200, "OK");
            }
            catch (const std::exception &e) {
                return crow::response(500, e.what());
            } });

    app.port(8080).multithreaded().run();

    return 0;
}
