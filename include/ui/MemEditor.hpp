#ifndef MEM_EDITOR_HPP
#define MEM_EDITOR_HPP

#include <string>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

#include "ui/Ui.hpp"
#include "med/MedTypes.hpp"
#include "mem/MemEd.hpp"

const int MEMORY_SIZE = 384; // 12 lines
const int ADDRESS_LINE = 24;

class MemEditor : public QWidget {
  Q_OBJECT

public:
  explicit MemEditor(MedUi* mainUi);
  virtual ~MemEditor();

  QPlainTextEdit* memArea;
  std::string memHex;

  void writeToProcessMemory(int position, char ch);
  void refresh();
  void updateAddresses();

  void setBaseAddress(Address addr);
  Address getBaseAddress();

  void boldText();

private slots:
  void onCurrAddressEdited();
  void onMemAreaCursorPositionChanged();
  void onRefreshButtonClicked();
  void onScanTypeComboChanged(int index);
  void onEnterClicked();

private:
  MemEd* med;
  MedUi* mainUi;
  QWidget* parent;
  QWidget* mainChild;
  Address baseAddress;

  QLineEdit* currAddress;
  QLineEdit* valueLine;
  QPlainTextEdit* addrArea;
  QPlainTextEdit* textArea;
  QPushButton* refreshButton;
  QComboBox *scanTypeCombo;
  QLineEdit *viewInt32;
  QLineEdit *viewFloat32;
  QLineEdit *viewFloat64;
  QPushButton *enterButton;

  Byte* rawMemory; // For re-use without keep read from PID

  void setupSignals();

  void loadMemory(Address address, size_t size = MEMORY_SIZE); // 12 lines
  void loadAddresses(Address address, size_t size = ADDRESS_LINE);

  void updateCurrAddress();
  void updateValueLine();
  void updateValueViews();
  void storeRawMemory(Byte* memory, size_t size);

  string getHexString(int position);
  Address getAddressByCursorPosition(int position);
  void setCursorPositionByAddress();

  static std::string memoryToHex(Byte* memory, size_t size);
  static std::string memoryToString(Byte* memory, size_t size, EncodingManager* encodingManager);

  void clearFormat();
};

#endif
