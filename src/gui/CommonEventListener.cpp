#include "gui/CommonEventListener.hpp"

/*************\
 ProcessDialog
\*************/

ProcessDialogEventListener::ProcessDialogEventListener(MainUi* mainUi) {
  this->mainUi = mainUi;
}
bool ProcessDialogEventListener::eventFilter(QObject* obj, QEvent* ev) {
  if(obj == mainUi->processTreeWidget && ev->type() == QEvent::KeyRelease) {
    if(static_cast<QKeyEvent*>(ev)->key() == Qt::Key_Return) { //Use Return instead of Enter
      mainUi->onProcessItemDblClicked(mainUi->processTreeWidget->currentItem(), 0); //Just use the first column
    }
  }
  return false;
}


/*************\
 MainWindow
\*************/

MainWindowEventListener::MainWindowEventListener(MainUi* mainUi) {
  this->mainUi = mainUi;
}

bool MainWindowEventListener::eventFilter(QObject* obj, QEvent* ev) {
  if (ev->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);

    if (keyEvent->key() == Qt::Key_Escape) {
      if (mainUi->getScanState() == UiState::Editing) {
        tryUnlock(mainUi->scanUpdateMutex);
        mainUi->setScanState(UiState::Idle);
      }
      if (mainUi->getStoreState() == UiState::Editing) {
        tryUnlock(mainUi->storeUpdateMutex);
        mainUi->setStoreState(UiState::Idle);
      }
    }

    if (keyEvent->key() == Qt::Key_F2) {
      QWidget* focused = mainUi->mainWindow->focusWidget()->parentWidget()->parentWidget(); // When the TreeView item value is selected, the TreeView will be the grandparent of the editing
      if (focused == mainUi->scanTreeView) {
        QModelIndex index = mainUi->scanTreeView->currentIndex();
        if (index.column() == SCAN_COL_VALUE && mainUi->getScanState() == UiState::Idle) {
          mainUi->scanUpdateMutex.lock();
          mainUi->setScanState(UiState::Editing);
        }
      }
      else if (focused == mainUi->storeTreeView) {
        QModelIndex index = mainUi->storeTreeView->currentIndex();
        if (index.column() == STORE_COL_VALUE && mainUi->getStoreState() == UiState::Idle) {
          mainUi->storeUpdateMutex.lock();
          mainUi->setStoreState(UiState::Editing);
        }
      }
    }
  }
  return false;
}
