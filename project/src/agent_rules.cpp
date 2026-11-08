#include "agent_rules.h"
#include "fields.h"
#include "rules.h"
#include "rule.h"
#include "conditions.h"
#include "make_rule.h"
#include "combinators.h"

#include <algorithm>
#include <string>


namespace nano_edr {

    RuleEngine BuildAgentRules(){

        RuleEngine rules;

        // ransom_extension header
        auto ransom_extension_rule = MakeRule<MatchRule>("ransom_extension", Severity::kCritical);
        ransom_extension_rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));
        ransom_extension_rule->AddAction("quarantine_file");

        // ransom_extension path checks
        std::unique_ptr<ICondition> ransom_extension_paths = MakeCondition(AnyOfStatic(
            FieldEndsWith("path", ".locked"),
            FieldEndsWith("to", ".locked")
        ));
        ransom_extension_rule->AddCondition(std::move(ransom_extension_paths));
        rules.AddRule(std::move(ransom_extension_rule));

        //script_host header
        auto script_host_rule = MakeRule<MatchRule>("script_host_from_temp", Severity::kHigh);
        script_host_rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        script_host_rule->AddAction("kill_process");

        //script_host cmdline checks
        std::unique_ptr<ICondition> script_host_cmdline = MakeCondition(AnyOfStatic(
            FieldContains("cmdline", "\\appdata\\local\\temp\\"),
            FieldContains("cmdline", "\\windows\\temp\\"),
            FieldContains("cmdline", "/appdata/local/temp/"),
            FieldContains("cmdline", "/windows/temp/"),
            FieldContains("cmdline", "%temp%"),
            FieldContains("cmdline", "%tmp%")
        ));
        script_host_rule->AddCondition(std::move(script_host_cmdline));

        //script_host image checks
        std::unique_ptr<ICondition> script_host_image = MakeCondition(AnyOfStatic(
            FieldEndsWith("image", "wscript.exe"),
            FieldEndsWith("image", "csript.exe")
        ));
        script_host_rule->AddCondition(std::move(script_host_image));
        rules.AddRule(std::move(script_host_rule));

        //autostart_write header
        auto autostart_write_rule = MakeRule<MatchRule>("autostart_write", Severity::kHigh);
        autostart_write_rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));
        autostart_write_rule->AddAction("quarantine_file");

        //autostart_write path checks
        std::unique_ptr<ICondition> autostart_write_paths = MakeCondition(AnyOfStatic(
            FieldContains("path", "\\start menu\\programs\\startup\\"),
            FieldContains("path", "/start menu/programs/startup/")
        ));
        autostart_write_rule->AddCondition(std::move(autostart_write_paths));
        rules.AddRule(std::move(autostart_write_rule));

        //lolbin_download header
        auto lolbin_download_rule = MakeRule<MatchRule>("lolbin_download", Severity::kHigh);
        lolbin_download_rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        lolbin_download_rule->AddAction("kill_process");

        //lolbin_download image checks
        std::unique_ptr<ICondition> lolbin_download_image = MakeCondition(AnyOfStatic(
            FieldEndsWith("image", "certutil.exe"),
            FieldEndsWith("image", "bitsadmin.exe")
        ));
        lolbin_download_rule->AddCondition(std::move(lolbin_download_image));

        //lolbin_download cmdline checks
        std::unique_ptr<ICondition> lolbin_download_cmdline = MakeCondition(AnyOfStatic(
            FieldContains("cmdline", "urlcache"),
            FieldContains("cmdline", "transfer"),
            FieldContains("cmdline", "http:"),
            FieldContains("cmdline", "https:")
        ));
        lolbin_download_rule->AddCondition(std::move(lolbin_download_cmdline));
        rules.AddRule(std::move(lolbin_download_rule));

        //hidden_powershell header
        auto hidden_powershell_rule = MakeRule<MatchRule>("hidden_powershell", Severity::kMedium);
        hidden_powershell_rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        hidden_powershell_rule->AddAction("kill_process");

        //hidden_powershell image checks
        std::unique_ptr<ICondition> hidden_powershell_image = MakeCondition(AnyOfStatic(
            FieldEndsWith("image", "powershell.exe"),
            FieldEndsWith("image", "pwsh.exe")
        ));
        hidden_powershell_rule->AddCondition(std::move(hidden_powershell_image));

        //hidden_powershell cmdline checks
        std::unique_ptr<ICondition> hidden_powershell_cmdline = MakeCondition(AnyOfStatic(
            FieldContains("cmdline", "-w hidden"),
            FieldContains("cmdline", "transfer"),
            FieldContains("cmdline", "http:"),
            FieldContains("cmdline", "https:")
        ));
        hidden_powershell_rule->AddCondition(std::move(hidden_powershell_cmdline));
        rules.AddRule(std::move(hidden_powershell_rule));

        //script_host_beacon first rule image checks
        std::unique_ptr<ICondition> script_host_beacon_image = MakeCondition(AnyOfStatic(
            FieldContains("image", "wscript.exe"),
            FieldContains("image", "cscript.exe")
        ));

        //script_host_beacon cmdline checks
        std::unique_ptr<ICondition> script_host_beacon_cmdline = MakeCondition(AnyOfStatic(
            FieldContains("cmdline", "\\appdata\\local\\temp\\"),
            FieldContains("cmdline", "\\windows\\temp\\"),
            FieldContains("cmdline", "/appdata/local/temp/"),
            FieldContains("cmdline", "/windows/temp/"),
            FieldContains("cmdline", "%temp%"),
            FieldContains("cmdline", "%tmp%")
        ));

        //script_host_beacon first rule header
        std::unique_ptr<ICondition> script_host_beacon_rule1 = MakeCondition(AllOfStatic(
            EventTypeIs(std::vector<EventType>{EventType::kProcessStart}),
            AnyOfStatic(
                FieldContains("cmdline", "\\appdata\\local\\temp\\"),
                FieldContains("cmdline", "\\windows\\temp\\"),
                FieldContains("cmdline", "/appdata/local/temp/"),
                FieldContains("cmdline", "/windows/temp/"),
                FieldContains("cmdline", "%temp%"),
                FieldContains("cmdline", "%tmp%")
            ),
            AnyOfStatic(
                FieldContains("image", "wscript.exe"),
                FieldContains("image", "cscript.exe")
            )
        ));

        //script_host beacon double-rule
        auto script_host_beacon_drule = MakeRule<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);
        script_host_beacon_drule->SetFirst(std::move(script_host_beacon_rule1));
        script_host_beacon_drule->SetSecond(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kNetConnect}));
        script_host_beacon_drule->AddAction("kill_process");
        rules.AddRule(std::move(script_host_beacon_drule));

        // mass_write header
        auto mass_write_rule = MakeRule<ThresholdRule<128>>( "ransomware_mass_write", Severity::kCritical, 20, 10000, "pid");  
        mass_write_rule->SetCondition(MakeCondition(AllOfStatic(
            EventTypeIs({EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}),
            AnyOfStatic(
                FieldContains("path", "\\Documents\\"),
                FieldContains("path", "\\Desktop\\"),
                FieldContains("to", "\\Documents\\"),
                FieldContains("to", "\\Desktop\\")
            )
        )));
        mass_write_rule->AddAction("kill_process");
        mass_write_rule->AddAction("quarantine_file");
        rules.AddRule(std::move(mass_write_rule));

        return rules;
    }

}