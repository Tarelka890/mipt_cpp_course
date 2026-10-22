#include "agent_rules.h"
#include "fields.h"
#include "rules.h"
#include <rule.h>
#include <algorithm>
#include <string>
#include <conditions.h>

namespace nano_edr {

    RuleEngine BuildAgentRules(){

        RuleEngine rules;

        // ransom_extension header
        auto rule = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));
        rule->AddAction("quarantine_file");

        // ransom_extension path checks
        auto paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
        paths->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
        rule->AddCondition(std::move(paths));
        rules.AddRule(std::move(rule));

        //script_host header
        rule = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        rule->AddAction("kill_process");

        //script_host cmdline checks
        paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
        rule->AddCondition(std::move(paths));

        //script_host image checks
        auto image_paths = std::make_unique<AnyOf>();
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        rule->AddCondition(std::move(image_paths));
        rules.AddRule(std::move(rule));

        //autostart_write header
        rule = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));
        rule->AddAction("quarantine_file");

        //autostart_write path checks
        paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldContains>("path", "\\start munu\\programs\\startup\\"));
        paths->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
        rule->AddCondition(std::move(paths));
        rules.AddRule(std::move(rule));

        //lolbin_download header
        rule = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        rule->AddAction("kill_process");

        //lolbin_download image checks
        image_paths = std::make_unique<AnyOf>();
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
        rule->AddCondition(std::move(image_paths));

        //lolbin_download cmdline checks
        paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "http:"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "https:"));
        rule->AddCondition(std::move(paths));
        rules.AddRule(std::move(rule));

        //hidden_powershell header
        rule = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        rule->AddAction("kill_process");

        //hidden_powershell image checks
        image_paths = std::make_unique<AnyOf>();
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
        rule->AddCondition(std::move(image_paths));

        //hidden_powershell cmdline checks
        paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "http:"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "https:"));
        rule->AddCondition(std::move(paths));
        rules.AddRule(std::move(rule));

        //script_host_beacon first rule header
        auto temp_rule = std::make_unique<AllOf>();
        temp_rule->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

        //script_host_beacon first rule image checks
        image_paths = std::make_unique<AnyOf>();
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        image_paths->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        temp_rule->Add(std::move(image_paths));

        //script_host_beacon cmdline checks
        paths = std::make_unique<AnyOf>();
        paths->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
        paths->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
        temp_rule->Add(std::move(paths));

        //script_host beacon double-rule
        auto beacon = std::make_unique<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);
        beacon->SetFirst(std::move(temp_rule));
        beacon->SetSecond(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kNetConnect}));
        beacon->AddAction("kill_process");
        rules.AddRule(std::move(beacon));

        return rules;
    }

}