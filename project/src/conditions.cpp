#include "conditions.h"
#include "fields.h"

#include <text.h>

namespace nano_edr {
    bool FieldEquals::Matches(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        return ToLower(*value) == ToLower(value_);
    }

    bool FieldContains::Matches(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        return ToLower(*value).contains(ToLower(needle_));
    }

    bool FieldEndsWith::Matches(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        return ToLower(*value).ends_with(ToLower(suffix_));
    }

    bool FieldInList::Matches(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        for (auto& value_ : values_){
            std::string lower_value = ToLower(value_);
            if (lower_value == ToLower(*value)) return true;
        }
        return false;
    }

    bool EventTypeIs::Matches(const Event& event) const{
        for (size_t i=0;i<types_.size();i++){
            if (types_[i] == event.event_type()) return true;
        }
        return false;
    }

    void AllOf::Add(std::unique_ptr<ICondition> condition){
        parts_.push_back(std::move(condition));
    }
    

    bool AllOf::Matches(const Event& event) const{
        for (size_t i=0;i<size();i++){
            if (!(*parts_[i])(event)) return false;
        }   
        return true;
    }

    void AnyOf::Add(std::unique_ptr<ICondition> condition){
        parts_.push_back(std::move(condition));
    }

    bool AnyOf::Matches(const Event& event) const{
        for (size_t i=0;i<size();i++){
            if (!(*parts_[i])(event)) return true;
        }   
        return false;
    }
};