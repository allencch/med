#ifndef MEM_EDITOR_HPP
#define MEM_EDITOR_HPP

#include <string>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "gui/med-qt.hpp"
#include "med/med.hpp"

class MemEditor : public QWidget {
  Q_OBJECT

public:
  MemEditor(QWidget* parent, MainUi* mainUi);

private slots:
  void onBaseAddressEdited();

private:
  Med* med;
  MainUi* mainUi;
  QWidget* parent;
  QWidget* mainChild;

  QLineEdit* baseAddress;
  QPlainTextEdit* memArea;

  void setupSignals();

  void loadMemory(MemAddr address, size_t size = 192); // 12 lines

  static std::string memoryToString(Byte* memory, size_t size);
};

#endif
