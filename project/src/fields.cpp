#include "event.h"
#include "fields.h"

#include <cstdint>
#include <string>
#include <stdexcept>
#include <algorithm>




namespace nano_edr {

// Ищет только среди fields. Шапка события — ts, type, pid — лежит отдельными
// полями и доступна напрямую, и особого случая для этих трёх ключей здесь нет
// намеренно: иначе у одних и тех же данных окажется два пути доступа,
// и читающему код придётся помнить, что ровно три ключа ведут себя
// не как все.
//
// Возвращается указатель, а не копия: копировать строку только чтобы посмотреть
// на неё — расточительство, а событий будет два миллиона. Указатель
// действителен столько же, сколько само событие.
const std::string* FindField(const Event& event, const std::string& key){
    if (key == "ts") return &(event.raw_ts());
    if (key == "type") return &(event.type());
    if (key == "pid") return &(event.pid());
    
    for (const auto& field : event.fields()){
        if (field.key == key){
            return &(field.value);
        }
    }
    return nullptr;
}


// Ищет там же, где FindField, и ровно так же — только среди fields. Шапка
// события сюда не попадает: два пути доступа к одним и тем же данным были бы
// ошибкой и здесь. Разница между двумя функциями одна — что делать, когда поля
// нет.
//
// Бросает std::invalid_argument, если поля нет: раз оно объявлено обязательным,
// его отсутствие — нарушение контракта формата, и вернуть пустую строку значило
// бы соврать вызывающему. Сообщение исключения называет поле: диагностика,
// по которой нельзя найти причину, — половина диагностики.
const std::string& GetRequiredField(const Event& event, const std::string& key){
    const std::string* result = FindField(event, key);
    if (result == nullptr){
        throw std::invalid_argument(key);
    }
    return *result;
}


// Возвращает false, если поля нет или оно не разбирается как целое (или вышли за границы). Второе
// бывает не реже первого: значение приходит из внешнего источника,
// и «size=абв» — битые данные, а не ошибка программы.
bool GetIntField(const Event& event, const std::string& key, uint64_t* out){
    for (const auto& field : event.fields()){
        if (field.key == key){
            if (field.value.empty()){
                return false;
            } 
            if (field.value[0] == '-'){
                return false;
            } 
            try {
                size_t index = 0;
                uint64_t parsed = std::stoull(field.value, &index);
                if (index != field.value.length()){
                    return false;
                }
                *out = parsed;
                return true;
            }
            catch (const std::invalid_argument&){
                return false;
            }
            catch (const std::out_of_range&){
                return false;
            }
        }
    }
    return false;
}

// Обертка для bool GetIntField
// Если мы не смогли получить результат, то возвращаем fallback как значение по умолчанию
// Битые данные - это не ошибка программы, неверный парсинг такого - это не ошибка
uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback){
    uint64_t result = 0;
    if (GetIntField(event, key, &result)){
        return result;
    }
    return fallback;
}

bool IsProcessStart(const Event& event){
    return event.type() == "process_start";
}
bool IsFileWrite(const Event& event){
    return event.type() == "file_write";
}
bool IsNetConnect(const Event& event){
    return event.type() == "net_connect";
}

// Проверка суффикса path с end_with. Нет поля - false, это не исключение, есть события без этого поля
// Регистр не учитывается, для этого вызывается NormalizePath
bool PathEndsWith(const Event& event, const std::string& suffix){
    const std::string* field_pointer = nano_edr::FindField(event, "path");

    if (field_pointer == nullptr) return false;

    std::string line = nano_edr::NormalizePath(*field_pointer);

    if (line.ends_with(nano_edr::NormalizePath(suffix))) return true;

    return false;
}

// Поиск подстроки в cmdline. Нет поля - false, это не исключение, есть события без этого поля
// Регистр не учитывается, для этого вызывается NormalizePath
bool CommandLineContains(const Event& event, const std::string& needle){
    const std::string* field_pointer = nano_edr::FindField(event, "cmdline");

    if (field_pointer == nullptr) return false;

    std::string line = nano_edr::NormalizePath(*field_pointer);

    if (line.find(nano_edr::NormalizePath(needle)) != std::string::npos) return true;

    return false;

}

// Нормализация пути: перевод в нижний регистр, замена на \ и раскрытие %TEMP% + %TMP%.
std::string NormalizePath(const std::string& path){
    std::string answer = path;

    size_t write_index = 0;

    for (size_t i = 0; i < answer.size(); i++){
        unsigned char c = static_cast<unsigned char>(answer[i]);
        if (c == '/'){
            c = '\\';
        }
        
        if (c == '\\' && i > 0 && answer[i - 1] == '\\'){
            continue; 
        }

        answer[write_index] = static_cast<char>(std::tolower(c));
        write_index++;
    }
    answer.resize(write_index);

    static const std::string targets[] = {
        "\%temp\%", "\%tmp\%"
    };

    for (const auto& target : targets){
        auto ptr = answer.find(target);
        while (ptr != std::string::npos){
            answer.replace(ptr, std::string_view(target).length(), "\\appdata\\local\\temp");        
            ptr = answer.find(target);
        }
    }

    return answer;
}

}  // namespace nano_edr

