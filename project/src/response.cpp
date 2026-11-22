#include <string>
#include <print>

#include "detection.h"
#include "entity_model.h"
#include "event.h"
#include "os_handle.h"
#include "response.h"
#include "text.h"
#include "fields.h"

namespace nano_edr {

// --- ResponsePlanner -------------------------------------------------------

bool IsSystemPath(const std::string& path){
    std::string normalized = ToLower(path);
    return normalized.starts_with("c:\\windows\\") || normalized.starts_with("c:\\program files");
}

ResponsePlanner::ResponsePlanner(const EntityModel* model,
                                 std::string quarantine_dir)
    : model_(model), quarantine_dir_(quarantine_dir) {}

ResponsePlan ResponsePlanner::Plan(const Detection& detection) const {
    ResponsePlan plan;
    const std::vector<std::string> tree = model_->GetProcessTree(detection.pid);
    for (const std::string& name : detection.actions) {
        if (name == "kill_process") {
            for (std::size_t i = tree.size(); i > 0; --i) {
                const std::string& pid = tree[i - 1];
                Action action;
                action.kind = ActionKind::kKillProcess;
                action.pid = pid;
                action.expected_start = model_->FindProcess(pid)->start;
                action.reason = detection.rule;
                if (pid != detection.pid) {
                    action.reason += ", потомок " + detection.pid;
                }
                plan.Add(std::move(action));
            }
        } else if (name == "quarantine_source") {
            const auto source = model_ ? model_->FindSpawnSource(detection.pid) : nullptr;
            if (source && !IsSystemPath(source->path)){
                std::string to = QuarantinePath(source->path, detection.pid);
                if (!to.empty()){
                    Action action;
                    action.kind = ActionKind::kQuarantineFile;
                    action.path = source->path;
                    action.to = std::move(to);
                    action.reason = detection.rule + ", источник запуска";
                    plan.Add(std::move(action));
                }
                
            }
        } else if (name == "quarantine_image"){
            const auto source = model_ ? model_->FindProcess(detection.pid) : nullptr;
            if (source && !IsSystemPath(source->image)){
                std::string to = QuarantinePath(source->image, detection.pid);
                if (!to.empty()){
                    Action action;
                    action.kind = ActionKind::kQuarantineFile;
                    action.path = source->image;
                    action.to = std::move(to);
                    action.reason = detection.rule + ", образ процесса";
                    plan.Add(std::move(action));
                }
                
            }
        } else if (name == "delete_dropped"){
            for (std::size_t i = tree.size(); i > 0; --i) {
                const std::string& pid = tree[i - 1];
                for (const std::string& path : model_->GetFilesCreatedBy(pid, true)){
                    if (!IsSystemPath(path)){
                        Action action;
                        action.kind = ActionKind::kDeleteFile;
                        action.path = path;
                        action.reason = detection.rule + ", принесён " + pid;
                        plan.Add(std::move(action));
                    }
                }
                
            }
        } else if (name == "delete_evidence"){
            for (const auto& value : detection.evidence){
                const std::string* path = nullptr;
                if (value->type() == "file_move"){
                    path = FindField(*value, "to");
                }
                else{
                    path = FindField(*value, "path");
                }
                if (path && !IsSystemPath(*path)){
                    Action action;
                    action.kind = ActionKind::kDeleteFile;
                    action.path = *path;
                    action.reason = detection.rule + ", файл из детекта";
                    plan.Add(std::move(action));
                }
            }
        }
        // незнакомое имя пропускается
    }
    return plan;
}

std::string ResponsePlanner::QuarantinePath(const std::string& path, const std::string& pid) const {
    if (quarantine_dir_.empty()){
        return "";
    }
    std::string q_path = path.substr(path.find_last_of('\\')+1);
    return std::format("{}\\{}-{}", quarantine_dir_, pid, q_path);
}


// --- Responder -------------------------------------------------------------

Responder::Responder(OsHandle* os, bool dry_run) : os_(os), dry_run_(dry_run) {}

void Responder::Execute(const ResponsePlan& plan) {
    for (const Action& action : plan.actions()) {
        ++stats_.planned;

        bool repeat = false;
        for (const std::string& target : done_) {
            if (target == action.target()) {
                repeat = true;
                break;
            }
        }
        if (repeat) {
            ++stats_.skipped_repeat;
            continue;
        }
        done_.push_back(action.target());

        if (dry_run_ || os_ == nullptr) {
            std::print("[PLAN] {}\n", ToString(action));
            continue;
        }
        
        switch(action.kind){
            case ActionKind::kKillProcess:
                Kill(action);
                break;
            case ActionKind::kQuarantineFile:
                MoveToQuarantine(action);
                break;
            case ActionKind::kDeleteFile:
                Delete(action);
                break;
        }
    }
}  // namespace nano_edr
}
