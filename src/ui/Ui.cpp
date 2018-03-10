#include <QtUiTools>
#include <QtDebug>

#include "ui/Ui.hpp"

MedUi::MedUi(QApplication* app) {
  this->app = app;
  loadUiFiles();
  setupUi();
  // TODO: other action here
}

void MedUi::loadUiFiles() {
  QUiLoader loader;
  QFile file("./main-qt.ui");
  file.open(QFile::ReadOnly);
  mainWindow = loader.load(&file);
  file.close();

  // TODO: other setup here
}

void MedUi::setupUi() {
  mainWindow->show();
}
