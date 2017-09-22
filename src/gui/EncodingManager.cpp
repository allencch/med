#include <iostream>

#include <QAction>
#include <QMenu>

#include "gui/med-qt.hpp"
#include "gui/EncodingManager.hpp"
#include "med/MedTypes.hpp"

using namespace std;

EncodingManager::EncodingManager(MainUi* mainUi) {
  this->mainUi = mainUi;
  mainWindow = mainUi->mainWindow;
  setupSubmenu();
  setupSignals();
}

void EncodingManager::setupSubmenu() {
  QActionGroup* menuActionGroup = new QActionGroup(mainWindow);
  menuActionGroup->setExclusive(true);

  menuActionGroup->addAction(mainWindow->findChild<QAction*>("actionDefaultEncoding"));
  menuActionGroup->addAction(mainWindow->findChild<QAction*>("actionBig5Encoding"));
}

void EncodingManager::setupSignals() {
  QObject::connect(mainWindow->findChild<QAction*>("actionDefaultEncoding"), // Requires header QAction
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(onDefaultEncodingTriggered(bool)));
  QObject::connect(mainWindow->findChild<QAction*>("actionBig5Encoding"),
                   SIGNAL(triggered(bool)),
                   this,
                   SLOT(onBig5EncodingTriggered(bool)));
}

void EncodingManager::onDefaultEncodingTriggered(bool value) {
  if (value) {
    encodingType = EncodingType::Default;
  }
};

void EncodingManager::onBig5EncodingTriggered(bool value) {
  if (value) {
    encodingType = EncodingType::Big5;
  }
};

EncodingType EncodingManager::getEncodingType() {
  return this->encodingType;
}
