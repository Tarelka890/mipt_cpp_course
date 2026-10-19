#include <string>

namespace nano_edr{
    std::string ToLower(const std::string& path){
        std::string answer = path;

        // size_t write_index = 0;

        for (size_t i = 0; i < answer.size(); i++){
            unsigned char c = static_cast<unsigned char>(answer[i]);
            // if (c == '/'){
            //     c = '\\';
            // }
            
            // if (c == '\\' && i > 0 && answer[i - 1] == '\\'){
            //     continue; 
            // }

            answer[i] = static_cast<char>(std::tolower(c));
            // write_index++;
        }
        // answer.resize(write_index);

        return answer;
    }
}