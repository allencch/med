#include <string>
#include <iostream>
#include <cctype>

#include <QWidget>
#include <QtUiTools>

#include "gui/MemEditor.hpp"
#include "med/MemOperator.hpp"

using namespace std;

MemEditor::MemEditor(MainUi* mainUi) : QWidget(NULL, Qt::SubWindow) {
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
  this->resize(800, 500);

  baseAddress = mainChild->findChild<QLineEdit*>("baseAddress");
  memArea = mainChild->findChild<QPlainTextEdit*>("memArea");
  addrArea = mainChild->findChild<QPlainTextEdit*>("addrArea");
  textArea = mainChild->findChild<QPlainTextEdit*>("textArea");

  QFont font("Monospace");
  font.setStyleHint(QFont::Monospace);
  memArea->setFont(font);
  addrArea->setFont(font);
  textArea->setFont(font);

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
  
  if (med->selectedProcess.pid == "") {
    mainUi->statusBar->showMessage("No process selected");
    return;
  }

  try {
    MemAddr roundedAddr = addressRoundDown(hexToInt(addr.toStdString()));
    baseAddress->setText(intToHex(roundedAddr).c_str());
    loadMemory(roundedAddr);
    loadAddresses(roundedAddr);

  } catch(MedException &ex) {
    cerr << ex.getMessage() << endl;
  }
}

void MemEditor::loadMemory(MemAddr address, size_t size) {
  Byte* memory = med->readMemory(address, size);
  string memoryView = memoryToHex(memory, size);
  string textView = memoryToString(memory, size);
  free(memory);

  memArea->setPlainText(QString(memoryView.c_str()));
  textArea->setPlainText(QString(textView.c_str()));
}

string MemEditor::memoryToHex(Byte* memory, size_t size) {
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

string MemEditor::memoryToString(Byte* memory, size_t size) {
  string textView = "";
  for (int i = 0; i < (int)size; i++) {
    if (iscntrl(memory[i])) {
      textView += ".";
    }
    else {
      textView += memory[i];
    }
    if (i % 16 == 15) {
      textView += "\n";
    }
  }
  return textView;
}

void  MemEditor::loadAddresses(MemAddr address, size_t size) {
  string addressView = "";
  for (int i = 0; i < (int)size; i++) {
    addressView += intToHex(address) + "\n";
    address += 16;
  }
  addrArea->setPlainText(QString(addressView.c_str()));
}
