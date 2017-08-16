#include <string>
#include <iostream>
#include <cctype>

#include <QWidget>
#include <QtUiTools>
#include <QDebug>

#include "gui/MemEditor.hpp"
#include "gui/MemEditorEventListener.hpp"
#include "med/MemOperator.hpp"

using namespace std;

MemEditor::MemEditor(MainUi* mainUi) : QWidget(NULL, Qt::SubWindow) {
  this->mainUi = mainUi;
  this->med = &(mainUi->med);
  rawMemory = NULL;

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
  currAddress= mainChild->findChild<QLineEdit*>("currAddress");
  valueLine = mainChild->findChild<QLineEdit*>("value");
  memArea = mainChild->findChild<QPlainTextEdit*>("memArea");
  addrArea = mainChild->findChild<QPlainTextEdit*>("addrArea");
  textArea = mainChild->findChild<QPlainTextEdit*>("textArea");

  QFont font("Monospace");
  font.setStyleHint(QFont::Monospace);
  memArea->setFont(font);
  memArea->setTextInteractionFlags(memArea->textInteractionFlags() | Qt::TextSelectableByKeyboard);
  addrArea->setFont(font);
  textArea->setFont(font);

  memArea->installEventFilter(new MemEditorEventListener(this));

  setupSignals();
}

MemEditor::~MemEditor() {
  if (rawMemory) {
    free(rawMemory);
    rawMemory = NULL;
  }
}

void MemEditor::setupSignals() {
  QObject::connect(baseAddress,
                   SIGNAL(editingFinished()),
                   this,
                   SLOT(onBaseAddressEdited()));
  QObject::connect(memArea,
                   SIGNAL(cursorPositionChanged()),
                   this,
                   SLOT(onMemAreaCursorPositionChanged()));
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

void MemEditor::refresh() {
  QString addr = baseAddress->text();
  MemAddr roundedAddr = addressRoundDown(hexToInt(addr.toStdString()));
  baseAddress->setText(intToHex(roundedAddr).c_str());
  loadMemory(roundedAddr);
}

void MemEditor::loadMemory(MemAddr address, size_t size) {
  Byte* memory = med->readMemory(address, size);
  memHex = memoryToHex(memory, size);
  string textView = memoryToString(memory, size);
  storeRawMemory(memory, size);
  free(memory);

  memArea->setPlainText(QString(memHex.c_str()));
  textArea->setPlainText(QString(textView.c_str()));
}

string MemEditor::memoryToHex(Byte* memory, size_t size) {
  string memoryView = "";
  char buffer[4];
  for (int i = 0; i < (int)size; i++) {
    sprintf(buffer, "%02X", memory[i]);
    memoryView += buffer;
    if (i % 16 == 15) {
      memoryView += "\n";
    }
    else {
      memoryView += " ";
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

void MemEditor::loadAddresses(MemAddr address, size_t size) {
  string addressView = "";
  for (int i = 0; i < (int)size; i++) {
    addressView += intToHex(address) + "\n";
    address += 16;
  }
  addrArea->setPlainText(QString(addressView.c_str()));
}

void MemEditor::onMemAreaCursorPositionChanged() {
  updateCurrAddress();
  updateValueLine();
}

MemAddr MemEditor::getAddressByCursorPosition(int position) {
  int distance = position / 3;

  MemAddr base = hexToInt(baseAddress->text().toStdString());
  return base + distance;
}

void MemEditor::updateCurrAddress() {
  int position = memArea->textCursor().position();
  MemAddr curr = getAddressByCursorPosition(position);
  currAddress->setText(intToHex(curr).c_str());
}

void MemEditor::updateValueLine() {
  int position = memArea->textCursor().position();
  int distance = position / 3;

  string scanType = mainChild->findChild<QComboBox*>("scanType")->currentText().toStdString();

  string value = memToString(rawMemory + distance, scanType);
  valueLine->setText(value.c_str());
}

void MemEditor::storeRawMemory(Byte* memory, size_t size) {
  if (!rawMemory) {
    rawMemory = (Byte*)malloc(size);
  }
  memcpy(rawMemory, memory, size);
}

void MemEditor::writeToProcessMemory(int position, char ch) {
  string hexaString = getHexString(position);
  hexaString[position % 3 % 2] = ch;
  string value = to_string(hexToInt(hexaString));

  MemAddr address = getAddressByCursorPosition(position);
  mainUi->med.setValueByAddress(address, value, "int8");

  refresh();
}

string MemEditor::getHexString(int position) {
  int rounded = position - (position % 3);
  return memHex.substr(rounded, 2);
}
