#include <iostream>
#include <string>
#include "med/MedCommon.hpp"
#include "ui/ScanTreeEventListener.hpp"

ScanTreeEventListener::ScanTreeEventListener(QTreeView* treeView, MedUi* mainUi) {
  this->mainUi = mainUi;
  this->treeView = treeView;
}

bool ScanTreeEventListener::eventFilter(QObject*, QEvent* ev) {
  if (ev->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);

    if (keyEvent->key() == Qt::Key_Escape) {
      if (mainUi->getScanState() == UiState::Editing) {
        tryUnlock(*mainUi->scanUpdateMutex);
        mainUi->setScanState(UiState::Idle);
      }
    }

    if (keyEvent->key() == Qt::Key_F2) {
      QWidget* focused = mainUi->mainWindow->focusWidget();
      if (focused == treeView) {
        QModelIndex index = treeView->currentIndex();
        if (index.column() == SCAN_COL_VALUE && mainUi->getScanState() == UiState::Idle) {
          mainUi->scanUpdateMutex->lock();
          mainUi->setScanState(UiState::Editing);
        }
      }
    }
  }
  return false;
}
