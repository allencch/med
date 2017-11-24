#include "med/SnapshotScanService.hpp"

SnapshotScanService::SnapshotScanService() {}

SnapshotScanService::~SnapshotScanService() {}

bool SnapshotScanService::compareScan(SnapshotScan* scan, long pid, const ScanParser::OpType& opType, const ScanType& scanType) {
  return scan->compare(pid, opType, scanType);
}

void SnapshotScanService::updateScannedValue(SnapshotScan* scan, long pid, const ScanType& scanType) {
  scan->updateScannedValue(pid, scanType);
}
