#include <string>

namespace nano_edr{
    std::string ToLower(const std::string& path){
        std::string answer = path;

        for (size_t i = 0; i < answer.size(); i++){
            unsigned char c = static_cast<unsigned char>(answer[i]);
            answer[i] = static_cast<char>(std::tolower(c));
        }

        return answer;
    }
}