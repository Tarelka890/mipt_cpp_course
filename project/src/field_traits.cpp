#include "field_traits.h"
#include "text.h"

namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& text, std::string* out) {
    *out = text;
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t* out) {
    if (text == "") return false;

    uint64_t parsed;

    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), parsed);

    if (ptr != text.data() + text.size() || ec == std::errc::invalid_argument || ec == std::errc::result_out_of_range){
        return false;
    }
    
    *out = parsed;

    return true;
}
bool FieldTraits<bool>::Parse(const std::string& text, bool* out) {
    std::string text_lower = ToLower(text);
    if (text_lower == "1" || text_lower == "yes" || text_lower == "true"){
        *out = true;
        return true;
    }
    if (text_lower == "0" || text_lower == "no" || text_lower == "false"){
        *out = false;
        return true;
    }
    return false;
}

}  // namespace nano_edr
