#include <cstdio>

#include <QtUiTools>
#include <QtDebug>

#include "ui/Ui.hpp"
#include "ui/ProcessEventListener.hpp"

MedUi::MedUi(QApplication* app) {
  this->app = app;
  med = new MemEd();

  loadUiFiles();
  loadProcessUi();
  setupSignals();
  setupUi();
  // TODO: other action here
}

MedUi::~MedUi() {
  delete med;
}

void MedUi::loadUiFiles() {
  QUiLoader loader;
  QFile file("./main-qt.ui");
  file.open(QFile::ReadOnly);
  mainWindow = loader.load(&file);
  file.close();

  selectedProcessLine = mainWindow->findChild<QLineEdit*>("selectedProcess");
  // TODO: other setup here
}

void MedUi::setupUi() {
  mainWindow->show();
}

void MedUi::setupSignals() {
  QObject::connect(mainWindow->findChild<QWidget*>("process"),
                   SIGNAL(clicked()),
                   this,
                   SLOT(onProcessClicked()));
}

void MedUi::loadProcessUi() {
  QUiLoader loader;
  //Cannot put the followings to another method
  processDialog = new QDialog(mainWindow); //If put this to another method, then I cannot set the mainWindow as the parent
  QFile processFile("./process.ui");
  processFile.open(QFile::ReadOnly);

  processSelector = loader.load(&processFile, processDialog);
  processFile.close();

  processTreeWidget = processSelector->findChild<QTreeWidget*>("processTreeWidget");

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(processSelector);
  processDialog->setLayout(layout);
  processDialog->setModal(true);
  processDialog->resize(400, 400);

  //Add signal
  QObject::connect(processTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onProcessItemDblClicked(QTreeWidgetItem*, int)));

  processTreeWidget->installEventFilter(new ProcessDialogEventListener(this));
}

void MedUi::onProcessClicked() {
  med->listProcesses();

  processDialog->show();

  processTreeWidget->clear(); //Remove all items

  //Add all the process into the tree widget
  for (int i = med->processes.size() - 1; i >= 0; i--) {
    QTreeWidgetItem* item = new QTreeWidgetItem(processTreeWidget);
    item->setText(0, med->processes[i].pid.c_str());
    item->setText(1, med->processes[i].cmdline.c_str());
  }
}

void MedUi::onProcessItemDblClicked(QTreeWidgetItem* item, int) {
  int index = item->treeWidget()->indexOfTopLevelItem(item); //Get the current row index

  Process process = med->selectProcessByIndex(med->processes.size() - 1 - index);

  selectedProcessLine->setText(QString::fromLatin1((process.pid + " " + process.cmdline).c_str())); //Do not use fromStdString(), it will append with some unknown characters

  processDialog->hide();
}
