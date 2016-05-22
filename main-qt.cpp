#include <iostream>
#include <cstdio>
#include <QApplication>
#include <QtUiTools>

using namespace std;

class MainUi : public QObject {
  Q_OBJECT

public:
  MainUi() {
    loadUiFiles();
  }

private slots:
  void onProcessClicked() {
    dialog->show();
  }

private:
  QWidget* mainWindow;
  QDialog* dialog;

  void loadUiFiles() {
    QUiLoader loader;

    QFile file("../main-qt.ui");
    file.open(QFile::ReadOnly);
    QWidget* mainWindow = loader.load(&file);
    file.close();

    //Cannot put the followings to another method
    dialog = new QDialog(mainWindow); //If put this to another method, then I cannot set the mainWindow as the parent
    QFile processFile("../process.ui");
    processFile.open(QFile::ReadOnly);

    QWidget* chooseProc = loader.load(&processFile, dialog);
    processFile.close();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(chooseProc);
    dialog->setLayout(layout);
    dialog->setModal(true);

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
