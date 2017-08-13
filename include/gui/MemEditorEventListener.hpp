#include "gui/MemEditor.hpp"

class MemEditorEventListener : public QObject {
  Q_OBJECT
public:
  MemEditorEventListener(MemEditor* memEditor);
  bool eventFilter(QObject* obj, QEvent* ev);
private:
  MemEditor* memEditor;
  QPlainTextEdit* memArea;

  void moveCursorBackward(int position);
  void moveCursorForward(int position);
};
