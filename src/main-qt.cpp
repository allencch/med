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
    line->setText(QString::fromLatin1(med.selectedProcess.cmdline.c_str())); //Do not use fromStdString(), it will append with some unknown characters

    processDialog->hide();
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

    //Add signal
    QTreeWidget* procTreeWidget = chooseProc->findChild<QTreeWidget*>("procTreeWidget");
    QObject::connect(procTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onProcItemDblClicked(QTreeWidgetItem*, int)));


    //TODO: center
    mainWindow->show();

    //Add signal to the process
    QWidget* process = mainWindow->findChild<QWidget*>("process");
    QObject::connect(process, SIGNAL(clicked()), this, SLOT(onProcessClicked()));

  }

};

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  MainUi* mainUi = new MainUi();
  return app.exec();
}

#include "main-qt.moc"
