#include <conditions.h>
#include <fields.h>
#include <headers/text.h>

namespace nano_edr {
    bool FieldEquals::operator()(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        return ToLower(*value) == ToLower(value_);
    }

    bool FieldContains::operator()(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        return ToLower(*value).contains(ToLower(needle_));
    }

    bool FieldEndsWith::operator()(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        return ToLower(*value).ends_with(ToLower(suffix_));
    }

    bool FieldInList::operator()(const Event& event) const{
        const std::string* value = FindField(event, key_);
        if (value == nullptr) return false;
        for (auto& value_ : values_){
            std::string lower_value = ToLower(value_);
            if (lower_value == ToLower(*value)) return true;
        }
        return false;
    }

    Condition Condition::Equals(const std::string& key, const std::vector<std::string>& values){
        return Condition(Match::kEquals, key, values);
    }

    Condition Condition::Contains(const std::string& key, const std::vector<std::string>& values){
        return Condition(Match::kContains, key, values);
    }

    Condition Condition::EndsWith(const std::string& key, const std::vector<std::string>& values){
        return Condition(Match::kEndsWith, key, values);
    }

    bool Condition::operator()(const Event& event) const{
        switch(match()){
            case Match::kEquals:
                for (auto& value : values()){
                    if (FieldEquals(key(), value)(event)) return true;
                }
                return false;
            case Match::kContains:
                for (auto& value : values()){
                    if (FieldContains(key(), value)(event)) return true;
                }
                return false;
            case Match::kEndsWith:
                for (auto& value : values()){
                    if (FieldEndsWith(key(), value)(event)) return true;
                }
                return false;
            return false;
        }
    }
};