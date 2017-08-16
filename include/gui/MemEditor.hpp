#ifndef MEM_EDITOR_HPP
#define MEM_EDITOR_HPP

#include <string>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "gui/med-qt.hpp"
#include "med/med.hpp"

const int MEMORY_SIZE = 384; // 12 lines
const int ADDRESS_LINE = 24;

class MemEditor : public QWidget {
  Q_OBJECT

public:
  MemEditor(MainUi* mainUi);
  virtual ~MemEditor();
  QPlainTextEdit* memArea;
  std::string memHex;

  void writeToProcessMemory(int position, char ch);

private slots:
  void onBaseAddressEdited();
  void onMemAreaCursorPositionChanged();

private:
  Med* med;
  MainUi* mainUi;
  QWidget* parent;
  QWidget* mainChild;

  QLineEdit* baseAddress;
  QLineEdit* currAddress;
  QLineEdit* valueLine;
  QPlainTextEdit* addrArea;
  QPlainTextEdit* textArea;

  Byte* rawMemory; // For re-use without keep read from PID

  void setupSignals();

  void loadMemory(MemAddr address, size_t size = MEMORY_SIZE); // 12 lines
  void loadAddresses(MemAddr address, size_t size = ADDRESS_LINE);

  void updateCurrAddress();
  void updateValueLine();
  void storeRawMemory(Byte* memory, size_t size);

  string getHexString(int position);
  MemAddr getAddressByCursorPosition(int position);

  void refresh();

  static std::string memoryToHex(Byte* memory, size_t size);
  static std::string memoryToString(Byte* memory, size_t size);
};

#endif
