#include <parse.h>

#include <print>

const char* ws = " \t";

namespace nano_edr {
    bool ParseEventParts(const std::string& line, EventParts* out){
        if (IsBlankOrComment(line)) return false;
        
        *out = EventParts();

        const std::string& str = line;

        std::string key = "";
        std::string value = "";
        size_t pos = 0;
        while (pos < str.size()){
            pos = str.find_first_not_of(ws, pos);

            // нашли строку формата =abc или пришли в конец
            if (pos == std::string::npos) break;
            if (str[pos] == '=') return false;

            size_t end_key = str.find_first_of('=', pos);

            // знака равно вообще не нашлось для этого ключа
            if (end_key == std::string::npos) return false;

            key = str.substr(pos, end_key-pos);

            // ключ оказался пуст (хотя в целом такого быть не должно)
            if (key.find_first_of(ws) != std::string::npos) return false;

            pos = end_key;

            // кавычки
            if ((pos + 1 < str.size()) && str[pos+1] == '"'){
                size_t end_value = str.find_first_of('"', pos+2);
                if (end_value == std::string::npos) return false;

                // смотрим на мусор после них
                if (end_value + 1 < str.size() && std::string_view(ws).find(str[end_value + 1]) == std::string::npos) {
                    return false;
                }

                value = str.substr(pos+2, end_value-pos-2);

                pos = end_value + 1;

            }
            else{
                size_t end_value = str.find_first_of(ws, pos+1);
                if (end_value == std::string::npos){
                    end_value = str.size();
                }
                value = str.substr(pos+1, end_value-pos-1);

                pos = end_value;
            }

            // вставляем значения
            if ((key == "ts") && out->ts.empty()) out->ts = value;
            else if ((key == "type") && out->type.empty()) out->type = value;
            else if ((key == "pid") && out->pid.empty()) out->pid = value;
            else out->fields.push_back(Field(key, value));

            key = "";
            value = "";
        
        }

        return true;
    };

    bool IsBlankOrComment(const std::string& line){
        if (line.empty()) return true;

        size_t pos = line.find_first_not_of(ws);
        if (pos == std::string::npos) return true;

        return line[pos] == '#' || line[pos] == ';';
    };

}