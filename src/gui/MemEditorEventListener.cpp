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
