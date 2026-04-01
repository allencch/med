#ifndef MED_MEM_EDITOR_HPP
#define MED_MEM_EDITOR_HPP

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"

class MainWindow;

class MemEditor : public QWidget {
    Q_OBJECT

public:
    explicit MemEditor(MainWindow* mainWin);
    ~MemEditor();

    void setBaseAddress(Address addr);
    void refresh();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

public slots:
    void onCurrAddressEdited();
    void onRefreshButtonClicked();
    void onEnterClicked();
    void onMemAreaCursorPositionChanged();
    void onMemoryReady(Address addr, const SizedBytes& data);

private:
    void setupUi();
    void loadMemory(Address address);
    void loadAddresses(Address address);
    void writeToProcessMemory(int position, char ch);

    MainWindow* mainWindow_;
    Address baseAddress_ = 0;

    QLineEdit* currAddressEdit_;
    QLineEdit* valueEdit_;
    QPlainTextEdit* memArea_;
    QPlainTextEdit* addrArea_;
    QPlainTextEdit* textArea_;
    QPushButton* refreshButton_;
    QComboBox* scanTypeCombo_;
    QLineEdit* viewInt32_;
    QLineEdit* viewFloat32_;
    QLineEdit* viewFloat64_;
    QPushButton* enterButton_;

    std::string memHex_;
    BytePtr rawMemory_;
    size_t rawMemorySize_ = 0;

    static std::string memoryToHex(const Byte* memory, size_t size);
    static std::string memoryToString(const Byte* memory, size_t size);
};

#endif
