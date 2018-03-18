#include <string>
#include <iostream>
#include <cctype>

#include <QWidget>
#include <QtUiTools>
#include <QDebug>

#include "ui/MemEditor.hpp"
#include "ui/MemEditorEventListener.hpp"
#include "ui/EncodingManager.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "mem/Pem.hpp"

using namespace std;

MemEditor::MemEditor(MedUi* mainUi) : QWidget(NULL, Qt::SubWindow) {
  this->mainUi = mainUi;
  this->med = mainUi->med;
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
  refreshButton = mainChild->findChild<QPushButton*>("refresh");

  QFont font("FreeMono");
  font.setStyleHint(QFont::Monospace);
  font.setPixelSize(14);
  memArea->setFont(font);
  memArea->setTextInteractionFlags(memArea->textInteractionFlags() | Qt::TextSelectableByKeyboard);
  addrArea->setFont(font);
  textArea->setFont(font);

  memArea->installEventFilter(new MemEditorEventListener(this));

  setupSignals();
}

MemEditor::~MemEditor() {
  if (rawMemory) {
    delete[] rawMemory;
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
  QObject::connect(refreshButton,
                   SIGNAL(clicked()),
                   this,
                   SLOT(onRefreshButtonClicked()));
}


void MemEditor::onBaseAddressEdited() {
  QString addr = baseAddress->text();
  if (addr.trimmed() == "") {
    return;
  }

  if (med->selectedProcess.pid == "") {
    mainUi->statusBar->showMessage("No process selected");
    return;
  }

  try {
    Address roundedAddr = addressRoundDown(hexToInt(addr.toStdString()));
    baseAddress->setText(intToHex(roundedAddr).c_str());
    loadMemory(roundedAddr);
    loadAddresses(roundedAddr);

  } catch(MedException &ex) {
    cerr << ex.getMessage() << endl;
  }
}

void MemEditor::refresh() {
  QString addr = baseAddress->text();
  if (addr.trimmed() == "") {
    return;
  }
  Address roundedAddr = addressRoundDown(hexToInt(addr.toStdString()));
  baseAddress->setText(intToHex(roundedAddr).c_str());
  loadMemory(roundedAddr);
}

void MemEditor::updateAddresses() {
  QString addr = baseAddress->text();
  if (addr.trimmed() == "") {
    return;
  }
  loadAddresses(hexToInt(addr.toStdString()));
}

void MemEditor::loadMemory(Address address, size_t size) {
  MemPtr mem = med->readMemory(address, size);
  memHex = memoryToHex(mem->getData(), size);
  string textView = memoryToString(mem->getData(), size, mainUi->encodingManager);
  storeRawMemory(mem->getData(), size);

  memArea->setPlainText(QString(memHex.c_str()));
  textArea->setPlainText(QString(textView.c_str()));
}

string MemEditor::memoryToHex(Byte* memory, size_t size) {
  string memoryView = "";
  char buffer[4];
  for (int i = 0; i < (int)size; i++) {
    sprintf(buffer, "%02x", memory[i]);
    memoryView += buffer;
    if (i % 16 == 15 && i != MEMORY_SIZE - 1) {
      memoryView += "\n";
    }
    else {
      memoryView += " ";
    }
  }
  return memoryView;
}

string MemEditor::memoryToString(Byte* memory, size_t size, EncodingManager* encodingManager) {
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

  return encodingManager->convertToUtf8(textView);
}

void MemEditor::loadAddresses(Address address, size_t size) {
  string addressView = "";
  for (size_t i = 0; i < size; i++) {
    addressView += intToHex(address) + "\n";
    address += 16;
  }
  addrArea->setPlainText(QString(addressView.c_str()));
}

void MemEditor::onMemAreaCursorPositionChanged() {
  updateCurrAddress();
  updateValueLine();
}

Address MemEditor::getAddressByCursorPosition(int position) {
  QString addr = baseAddress->text();
  if (addr.trimmed() == "") {
    return 0;
  }
  int distance = position / 3;
  Address base = hexToInt(addr.toStdString());
  return base + distance;
}

void MemEditor::updateCurrAddress() {
  int position = memArea->textCursor().position();
  Address curr = getAddressByCursorPosition(position);
  if (curr == 0) {
    return;
  }
  currAddress->setText(intToHex(curr).c_str());
}

void MemEditor::updateValueLine() {
  int position = memArea->textCursor().position();
  int distance = position / 3;

  string scanType = mainChild->findChild<QComboBox*>("scanType")->currentText().toStdString();

  string value = Pem::bytesToString(rawMemory + distance, scanType);
  valueLine->setText(value.c_str());
}

void MemEditor::storeRawMemory(Byte* memory, size_t size) {
  if (!rawMemory) {
    rawMemory = new Byte[size];
  }
  memcpy(rawMemory, memory, size);
}

void MemEditor::writeToProcessMemory(int position, char ch) {
  string hexaString = getHexString(position);
  hexaString[position % 3 % 2] = ch;
  string value = to_string(hexToInt(hexaString));

  Address address = getAddressByCursorPosition(position);
  if (address) {
    mainUi->med->setValueByAddress(address, value, SCAN_TYPE_INT_8);
  }

  refresh();
}

string MemEditor::getHexString(int position) {
  int rounded = position - (position % 3);
  return memHex.substr(rounded, 2);
}

void MemEditor::onRefreshButtonClicked() {
  refresh();
}
