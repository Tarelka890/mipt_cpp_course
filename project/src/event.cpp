#include <event.h>
#include <cstring>

namespace nano_edr{
    Event::Event(const EventParts& parts){
        if (parts.ts.empty() || parts.type.empty()){
            throw std::invalid_argument("ts/type is empty");
        }

        uint64_t parsed;

        auto [ptr, ec] = std::from_chars(parts.ts.data(), parts.ts.data() + parts.ts.size(), parsed);

        if (ptr != parts.ts.data() + parts.ts.size()){
            throw std::invalid_argument(std::format("didn't parse the whole string: {}", parts.ts));
        }
        if (ec == std::errc::invalid_argument)
            throw std::invalid_argument(std::format("invalid timestamp format: {}", parts.ts));
        else if (ec == std::errc::result_out_of_range)
            throw std::invalid_argument(std::format("value is out of range (too large): {}", parts.ts));

        ts_ = Timestamp(parsed); 

        raw_ts_ = parts.ts;
        type_ = parts.type;
        event_type_ = EventTypeFromString(type_);
        pid_ = parts.pid;
        fields_ = parts.fields;
    
    }

    EventType EventTypeFromString(const std::string& type){
        if (type == "process_start") return EventType::kProcessStart;
        if (type == "process_end") return EventType::kProcessEnd;
        if (type == "net_connect") return EventType::kNetConnect;
        if (type == "file_write") return EventType::kFileWrite;
        if (type == "file_move") return EventType::kFileMove;
        if (type == "file_delete") return EventType::kFileDelete;
        if (type == "file_create") return EventType::kFileCreate;

        return EventType::kOther;
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