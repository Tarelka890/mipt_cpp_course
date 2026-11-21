#include <stdexcept>
#include <string>
#include <iostream>

#include "alert_sink.h"
#include "dedup_sink.h"
#include "detection.h"

namespace nano_edr {

// --- ConsoleSink -----------------------------------------------------------

void ConsoleSink::operator()(const Detection& detection) const {
    std::cout << ToString(detection) << '\n';
}

// --- FileSink --------------------------------------------------------------

FileSink::FileSink(const std::string& path) : out_(path) {
    if (!out_.is_open()) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection) {
    out_ << ToString(detection) << '\n';
}

// --- CountingSink ----------------------------------------------------------

void CountingSink::operator()(const Detection& detection) const {
    counter_->total++;
    counter_->seen.push_back(detection);
}

// --- FanOutSink ------------------------------------------------------------

void FanOutSink::Add(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

void FanOutSink::operator()(const Detection& detection) const {
    for (const auto& sink : sinks_){
        sink(detection);
    }
}

// --- DedupSink -------------------------------------------------------------

DedupSink::DedupSink(AlertSink inner, uint64_t period_ms, std::size_t* suppressed) : inner_(std::move(inner)), period_ms_(period_ms), suppressed_(suppressed) {}

void DedupSink::CheckRunSink(const Detection& detection){
    if (inner_){
        inner_(detection);
    }
}

void DedupSink::UpdateSuppressed(){
    if (suppressed_){
        (*suppressed_)++;    
    }
}

void DedupSink::operator()(const Detection& detection) {
    if (period_ms_ == 0){
        CheckRunSink(detection);
        return;
    }

    for (auto& value : window_){
        if (value.rule == detection.rule && value.pid == detection.pid){
            if (detection.ts.ms - value.first_ms <= period_ms_){
                UpdateSuppressed();
                return;
            }
            CheckRunSink(detection);
            value.first_ms = detection.ts.ms;
            return;
        }
    }

    window_.push_back(Entry(detection.rule, detection.pid, detection.ts.ms));
    CheckRunSink(detection);
}

}  // namespace nano_edr
