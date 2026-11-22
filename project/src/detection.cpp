#include "detection.h"
#include "rules.h"

#include <format>

namespace nano_edr{
    std::string ToString(const Detection& detection){
        return std::format("[DETECT] {}  {}  ts={} pid={}{}", 
            SeverityName(detection.severity), 
            detection.rule, 
            detection.ts.ms, 
            detection.pid, 
            detection.chain.empty() ? "" : std::format(" chain={}", detection.chain)
        );
    }
}

