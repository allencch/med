#include <iostream>
#include "ui/Ui.hpp"
#include "ui/NamedScansController.hpp"

using namespace std;

NamedScansController::NamedScansController(MedUi *mainUi) {
  this->mainUi = mainUi;

  auto mainWindow = mainUi->mainWindow;

  QObject::connect(mainWindow->findChild<QWidget*>("namedScan_add"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onAddClicked()));
}

void NamedScansController::onAddClicked() {
  cout << "foobar" << endl;
}
