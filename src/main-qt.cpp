/**
 * 2016-09-19
 * Try to use QThread, but it involves signal and slot. So, I choose C++11 thread instead, for simpler implementation
 * TODO: Need to redesign the "read" and "write" when item change. Not every time it is "write".
 * Possible solution, create property to the object.
 */
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>
#include <mutex>
#include <QApplication>
#include <QtUiTools>
#include <QtDebug>

#include "med-qt.hpp"
#include "TreeItem.hpp"
#include "TreeModel.hpp"
#include "StoreTreeModel.hpp"
#include "ComboBoxDelegate.hpp"
#include "med.hpp"

using namespace std;

class MainUi : public QObject {
  Q_OBJECT

public:
  MainUi() {
    loadUiFiles();
  }
  Med med;
  std::thread* refreshThread;
  std::mutex scanUpdateMutex;
  std::mutex addressUpdateMutex;

  static void refresh(MainUi* mainUi) {
    while(1) {
      //mainUi->refreshScanTreeWidget();
      //mainUi->refreshAddressTreeWidget();
      std::this_thread::sleep_for(chrono::milliseconds(800));
    }
  }

  void refreshScanTreeWidget() {
    QTreeWidget* scanTreeWidget = this->mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    QTreeWidgetItemIterator it(scanTreeWidget);
    scanUpdateMutex.lock();
    while(*it) {
      try {
        int index = scanTreeWidget->indexOfTopLevelItem(*it);
        string value = med.getScanValueByIndex(index);
        (*it)->setText(2, value.c_str());
      } catch(string e) {
        (*it)->setText(2, "Error memory");
      }
      it++;
    }
    scanUpdateMutex.unlock();
  }

  void refreshAddressTreeWidget() {
    QTreeWidget* addressTreeWidget = this->mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItemIterator it(addressTreeWidget);
    addressUpdateMutex.lock();
    while(*it) {
      try {
        int index = addressTreeWidget->indexOfTopLevelItem(*it);
        string value = med.getAddressValueByIndex(index);
        (*it)->setText(3, value.c_str());
      } catch(string e) {
        (*it)->setText(3, "Error memory");
      }
      it++;
    }
    addressUpdateMutex.unlock();
  }

private slots:
  void onProcessClicked() {
    med.listProcesses();

    processDialog->show();

    //Get the tree widget
    QTreeWidget* procTreeWidget = chooseProc->findChild<QTreeWidget*>("procTreeWidget");

    procTreeWidget->clear(); //Remove all items

    //Add all the process into the tree widget
    for(int i=med.processes.size()-1;i>=0;i--) {
      QTreeWidgetItem* item = new QTreeWidgetItem(procTreeWidget);
      item->setText(0, med.processes[i].pid.c_str());
      item->setText(1, med.processes[i].cmdline.c_str());
    }
  }

  void onProcItemDblClicked(QTreeWidgetItem* item, int column) {
    int index = item->treeWidget()->indexOfTopLevelItem(item); //Get the current row index
    med.selectedProcess = med.processes[med.processes.size() -1 - index];

    //Make changes to the selectedProc and hide the window
    QLineEdit* line = this->mainWindow->findChild<QLineEdit*>("selectedProc");
    line->setText(QString::fromLatin1((med.selectedProcess.pid + " " + med.selectedProcess.cmdline).c_str())); //Do not use fromStdString(), it will append with some unknown characters

    processDialog->hide();
  }

  void onScanClicked() {
    scanUpdateMutex.lock();
    //QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    //scanTreeWidget->clear();
    //scanModel->clearAll();

    //Get scanned type
    string scanType = mainWindow->findChild<QComboBox*>("scanType")->currentText().toStdString();

    string scanValue = mainWindow->findChild<QLineEdit*>("scanEntry")->text().toStdString();

    if(med.selectedProcess.pid == "") {
      cerr << "No process seelcted " <<endl;
      return;
    }
    try {
      med.scanEqual(scanValue, scanType);
    } catch(string e) {
      cerr << "scan: "<<e<<endl;
    }

    if(med.scanAddresses.size() <= 800) {
      //addressToScanTreeWidget(med, scanType, scanTreeWidget);
      //addressToScanModel(med, scanType, scanModel);
      scanModel->addScan(scanType);
    }

    updateNumberOfAddresses(mainWindow);
    scanUpdateMutex.unlock();
  }

  void onFilterClicked() {
    scanUpdateMutex.lock();
    //QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    //scanTreeWidget->clear();

    //scanModel->clearAll();

    //Get scanned type
    string scanType = mainWindow->findChild<QComboBox*>("scanType")->currentText().toStdString();

    string scanValue = mainWindow->findChild<QLineEdit*>("scanEntry")->text().toStdString();

    med.scanFilter(scanValue, scanType);

    if(med.scanAddresses.size() <= 800) {
      scanModel->addScan(scanType);
    }

    updateNumberOfAddresses(mainWindow);
    scanUpdateMutex.unlock();
  }

  void onClearClicked() {
    //med.scanAddresses.clear();
    scanModel->clearAll();
    mainWindow->findChild<QStatusBar*>("statusbar")->showMessage("Scan cleared");
  }

  void onScanAddClicked() {
    QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    QTreeWidgetItem* item = scanTreeWidget->currentItem();
    int index = scanTreeWidget->indexOfTopLevelItem(item);
    if (index == -1)
      return;

    //TODO: Write this to a function
    MedAddress medAddress;
    medAddress.address = med.scanAddresses[index].address;
    medAddress.scanType = med.scanAddresses[index].scanType;
    med.addresses.push_back(medAddress);

    //Add to AddressTreeWidget
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItem* itemToAdd = new QTreeWidgetItem(addressTreeWidget);
    itemToAdd->setText(0, "Your description");
    itemToAdd->setText(1, intToHex(medAddress.address).c_str());
    itemToAdd->setText(3, med.getAddressValueByIndex(med.addresses.size()-1).c_str());
    itemToAdd->setFlags(itemToAdd->flags() | Qt::ItemIsEditable);

    //Add combo box
    QComboBox* combo = createTypeComboBox(addressTreeWidget, medAddress.getScanType());
    combo->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
    addressTreeWidget->setItemWidget(itemToAdd, 2, combo);

    //Add checkbox
    QCheckBox* checkbox = new QCheckBox(addressTreeWidget);
    checkbox->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
    checkbox->setCheckState(Qt::Unchecked);
    addressTreeWidget->setItemWidget(itemToAdd, 4, checkbox);

    //Add signal
    QObject::connect(combo,
                     SIGNAL(currentTextChanged(QString)),
                     this,
                     SLOT(onAddressTypeChanged(QString)));
  }

  //TODO: Complete this one
  void onScanAddAllClicked() {
  }

  void onScanItemChanged(QTreeWidgetItem* item, int column) {
    switch(column) {
    case 1: //Not available, because the 2nd column is combobox
      break;
    case 2:
      editScanValue(item, column);
      break;
    }
  }

  void onScanItemDblClicked(QTreeWidgetItem* item, int column) {
    if(column == 0) {
      //Change to disable
      item->setFlags(item->flags() & (~Qt::ItemIsEditable)); //Set editable to 0. Do not use XOR
    }
    else {
      item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
  }

  void onScanTypeChanged(QString text) {
    QComboBox* combo = qobject_cast<QComboBox*>(sender());

    QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");

    QTreeWidgetItem* item = scanTreeWidget->topLevelItem(combo->property("tree-row").toInt());
    scanTreeWidget->setCurrentItem(item);

    editScanType(item, text.toStdString());
  }

  void onAddressTypeChanged(QString text) {
    QComboBox* combo = qobject_cast<QComboBox*>(sender());

    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");

    QTreeWidgetItem* item = addressTreeWidget->topLevelItem(combo->property("tree-row").toInt());
    addressTreeWidget->setCurrentItem(item);

    editAddressType(item, text.toStdString());
  }

  void onAddressItemChanged(QTreeWidgetItem* item, int column) {
    switch(column) {
    case ADDRESS_COL_DESCRIPTION:
      editAddressDescription(item, column);
      break;
    case ADDRESS_COL_ADDRESS:
      editAddressAddress(item, column);
      break;
    case ADDRESS_COL_TYPE: //Not available, because the 2nd column is combobox
      break;
    case ADDRESS_COL_VALUE:
      editAddressValue(item, column);
      break;
    }
  }

  void onAddressLockChanged(int state) {
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItem* item = addressTreeWidget->topLevelItem(checkbox->property("tree-row").toInt());
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    med.addresses[index].lock = state == Qt::Checked ? true : false;
  }

  void onAddressNewClicked() {
    MedAddress medAddress;
    medAddress.description = "Your description";
    medAddress.address = 0;
    medAddress.setScanType("int16");
    medAddress.lock = false;
    med.addresses.push_back(medAddress);

     //Add to AddressTreeWidget
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItem* itemToAdd = new QTreeWidgetItem(addressTreeWidget);
    itemToAdd->setText(0, medAddress.description.c_str());
    itemToAdd->setText(1, "0");
    itemToAdd->setFlags(itemToAdd->flags() | Qt::ItemIsEditable);

    //Add combo box
    QComboBox* combo = createTypeComboBox(addressTreeWidget, medAddress.getScanType());
    combo->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
    addressTreeWidget->setItemWidget(itemToAdd, 2, combo);

    QCheckBox* checkbox = new QCheckBox(addressTreeWidget);
    checkbox->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
    checkbox->setCheckState(Qt::Unchecked);
    addressTreeWidget->setItemWidget(itemToAdd, 4, checkbox);

    //Add signal
    QObject::connect(combo,
                     SIGNAL(currentTextChanged(QString)),
                     this,
                     SLOT(onAddressTypeChanged(QString)));
    QObject::connect(checkbox,
                     SIGNAL(stateChanged(int)),
                     this,
                     SLOT(onAddressLockChanged(int)));
  }

  void onAddressDeleteClicked() {
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItem* item = addressTreeWidget->currentItem();
    delete item;
  }

  void onAddressShiftClicked() {
    //Get the from and to
    long shiftFrom, shiftTo, difference;
    try {
      shiftFrom = hexToInt(mainWindow->findChild<QLineEdit*>("shiftFrom")->text().toStdString());
      shiftTo = hexToInt(mainWindow->findChild<QLineEdit*>("shiftTo")->text().toStdString());
      difference = shiftTo - shiftFrom;
    } catch(string e) {
      throw e;
    }

    //Get PID
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }

    QTreeWidget* addressTree = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    for(int i=0;i<med.addresses.size();i++) {
      long address = med.addresses[i].address;
      address += difference;
      med.addresses[i].address = address;
      QTreeWidgetItem* item = addressTree->topLevelItem(i);
      item->setText(1, intToHex(address).c_str());
    }
  }

  void onSaveAsTriggered() {
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }
    QString filename = QFileDialog::getSaveFileName(mainWindow,
                                                    QString("Save JSON"),
                                                    "./",
                                                    QString("Save JSON (*.json)"));

    med.saveFile(filename.toStdString().c_str());
  }

  void onOpenTriggered() {
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }
    QString filename = QFileDialog::getOpenFileName(mainWindow,
                                                    QString("Open JSON"),
                                                    "./",
                                                    QString("Open JSON (*.json)"));
    med.openFile(filename.toStdString().c_str());

    //Load the data to the address panel
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    addressTreeWidget->clear();

    for(int i=0;i<med.addresses.size();i++) {
      QTreeWidgetItem* itemToAdd = new QTreeWidgetItem(addressTreeWidget);
      itemToAdd->setText(ADDRESS_COL_DESCRIPTION, med.addresses[i].description.c_str());
      itemToAdd->setText(ADDRESS_COL_ADDRESS, intToHex(med.addresses[i].address).c_str());
      itemToAdd->setText(ADDRESS_COL_VALUE, med.getAddressValueByIndex(i).c_str());
      itemToAdd->setFlags(itemToAdd->flags() | Qt::ItemIsEditable);

      QComboBox* combo = createTypeComboBox(addressTreeWidget, med.addresses[i].getScanType());
      combo->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
      addressTreeWidget->setItemWidget(itemToAdd, ADDRESS_COL_TYPE, combo);

      QCheckBox* checkbox = new QCheckBox(addressTreeWidget);
      checkbox->setCheckState(med.addresses[i].lock ? Qt::Checked : Qt::Unchecked);
      addressTreeWidget->setItemWidget(itemToAdd, ADDRESS_COL_LOCK , checkbox);

      //Add signal
      QObject::connect(combo,
                       SIGNAL(currentTextChanged(QString)),
                       this,
                       SLOT(onAddressTypeChanged(QString)));
      QObject::connect(checkbox,
                       SIGNAL(stateChanged(int)),
                       this,
                       SLOT(onAddressLockChanged(int)));
    }
  }

  void onScanTreeViewClicked(const QModelIndex &index) {
    if(index.column() == 1) {
      mainWindow->findChild<QTreeView*>("scanTreeView")->edit(index); //Trigger edit by 1 click
    }
  }

  void onScanTreeViewDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>()) {
    qDebug() << topLeft << bottomRight << roles;
  }

private:
  QWidget* mainWindow;
  QWidget* chooseProc;
  QDialog* processDialog;

  TreeModel* scanModel;
  StoreTreeModel * storeModel; //Previously is the address model, but named as "store" is better

  void loadUiFiles() {
    QUiLoader loader;

    QFile file("./main-qt.ui");
    file.open(QFile::ReadOnly);
    mainWindow = loader.load(&file);
    file.close();

    //Cannot put the followings to another method
    processDialog = new QDialog(mainWindow); //If put this to another method, then I cannot set the mainWindow as the parent
    QFile processFile("./process.ui");
    processFile.open(QFile::ReadOnly);

    chooseProc = loader.load(&processFile, processDialog);
    processFile.close();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(chooseProc);
    processDialog->setLayout(layout);
    processDialog->setModal(true);
    processDialog->resize(400, 400);

    //Statusbar message
    QStatusBar* statusBar = mainWindow->findChild<QStatusBar*>("statusbar");
    statusBar->showMessage("Tips: Left panel is scanned address. Right panel is stored address.");

    //Add signal
    QTreeWidget* procTreeWidget = chooseProc->findChild<QTreeWidget*>("procTreeWidget");
    QObject::connect(procTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onProcItemDblClicked(QTreeWidgetItem*, int)));

    procTreeWidget->installEventFilter(this);

    //Set default scan type
    mainWindow->findChild<QComboBox*>("scanType")->setCurrentIndex(1);

    //TODO: center
    mainWindow->show();

    qRegisterMetaType<QVector<int>>(); //For multithreading.

    //Tree model
    scanModel = new TreeModel(&med, mainWindow);
    mainWindow->findChild<QTreeView*>("scanTreeView")->setModel(scanModel);
    ComboBoxDelegate* delegate = new ComboBoxDelegate();
    mainWindow->findChild<QTreeView*>("scanTreeView")->setItemDelegateForColumn(SCAN_COL_TYPE, delegate);
    QObject::connect(mainWindow->findChild<QTreeView*>("scanTreeView"),
                     SIGNAL(clicked(QModelIndex)),
                     this,
                     SLOT(onScanTreeViewClicked(QModelIndex)));

    storeModel = new StoreTreeModel(&med, mainWindow);
    mainWindow->findChild<QTreeView*>("storeTreeView")->setModel(storeModel);
    ComboBoxDelegate* storeDelegate = new ComboBoxDelegate();
    mainWindow->findChild<QTreeView*>("storeTreeView")->setItemDelegateForColumn(ADDRESS_COL_TYPE, storeDelegate);
    QObject::connect(mainWindow->findChild<QTreeView*>("storeTreeView"),
                     SIGNAL(clicked(QModelIndex)),
                     this,
                     SLOT(onStoreTreeViewClicked(QModelIndex)));

    //Add signal to the process
    QWidget* process = mainWindow->findChild<QWidget*>("process");
    QObject::connect(process, SIGNAL(clicked()), this, SLOT(onProcessClicked()));

    //Add signal to scan
    QWidget* scanButton = mainWindow->findChild<QWidget*>("scanButton");
    QObject::connect(scanButton, SIGNAL(clicked()), this, SLOT(onScanClicked()));

    QWidget* filterButton = mainWindow->findChild<QWidget*>("filterButton");
    QObject::connect(filterButton, SIGNAL(clicked()), this, SLOT(onFilterClicked()));

    QObject::connect(mainWindow->findChild<QWidget*>("scanClear"),
                     SIGNAL(clicked()),
                     this,
                     SLOT(onClearClicked())
                     );

    //Add signal
    QObject::connect(mainWindow->findChild<QTreeWidget*>("scanTreeWidget"),
                     SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                     this,
                     SLOT(onScanItemChanged(QTreeWidgetItem*, int))
                     );
    QObject::connect(mainWindow->findChild<QTreeWidget*>("scanTreeWidget"),
                     SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                     this,
                     SLOT(onScanItemDblClicked(QTreeWidgetItem*, int))
                     );

    QObject::connect(mainWindow->findChild<QPushButton*>("scanAddAll"),
                     SIGNAL(clicked()),
                     this,
                     SLOT(onScanAddAllClicked())
                     );

    QObject::connect(mainWindow->findChild<QPushButton*>("scanAdd"),
                      SIGNAL(clicked()),
                      this,
                      SLOT(onScanAddClicked())
                      );

    QObject::connect(mainWindow->findChild<QTreeWidget*>("addressTreeWidget"),
                     SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                     this,
                     SLOT(onAddressItemChanged(QTreeWidgetItem*, int))
                     );

    QObject::connect(mainWindow->findChild<QPushButton*>("addressNew"),
                     SIGNAL(clicked()),
                     this,
                     SLOT(onAddressNewClicked()));
    QObject::connect(mainWindow->findChild<QPushButton*>("addressDelete"),
                     SIGNAL(clicked()),
                     this,
                     SLOT(onAddressDeleteClicked()));

    QObject::connect(mainWindow->findChild<QPushButton*>("addressShift"),
                     SIGNAL(clicked()),
                     this,
                     SLOT(onAddressShiftClicked()));

    QObject::connect(mainWindow->findChild<QAction*>("actionSaveAs"),
                     SIGNAL(triggered()),
                     this,
                     SLOT(onSaveAsTriggered()));
    QObject::connect(mainWindow->findChild<QAction*>("actionOpen"),
                     SIGNAL(triggered()),
                     this,
                     SLOT(onOpenTriggered()));

    /*QObject::connect(scanModel,
                     SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                     this,
                     SLOT(onScanTreeViewDataChanged(QModelIndex, QModelIndex, QVector<int>)));*/

    //Multi-threading
    refreshThread = new std::thread(MainUi::refresh, this);
  }

  bool eventFilter(QObject* obj, QEvent* ev) {
    QTreeWidget* procTreeWidget = chooseProc->findChild<QTreeWidget*>("procTreeWidget");
    if(obj == procTreeWidget && ev->type() == QEvent::KeyRelease) {
      if(static_cast<QKeyEvent*>(ev)->key() == Qt::Key_Return) { //Use Return instead of Enter
        onProcItemDblClicked(procTreeWidget->currentItem(), 0); //Just use the first column
      }
    }
  }

  /**
   * @deprecated
   */
  void addressToScanTreeWidget(Med med, string scanType, QTreeWidget* scanTreeWidget) {
    for(int i=0;i<med.scanAddresses.size();i++) {
      char address[32];
      sprintf(address, "%p", (void*)(med.scanAddresses[i].address));

      //Get the value from address and type
      string value = med.getScanAddressValueByIndex(i, scanType);

      QTreeWidgetItem* item = new QTreeWidgetItem(scanTreeWidget);
      item->setText(0, address);
      item->setText(2, value.c_str());

      item->setFlags(item->flags() | (Qt::ItemIsEditable));
    }

    //This is how to add combobox to the item
    QTreeWidgetItemIterator it(scanTreeWidget);
    while(*it) {
      QComboBox* combo = createTypeComboBox(scanTreeWidget, scanType);

      //Add property to the combobox, so that can be used to select the tree widget's row
      // http://stackoverflow.com/questions/30484784/how-to-work-with-signals-from-qtablewidget-cell-with-cellwidget-set
      combo->setProperty("tree-row", scanTreeWidget->indexOfTopLevelItem(*it));

      scanTreeWidget->setItemWidget(*it, 1, combo);

      //Add signal
      QObject::connect(combo,
                       SIGNAL(currentTextChanged(QString)),
                       this,
                       SLOT(onScanTypeChanged(QString)));
      it++;
    }
  }


  void updateNumberOfAddresses(QWidget* mainWindow) {
    char message[128];
    sprintf(message, "%ld addresses found", med.scanAddresses.size());
    mainWindow->findChild<QStatusBar*>("statusbar")->showMessage(message);
  }

  /**
   * @deprecated
   */
  void editScanValue(QTreeWidgetItem* item, int column) {
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    string text = item->text(column).toStdString();
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }

    try {
      med.setValueByAddress(med.scanAddresses[index].address,
                            text,
                            med.scanAddresses[index].getScanType());
    } catch(string e) {
      cerr << "editScanValue: "<<e<<endl;
    }
  }

  //TODO: Combine with editScanValue
  void editAddressValue(QTreeWidgetItem* item, int column) {
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    string text = item->text(column).toStdString();
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }

    try {
      med.setValueByAddress(med.addresses[index].address,
                            text,
                            med.addresses[index].getScanType());
    } catch(string e) {
      cerr << "editAddressValue: "<<e<<endl;
    }
  }

  void editAddressAddress(QTreeWidgetItem* item, int column) {
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    string text = item->text(column).toStdString();
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }

    long address;
    try {
      address = hexToInt(text);
      if(address == 0) //Do not read
        return;
      med.addresses[index].address = address;
      string value2 = med.getValueByAddress(med.addresses[index].address, med.addresses[index].getScanType());
      item->setText(3, value2.c_str());
    } catch(string e) {
      cerr << "editAddressAddress: "<<e<<endl;
    }
  }

  void editAddressDescription(QTreeWidgetItem* item, int column) {
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    string text = item->text(column).toStdString();

    med.addresses[index].description = text;
  }

  void editScanType(QTreeWidgetItem* item, string type) {
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    string text = type;
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }

    try {
      med.scanAddresses[index].setScanType(text);
      string value2 = med.getValueByAddress(med.scanAddresses[index].address, text);
      item->setText(2, value2.c_str());
    } catch(string e) {
      cerr << "editScanType: "<<e<<endl;
    }
  }

  void editAddressType(QTreeWidgetItem* item, string type) {
    int index = item->treeWidget()->indexOfTopLevelItem(item);
    string text = type;
    if(med.selectedProcess.pid == "") {
      cerr<< "No PID" <<endl;
      return;
    }

    try {
      med.addresses[index].setScanType(text);
      string value2 = med.getValueByAddress(med.addresses[index].address, text);
      item->setText(3, value2.c_str());
    } catch(string e) {
      cerr << "editScanType: "<<e<<endl;
    }
  }



  QComboBox* createTypeComboBox(QWidget* widget, string type) {
    QComboBox* combo = new QComboBox(widget);
    combo->addItems(QStringList() <<
                    "int8" <<
                    "int16" <<
                    "int32" <<
                    "float32" <<
                    "float64");

    combo->setCurrentText(type.c_str());
    return combo;
  }

};

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  MainUi* mainUi = new MainUi();
  return app.exec();
}

#include "main-qt.moc"
