#ifndef NAMED_SCANS_CONTROLLER_HPP
#define NAMED_SCANS_CONTROLLER_HPP

#include <QObject>
#include <QComboBox>
#include "mem/MemEd.hpp"
#include "mem/NamedScans.hpp"

class MedUi; // Forward declaration

class NamedScansController : public QObject {
  Q_OBJECT
public:
  explicit NamedScansController(MedUi *mainUi);
private slots:
  void onAddClicked();
  void onDeleteClicked();
  void onComboBoxChanged(int index);
private:
  void selectByName(string name);
  void updateScanTree();
  MedUi *mainUi;
  QWidget *mainWindow;
  MemEd *med;
  NamedScans *namedScans;

  QComboBox *comboBox;
};
#endif




