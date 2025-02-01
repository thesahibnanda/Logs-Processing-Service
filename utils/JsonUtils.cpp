#include "../dependencies/json.hpp"
#include <string>
using json = nlohmann::json;

class JsonUtils
{
public:
    static json parseJson(const std::string &jsonString)
    {
        return json::parse(jsonString);
    }

    static std::string stringifyJson(const json &jsonObject)
    {
        return jsonObject.dump();
    }

    static json addDictToJsonArray(json &jsonArray, const json &dict)
    {
        if (!jsonArray.is_array())
        {
            throw std::invalid_argument("Error: jsonArray is not an array.");
        }

        if (jsonArray.is_null() || jsonArray.empty())
        {
            jsonArray = json::array();
        }

        jsonArray.push_back(dict);
        return jsonArray;
    }

    static json emptyJsonDict()
    {
        return json::object();
    }
};
