#include <event.h>

namespace nano_edr{
    Event::Event(const EventParts& parts){
        if (parts.ts.empty() || parts.type.empty()){
            throw std::invalid_argument("ts/type is empty");
        }

        try {
            std::size_t index = 0;
            uint64_t parsed = std::stoull(parts.ts, &index);

            if (index != parts.ts.length()) {
                throw std::invalid_argument(std::format("invalid characters: {}", parts.ts));
            }

            ts_ = Timestamp(parsed); 

        } catch (const std::invalid_argument&) {
            throw std::invalid_argument(std::format("invalid timestamp format: {}", parts.ts));
        } catch (const std::out_of_range&) {
            throw std::invalid_argument(std::format("value is out of range (too large): {}", parts.ts));
        }

        raw_ts_ = parts.ts;
        type_ = parts.type;
        pid_ = parts.pid;
        fields_ = parts.fields;
    
    }



    std::string ToString(const Event& event){
        std::string result = "";
        result.append(std::format("ts={} ", event.raw_ts()));
        result.append(std::format("type={}", event.type()));

        if (!event.pid().empty()) result.append(std::format("pid={} ", event.pid()));

        for (const auto& field : event.fields()){
            result.append(std::format("{}={} ", field.key, field.value));
        }

        return result;
    }

    std::ostream& operator<<(std::ostream& out, const Event& event){
        std::string str = ToString(event);
        out<<str;
        return out;
    }
};