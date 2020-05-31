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
  baseAddress = 0;

  QUiLoader loader;
  QFile file("./mem-editor.ui");
  file.open(QFile::ReadOnly);
  mainChild = loader.load(&file, this);
  file.close();
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(mainChild);
  this->setLayout(layout);
  this->resize(800, 500);

  currAddress= mainChild->findChild<QLineEdit*>("currAddress");
  valueLine = mainChild->findChild<QLineEdit*>("value");
  memArea = mainChild->findChild<QPlainTextEdit*>("memArea");
  addrArea = mainChild->findChild<QPlainTextEdit*>("addrArea");
  textArea = mainChild->findChild<QPlainTextEdit*>("textArea");
  refreshButton = mainChild->findChild<QPushButton*>("refresh");
  scanTypeCombo = mainChild->findChild<QComboBox*>("scanType");

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
  QObject::connect(currAddress,
                   SIGNAL(editingFinished()),
                   this,
                   SLOT(onCurrAddressEdited()));
  QObject::connect(memArea,
                   SIGNAL(cursorPositionChanged()),
                   this,
                   SLOT(onMemAreaCursorPositionChanged()));
  QObject::connect(refreshButton,
                   SIGNAL(clicked()),
                   this,
                   SLOT(onRefreshButtonClicked()));
  QObject::connect(scanTypeCombo,
                   SIGNAL(currentIndexChanged(int)),
                   this,
                   SLOT(onScanTypeComboChanged(int)));
}


void MemEditor::onCurrAddressEdited() {
  QString addr = currAddress->text();
  if (addr.trimmed() == "") {
    return;
  }

  if (med->selectedProcess.pid == "") {
    mainUi->statusBar->showMessage("No process selected");
    return;
  }

  try {
    baseAddress = addressRoundDown(hexToInt(addr.toStdString()));
    loadMemory(baseAddress);
    loadAddresses(baseAddress);
    currAddress->setText(addr);
    setCursorPositionByAddress();
  } catch(MedException &ex) {
    cerr << ex.getMessage() << endl;
  }
}

void MemEditor::refresh() {
  if (!baseAddress) {
    return;
  }
  QString addr = currAddress->text();
  loadMemory(baseAddress);
  currAddress->setText(addr);
  setCursorPositionByAddress();
}

void MemEditor::updateAddresses() {
  if (!baseAddress) {
    return;
  }
  loadAddresses(baseAddress);
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
  boldText();
}

Address MemEditor::getAddressByCursorPosition(int position) {
  if (!baseAddress) {
    return 0;
  }
  int distance = position / 3;
  return baseAddress + distance;
}

void MemEditor::setCursorPositionByAddress() {
  QString addr = currAddress->text();
  if (!baseAddress || addr.trimmed() == "") {
    return;
  }

  int distance = hexToInt(addr.toStdString()) - baseAddress;
  int position = distance * 3;

  auto cursor = memArea->textCursor();
  cursor.setPosition(position, QTextCursor::MoveAnchor);
  memArea->setTextCursor(cursor);
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

  string scanType = scanTypeCombo->currentText().toStdString();

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

void MemEditor::setBaseAddress(Address addr) {
  baseAddress = addr;
}

Address MemEditor::getBaseAddress() {
  return baseAddress;
}

void MemEditor::clearFormat() {
  QTextCursor cursor = memArea->textCursor();
  auto cursorPosition = cursor.position();

  cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

  QTextCharFormat format;
  format.setFontWeight(QFont::Normal);
  cursor.mergeCharFormat(format);

  cursor.setPosition(cursorPosition);
}

void MemEditor::boldText() {
  QString addr = currAddress->text();
  if (addr.trimmed() == "") {
    return;
  }

  clearFormat();

  int distance = hexToInt(addr.toStdString()) - baseAddress;
  int position = distance * 3;

  QTextCursor cursor = memArea->textCursor();
  auto cursorPosition = cursor.position();

  cursor.setPosition(position, QTextCursor::MoveAnchor);
  cursor.setPosition(position + 2, QTextCursor::KeepAnchor);

  QTextCharFormat format;
  format.setFontWeight(QFont::Bold);
  cursor.mergeCharFormat(format);

  cursor.setPosition(cursorPosition);
}

void MemEditor::onScanTypeComboChanged(int) {
  refresh();
}
