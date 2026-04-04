#ifndef MED_NAMED_SCANS_HPP
#define MED_NAMED_SCANS_HPP

#include <string>
#include <vector>
#include <map>
#include "med/MemScanner.hpp"

namespace med {

class NamedScans {
public:
    static constexpr const char* DEFAULT_NAME = "Default";

    NamedScans();
    
    void addNewScan(const std::string& name, ScanType type = ScanType::UInt32);
    bool remove(const std::string& name);
    
    void setActiveName(const std::string& name);
    std::string getActiveName() const { return activeName_; }
    
    std::vector<ScanResult>& getActiveResults();
    void setActiveResults(const std::vector<ScanResult>& results);
    
    ScanType getActiveType() const;
    void setActiveType(ScanType type);
    
    std::vector<std::string> getNames() const;

private:
    struct ScanData {
        std::vector<ScanResult> results;
        ScanType type;
    };
    
    std::map<std::string, ScanData> data_;
    std::string activeName_;
};

} // namespace med

#endif
