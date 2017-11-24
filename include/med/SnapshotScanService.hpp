#include "med/SnapshotScan.hpp"

class SnapshotScanService {
public:
  SnapshotScanService();
  virtual ~SnapshotScanService();

  virtual bool compareScan(SnapshotScan* scan, long pid, const ScanParser::OpType& opType, const ScanType& scanType);

  virtual void updateScannedValue(SnapshotScan* scan, long pid, const ScanType& scanType);
};
