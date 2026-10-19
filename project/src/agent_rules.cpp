#include "headers/agent_rules.h"
#include "fields.h"
#include "rules.h"
#include <rule.h>
#include <algorithm>
#include <string>

namespace nano_edr {

    std::vector<MatchRule> BuildAgentRules(){

        std::vector<MatchRule> rules;

        MatchRule by_path("ransom_extension", Severity::kCritical);
        by_path.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
        by_path.AddCondition(Condition::EndsWith("path", {".locked"}));
        by_path.AddAction("quarantine_file");
        rules.push_back(by_path);

        MatchRule by_to("ransom_extension", Severity::kCritical);
        by_to.AddCondition(Condition::Equals("type", {"file_move"}));
        by_to.AddCondition(Condition::EndsWith("to", {".locked"}));
        by_to.AddAction("quarantine_file");
        rules.push_back(by_to);

        MatchRule script_host("script_host_from_temp", Severity::kHigh);
        script_host.AddCondition(Condition::Equals("type", {"process_start"}));
        script_host.AddCondition(Condition::EndsWith("image", {"wscript.exe", "cscript.exe"}));
        script_host.AddCondition(Condition::Contains("cmdline", {
            "\\appdata\\local\\temp\\", "/appdata/local/temp/",
            "\\windows\\temp\\", "/windows/temp/",
            "%temp%", "%tmp%"
        }));
        script_host.AddAction("kill_process");
        rules.push_back(script_host);

        MatchRule auto_write_path("autostart_write", Severity::kHigh);
        auto_write_path.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
        auto_write_path.AddCondition(Condition::Contains("path", {
            "\\start menu\\programs\\startup\\", "/start menu/programs/startup/"
        }));
        auto_write_path.AddAction("quarantine_file");
        rules.push_back(auto_write_path);

        MatchRule auto_write_to("autostart_write", Severity::kHigh);
        auto_write_to.AddCondition(Condition::Equals("type", {"file_move"}));
        auto_write_to.AddCondition(Condition::Contains("to", {
            "\\start menu\\programs\\startup\\", "/start menu/programs/startup/"
        }));
        auto_write_to.AddAction("quarantine_file");
        rules.push_back(auto_write_to);

        MatchRule lolbin_download("lolbin_download", Severity::kHigh);
        lolbin_download.AddCondition(Condition::Equals("type", {"process_start"}));
        lolbin_download.AddCondition(Condition::EndsWith("image", {"certutil.exe", "bitsadmin.exe"}));
        lolbin_download.AddCondition(Condition::Contains("cmdline", {
            "urlcache", "transfer", "http:", "https:"
        }));
        lolbin_download.AddAction("kill_process");
        rules.push_back(lolbin_download);

        MatchRule hidden_powershell("hidden_powershell", Severity::kMedium);
        hidden_powershell.AddCondition(Condition::Equals("type", {"process_start"}));
        hidden_powershell.AddCondition(Condition::EndsWith("image", {"powershell.exe", "pwsh.exe"}));
        hidden_powershell.AddCondition(Condition::Contains("cmdline", {
            "-w hidden", "-windowstyle hidden", "-enc", "-encodedcommand"
        }));
        hidden_powershell.AddAction("kill_process");
        rules.push_back(hidden_powershell);

        return rules;
    }

}