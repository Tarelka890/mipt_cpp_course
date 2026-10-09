#include "agent_rules.h"
#include "fields.h"
#include "rules.h"
#include <algorithm>
#include <string>

namespace nano_edr {

    const std::string lolbin_checks[] = {
        "urlcache", "transfer", "http:", "https:"
    };

    const std::string powershell_checks[] = {
        "-w hidden", "-windowstyle hidden", "-enc", "-encodedcommand"
    };

    constexpr Rule kRules[] = {
        {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
        {"lolbin_download", LolbinDownload, Severity::kHigh},
        {"hidden_powershell", HiddenPowershell, Severity::kMedium},
        {"autostart_write", AutostartWrite, Severity::kHigh},
        {"ransom_extension", RansomExtension, Severity::kCritical}
    };

    bool ScriptHostFromTemp(const Event& event){
        static const std::string autolaunch_checks[] = {
            "\\appdata\\local\\temp\\", "\\windows\\temp\\"
        };

        if (!IsProcessStart(event)){
            return false;
        }

        const std::string& image = GetRequiredField(event, "image");

        std::string path = NormalizePath(image);

        if (!path.ends_with("wscript.exe") && !path.ends_with("cscript.exe")){
            return false;
        }

        for (const auto& value : autolaunch_checks){
            if (CommandLineContains(event, value)){
                return true;
            }
        }

        return false;
    }

    bool LolbinDownload(const Event& event){
        if (!IsProcessStart(event)){
            return false;
        }

        const std::string& image = GetRequiredField(event, "image");

        std::string path = NormalizePath(image);
        
        if (!path.ends_with("certutil.exe") && !path.ends_with("bitsadmin.exe")){
            return false;
        }

        for (const auto& value : lolbin_checks){
            if (CommandLineContains(event, value)){
                return true;
            }
        }

        return false;
    }

    bool HiddenPowershell(const Event& event){
        if (!IsProcessStart(event)){
            return false;
        }

        const std::string& image = GetRequiredField(event, "image");

        std::string path = NormalizePath(image);

        if (!path.ends_with("powershell.exe") && !path.ends_with("pwsh.exe")){
            return false;
        }

        for (const auto& value : powershell_checks){
            if (CommandLineContains(event, value)){
                return true;
            }
        }

        return false;
    }

    bool AutostartWrite(const Event& event){
        if (IsFileCreate(event) || IsFileWrite(event)){
            if (FieldContains(event, "path", "\\start menu\\programs\\startup\\")){
                return true;
            }
        }
        
        if (IsFileMove(event)){
            if (FieldContains(event, "to", "\\start menu\\programs\\startup\\")){
                return true;
            }
        }

        return false;
    }

    bool RansomExtension(const Event& event){
        if (IsFileCreate(event) || IsFileWrite(event)){
            if (FieldEndsWith(event, "path", ".locked")){
                return true;
            }
        }

        if (IsFileMove(event)){
            if (FieldEndsWith(event, "to", ".locked")){
                return true;
            }
        }

        return false;
    }

    bool FieldContains(const Event& event, const std::string& key, const std::string& needle){
        const std::string* field_pointer = FindField(event, key);
        if (field_pointer == nullptr){
            return false;
        }
        std::string line = NormalizePath(*field_pointer);
        return line.find(needle) != std::string::npos;
    }

    bool FieldEndsWith(const Event& event, const std::string& key, const std::string& suffix){
        const std::string* field_pointer = FindField(event, key);
        if (field_pointer == nullptr) {
            return false;
        }
        std::string line = NormalizePath(*field_pointer);
        return line.ends_with(suffix);
    }

    bool IsFileCreate(const Event& event){
        return event.type() == "file_create";
    }

    bool IsFileMove(const Event& event){
        return event.type() == "file_move";
    }

    const Rule* AgentRules() {
        return kRules;
    }

    size_t AgentRuleCount() {
        return std::size(kRules);
    }

    // size_t FileChecksCount(){
    //     return std::size(file_checks);
    // }

}