#ifndef MEM_EDITOR_HPP
#define MEM_EDITOR_HPP

#include <string>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
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
  QLineEdit* baseAddress;

  void writeToProcessMemory(int position, char ch);
  void refresh();
  void updateAddresses();

private slots:
  void onBaseAddressEdited();
  void onMemAreaCursorPositionChanged();
  void onRefreshButtonClicked();

private:
  Med* med;
  MainUi* mainUi;
  QWidget* parent;
  QWidget* mainChild;

  QLineEdit* currAddress;
  QLineEdit* valueLine;
  QPlainTextEdit* addrArea;
  QPlainTextEdit* textArea;
  QPushButton* refreshButton;

  Byte* rawMemory; // For re-use without keep read from PID

  void setupSignals();

  void loadMemory(MemAddr address, size_t size = MEMORY_SIZE); // 12 lines
  void loadAddresses(MemAddr address, size_t size = ADDRESS_LINE);

  void updateCurrAddress();
  void updateValueLine();
  void storeRawMemory(Byte* memory, size_t size);

  string getHexString(int position);
  MemAddr getAddressByCursorPosition(int position);

  static std::string memoryToHex(Byte* memory, size_t size);
  static std::string memoryToString(Byte* memory, size_t size);
};

#endif
