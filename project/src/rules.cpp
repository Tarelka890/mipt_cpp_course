#include "agent_rules.h"
#include "rules.h"

#include <ostream>
#include <print>

namespace nano_edr {

const char* SeverityName(Severity severity){
    switch (severity){
        case Severity::kLow:
            return "low";
        case Severity::kMedium:
            return "medium";
        case Severity::kHigh:
            return "high";
        case Severity::kCritical:
            return "critical";
    }
    return "?";
}

}  
