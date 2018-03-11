#include <iostream>
#include <string>

#include <QAction>
#include <QMenu>

#include "ui/Ui.hpp"
#include "ui/EncodingManager.hpp"
#include "med/MedTypes.hpp"
#include "med/Coder.hpp"

using namespace std;

EncodingManager::EncodingManager(MedUi* mainUi) {
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

string EncodingManager::convertToUtf8(string text) {
  switch (encodingType) {
  case EncodingType::Default:
    return text;
  case EncodingType::Big5:
    return convertBig5ToUtf8(text);
  }
  return text;
}

string EncodingManager::encode(string text) {
  switch (encodingType) {
  case EncodingType::Default:
    return text;
  case EncodingType::Big5:
    return convertFromUtf8(text, "big5");
  }
  return text;
}
