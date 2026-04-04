#include "med/NamedScans.hpp"
#include <algorithm>

namespace med {

NamedScans::NamedScans() : activeName_(DEFAULT_NAME) {
    data_[DEFAULT_NAME] = { {}, ScanType::Int32 };
}

void NamedScans::addNewScan(const std::string& name, ScanType type) {
    if (name.empty() || data_.count(name)) return;
    data_[name] = { {}, type };
}

bool NamedScans::remove(const std::string& name) {
    if (name == DEFAULT_NAME || !data_.count(name)) return false;
    
    data_.erase(name);
    if (activeName_ == name) {
        activeName_ = DEFAULT_NAME;
    }
    return true;
}

void NamedScans::setActiveName(const std::string& name) {
    if (data_.count(name)) {
        activeName_ = name;
    }
}

std::vector<ScanResult>& NamedScans::getActiveResults() {
    return data_[activeName_].results;
}

void NamedScans::setActiveResults(const std::vector<ScanResult>& results) {
    data_[activeName_].results = results;
}

ScanType NamedScans::getActiveType() const {
    auto it = data_.find(activeName_);
    if (it != data_.end()) {
        return it->second.type;
    }
    return ScanType::Int32;
}

void NamedScans::setActiveType(ScanType type) {
    data_[activeName_].type = type;
}

std::vector<std::string> NamedScans::getNames() const {
    std::vector<std::string> names;
    for (const auto& pair : data_) {
        names.push_back(pair.first);
    }
    // Ensure Default is first
    auto it = std::find(names.begin(), names.end(), DEFAULT_NAME);
    if (it != names.end() && it != names.begin()) {
        std::swap(*it, *names.begin());
    }
    return names;
}

} // namespace med
