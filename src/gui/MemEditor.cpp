#include <string>
#include <iostream>

#include <QWidget>
#include <QtUiTools>

#include "gui/MemEditor.hpp"
#include "med/MemOperator.hpp"

using namespace std;

MemEditor::MemEditor(QWidget* parent, MainUi* mainUi) : QWidget(parent, Qt::Tool) {
  this->mainUi = mainUi;
  this->med = &(mainUi->med);

  QUiLoader loader;
  QFile file("./mem-editor.ui");
  file.open(QFile::ReadOnly);
  mainChild = loader.load(&file, this);
  file.close();
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(mainChild);
  this->setLayout(layout);
  this->setWindowModality(Qt::WindowModal);
  this->resize(600, 500);

  baseAddress = mainChild->findChild<QLineEdit*>("baseAddress");
  memArea = mainChild->findChild<QPlainTextEdit*>("memArea");

  QFont font("Monospace");
  font.setStyleHint(QFont::Monospace);
  memArea->setFont(font);

  setupSignals();
}

void MemEditor::setupSignals() {
  QObject::connect(baseAddress,
                   SIGNAL(editingFinished()),
                   this,
                   SLOT(onBaseAddressEdited()));
}


void MemEditor::onBaseAddressEdited() {
  QString addr = baseAddress->text();
  if (addr == "") {
    return;
  }

  try {
    MemAddr roundedAddr = addressRoundDown(hexToInt(addr.toStdString()));
    baseAddress->setText(intToHex(roundedAddr).c_str());
    loadMemory(roundedAddr);

  } catch(MedException &ex) {
    cerr << ex.getMessage() << endl;
  }
}

void MemEditor::loadMemory(MemAddr address, size_t size) {
  if (med->selectedProcess.pid == "") {
    mainUi->statusBar->showMessage("No process selected");
  }

  Byte* memory = med->readMemory(address, size);
  string memoryView = memoryToString(memory, size);
  free(memory);

  memArea->setPlainText(QString(memoryView.c_str()));
}

string MemEditor::memoryToString(Byte* memory, size_t size) {
  string memoryView = "";
  char buffer[4];
  for (int i = 0; i < (int)size; i++) {
    sprintf(buffer, "%02X ", memory[i]);
    memoryView += buffer;
    if (i % 16 == 15) {
      memoryView += "\n";
    }
  }
  return memoryView;
}
