#include "gui/MemEditor.hpp"

class MemEditorEventListener : public QObject {
  Q_OBJECT
public:
  explicit MemEditorEventListener(MemEditor* memEditor);
  bool eventFilter(QObject* obj, QEvent* ev);
private:
  MemEditor* memEditor;
  QPlainTextEdit* memArea;

  void moveCursorBackward(int position);
  void moveCursorForward(int position);
  bool handleHexaInput(int key);
  void moveCursorNext(int position);
  void moveCursorUpward();
  void moveCursorDownward();
};
