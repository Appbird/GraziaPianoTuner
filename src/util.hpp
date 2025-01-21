# pragma once
# include <Siv3D.hpp>

#define JSON2Field(json, struct, name) (struct).name = (json)[(U###name)].get<__typeof__(struct.name)>()
#define JSON2Field_Array(json, struct, name, type) \
    for (const auto& element: json[U###name].arrayView()) { \
        struct.name.push_back(element.get<type>()); \
    }
#define Field2JSON(json, name) (json)[(U###name)] = (name)
#define Field2JSON_Array(json, sequence) \
    for (const auto& val : sequence) { \
        result[U###sequence].push_back(val); \
    }


const HSV main_color{210, 0.4, 0.2};

String find_last_abc_block(const String& GPT_answer);
String find_last_axis_block(const String& GPT_answer);