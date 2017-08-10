#ifndef MEM_EDITOR_HPP
#define MEM_EDITOR_HPP

#include <string>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "gui/med-qt.hpp"
#include "med/med.hpp"

const int MEMORY_SIZE = 192; // 12 lines
const int ADDRESS_LINE = 12;

class MemEditor : public QWidget {
  Q_OBJECT

public:
  MemEditor(MainUi* mainUi);

private slots:
  void onBaseAddressEdited();

private:
  Med* med;
  MainUi* mainUi;
  QWidget* parent;
  QWidget* mainChild;

  QLineEdit* baseAddress;
  QPlainTextEdit* memArea;
  QPlainTextEdit* addrArea;
  QPlainTextEdit* textArea;

  void setupSignals();

  void loadMemory(MemAddr address, size_t size = MEMORY_SIZE); // 12 lines
  void loadAddresses(MemAddr address, size_t size = ADDRESS_LINE);

  static std::string memoryToString(Byte* memory, size_t size);
};

#endif
