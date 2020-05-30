#ifndef NAMED_SCANS_CONTROLLER_HPP
#define NAMED_SCANS_CONTROLLER_HPP

#include <QObject>

class MedUi; // Forward declaration

class NamedScansController : public QObject {
  Q_OBJECT
public:
  explicit NamedScansController(MedUi *mainUi);
private slots:
  void onAddClicked();
private:
  MedUi *mainUi;
};
#endif




