#include <cctype>
#include <iostream>
#include "gui/MemEditorEventListener.hpp"

MemEditorEventListener::MemEditorEventListener(MemEditor* memEditor) {
  this->memEditor = memEditor;
  memArea = memEditor->memArea;
}

bool MemEditorEventListener::eventFilter(QObject* obj, QEvent* ev) {
  if (ev->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);

    // Navigation events should not user cursorPositionChanged() signal. Because it is done before this.
    // As a result, we cannot reset to the position we want if the position is changed in cursorPositionChanged.
    if (keyEvent->key() == Qt::Key_Left) {
      int position = memArea->textCursor().position();
      moveCursorBackward(position - 1);
      return true;
    }
    else if (keyEvent->key() == Qt::Key_Right) {
      int position = memArea->textCursor().position();
      moveCursorForward(position + 1);
      return true;
    }
    else if (keyEvent->key() == Qt::Key_Up) {
      moveCursorUpward();
      return true;
    }
    else if (keyEvent->key() == Qt::Key_Down) {
      moveCursorDownward();
      return true;
    }
    else {
      return handleHexaInput(keyEvent->key());
    }
  }
  return false;
}

void MemEditorEventListener::moveCursorBackward(int position) {
  auto& memHex = memEditor->memHex;
  if (!isspace(memHex[position + 1]) || position < 0) {
    return;
  }

  if (isspace(memHex[position])) {
    moveCursorBackward(position - 1);
  }
  else {
    auto cursor = memArea->textCursor();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    memArea->setTextCursor(cursor);
  }
}

void MemEditorEventListener::moveCursorForward(int position) {
  auto& memHex = memEditor->memHex;
  if (!isspace(memHex[position - 1]) || position >= (int)memHex.size()) {
    return;
  }

  if (isspace(memHex[position])) {
    moveCursorForward(position + 1);
  }
  else {
    auto cursor = memArea->textCursor();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    memArea->setTextCursor(cursor);
  }
}

bool MemEditorEventListener::handleHexaInput(int key) {
  if (!((key >= Qt::Key_0 && key <= Qt::Key_9) ||
        (key >= Qt::Key_A && key <= Qt::Key_F))) {
    return false;
  }
  char ch = key;
  auto cursor = memArea->textCursor();
  int position = cursor.position();
  if (isspace(memEditor->memHex[position])) {
    return false;
  }
  memEditor->writeToProcessMemory(position, ch);
  moveCursorNext(position + 1);
  return false;
}

void MemEditorEventListener::moveCursorNext(int position) {
  auto& memHex = memEditor->memHex;

  if (position >= (int)memHex.size()) {
    return;
  }
  
  if (isspace(memHex[position])) {
    moveCursorNext(position + 1);
  }
  else {
    auto cursor = memArea->textCursor();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    memArea->setTextCursor(cursor);
  }
}

void MemEditorEventListener::moveCursorUpward() {
  auto cursor = memArea->textCursor();
  int position = cursor.position();
  if (position >= 16 * 3) { // one line has 16 bytes, each byte represented by 3 characters
    return;
  }

  MemAddr base = hexToInt(memEditor->baseAddress->text().toStdString());
  base -= 16;
  memEditor->baseAddress->setText(intToHex(base).c_str());
  memEditor->refresh();
  memEditor->updateAddresses();
  cursor.setPosition(position, QTextCursor::MoveAnchor);
  memArea->setTextCursor(cursor);
}

void MemEditorEventListener::moveCursorDownward() {
  auto cursor = memArea->textCursor();
  int position = cursor.position();
  if (position < (ADDRESS_LINE - 1) * 16 * 3) { // one line has 16 bytes, each byte represented by 3 characters
    return;
  }

  MemAddr base = hexToInt(memEditor->baseAddress->text().toStdString());
  base += 16;
  memEditor->baseAddress->setText(intToHex(base).c_str());
  memEditor->refresh();
  memEditor->updateAddresses();
  cursor.setPosition(position, QTextCursor::MoveAnchor);
  memArea->setTextCursor(cursor);
}
