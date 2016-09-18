#include <iostream>
#include <cstdio>
#include <QApplication>
#include <QtUiTools>

#include "med.hpp"

using namespace std;


class MainUi : public QObject {
  Q_OBJECT

public:
  MainUi() {
    loadUiFiles();
  }
  Med med;

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
    QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    scanTreeWidget->clear();

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
      addressToScanTreeWidget(med, scanType, scanTreeWidget);
    }

    updateNumberOfAddresses(mainWindow);
  }

  void onFilterClicked() {
    QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    scanTreeWidget->clear();

    //Get scanned type
    string scanType = mainWindow->findChild<QComboBox*>("scanType")->currentText().toStdString();

    string scanValue = mainWindow->findChild<QLineEdit*>("scanEntry")->text().toStdString();

    med.scanFilter(scanValue, scanType);

    if(med.scanAddresses.size() <= 800)
      addressToScanTreeWidget(med, scanType, scanTreeWidget);

    updateNumberOfAddresses(mainWindow);
  }

  void onClearClicked() {
    mainWindow->findChild<QTreeWidget*>("scanTreeWidget")->clear();
    med.scanAddresses.clear();
    mainWindow->findChild<QStatusBar*>("statusbar")->showMessage("Scan cleared");
  }

  void onScanAddClicked() {
    QTreeWidget* scanTreeWidget = mainWindow->findChild<QTreeWidget*>("scanTreeWidget");
    QTreeWidgetItem* item = scanTreeWidget->currentItem();
    int index = scanTreeWidget->indexOfTopLevelItem(item);

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
    itemToAdd->setText(4, "false");
    itemToAdd->setFlags(itemToAdd->flags() | Qt::ItemIsEditable);

    //Add combo box
    QComboBox* combo = createTypeComboBox(addressTreeWidget, medAddress.getScanType());
    combo->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
    addressTreeWidget->setItemWidget(itemToAdd, 2, combo);

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
    case 0:
      editAddressDescription(item, column);
      break;
    case 1:
      editAddressAddress(item, column);
      break;
    case 2: //Not available, because the 2nd column is combobox
      break;
    case 3:
      editAddressValue(item, column);
      break;
    }
  }

  void onAddressNewClicked() {
    MedAddress medAddress;
    medAddress.description = "Your description";
    medAddress.address = 0;
    medAddress.setScanType("int8");
    medAddress.lock = false;
    med.addresses.push_back(medAddress);

     //Add to AddressTreeWidget
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItem* itemToAdd = new QTreeWidgetItem(addressTreeWidget);    itemToAdd->setText(0, medAddress.description.c_str());
    itemToAdd->setText(1, "0");
    itemToAdd->setText(4, "false");
    itemToAdd->setFlags(itemToAdd->flags() | Qt::ItemIsEditable);

    //Add combo box
    QComboBox* combo = createTypeComboBox(addressTreeWidget, medAddress.getScanType());
    combo->setProperty("tree-row", addressTreeWidget->indexOfTopLevelItem(itemToAdd));
    addressTreeWidget->setItemWidget(itemToAdd, 2, combo);

    //Add signal
    QObject::connect(combo,
                     SIGNAL(currentTextChanged(QString)),
                     this,
                     SLOT(onAddressTypeChanged(QString)));
  }

  void onAddressDeleteClicked() {
    QTreeWidget* addressTreeWidget = mainWindow->findChild<QTreeWidget*>("addressTreeWidget");
    QTreeWidgetItem* item = addressTreeWidget->currentItem();
    delete item;
  }

private:
  QWidget* mainWindow;
  QWidget* chooseProc;
  QDialog* processDialog;

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

    //TODO: center
    mainWindow->show();

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

  }

  bool eventFilter(QObject* obj, QEvent* ev) {
    QTreeWidget* procTreeWidget = chooseProc->findChild<QTreeWidget*>("procTreeWidget");
    if(obj == procTreeWidget && ev->type() == QEvent::KeyRelease) {
      if(static_cast<QKeyEvent*>(ev)->key() == Qt::Key_Return) { //Use Return instead of Enter
        onProcItemDblClicked(procTreeWidget->currentItem(), 0); //Just use the first column
      }
    }
  }

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



  QComboBox* createTypeComboBox(QTreeWidget* widget, string type) {
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
