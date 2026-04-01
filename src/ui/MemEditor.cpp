#include <QVBoxLayout>
#include <QFile>
#include <QtUiTools/QUiLoader>
#include <QMessageBox>
#include <QFont>
#include <QTextCursor>
#include <QTextCharFormat>

#include "ui/MemEditor.hpp"
#include "ui/MainWindow.hpp"
#include "med/MedCommon.hpp"
#include "med/MemOperator.hpp"

const int MEMORY_SIZE = 384;

MemEditor::MemEditor(MainWindow* mainWin) : QWidget(nullptr, Qt::Window), mainWindow_(mainWin) {
    setupUi();
}

MemEditor::~MemEditor() {}

void MemEditor::setupUi() {
    QUiLoader loader;
    QFile file("ui/mem-editor.ui");
    if (!file.open(QFile::ReadOnly)) {
        return;
    }
    QWidget* widget = loader.load(&file, this);
    file.close();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(widget);
    setLayout(layout);
    resize(750, 550);
    setWindowTitle("Memory Editor");

    currAddressEdit_ = widget->findChild<QLineEdit*>("currAddress");
    valueEdit_ = widget->findChild<QLineEdit*>("value");
    memArea_ = widget->findChild<QPlainTextEdit*>("memArea");
    addrArea_ = widget->findChild<QPlainTextEdit*>("addrArea");
    textArea_ = widget->findChild<QPlainTextEdit*>("textArea");
    refreshButton_ = widget->findChild<QPushButton*>("refresh");
    scanTypeCombo_ = widget->findChild<QComboBox*>("scanType");
    viewInt32_ = widget->findChild<QLineEdit*>("view_int32");
    viewFloat32_ = widget->findChild<QLineEdit*>("view_float32");
    viewFloat64_ = widget->findChild<QLineEdit*>("view_float64");
    enterButton_ = widget->findChild<QPushButton*>("enterButton");

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    if (memArea_) memArea_->setFont(font);
    if (addrArea_) addrArea_->setFont(font);
    if (textArea_) textArea_->setFont(font);

    if (refreshButton_) connect(refreshButton_, &QPushButton::clicked, this, &MemEditor::onRefreshButtonClicked);
    if (enterButton_) connect(enterButton_, &QPushButton::clicked, this, &MemEditor::onEnterClicked);
    if (currAddressEdit_) connect(currAddressEdit_, &QLineEdit::editingFinished, this, &MemEditor::onCurrAddressEdited);
    if (memArea_) connect(memArea_, &QPlainTextEdit::cursorPositionChanged, this, &MemEditor::onMemAreaCursorPositionChanged);
}

void MemEditor::setBaseAddress(Address addr) {
    baseAddress_ = addr;
    if (currAddressEdit_) currAddressEdit_->setText(QString::fromStdString(MedUtil::intToHex(addr)));
    refresh();
}

void MemEditor::refresh() {
    if (baseAddress_ == 0 || mainWindow_->getPid() == 0) return;
    
    // Request memory from worker via mainWindow
    QMetaObject::invokeMethod(mainWindow_->findChild<MedWorker*>(), "requestMemory", Qt::QueuedConnection,
                              Q_ARG(Address, baseAddress_),
                              Q_ARG(size_t, (size_t)MEMORY_SIZE));
}

void MemEditor::onRefreshButtonClicked() {
    refresh();
}

void MemEditor::onCurrAddressEdited() {
    if (!currAddressEdit_) return;
    try {
        Address addr = MedUtil::hexToInt(currAddressEdit_->text().toStdString());
        setBaseAddress(addr);
    } catch (...) {}
}

void MemEditor::onEnterClicked() {
    if (!valueEdit_ || !currAddressEdit_) return;
    
    Address addr = MedUtil::hexToInt(currAddressEdit_->text().toStdString());
    QString val = valueEdit_->text();
    ScanType type = MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString());
    
    QMetaObject::invokeMethod(mainWindow_->findChild<MedWorker*>(), "writeMemory", Qt::QueuedConnection,
                              Q_ARG(Address, addr),
                              Q_ARG(QString, val),
                              Q_ARG(ScanType, type));
    refresh();
}

void MemEditor::onMemAreaCursorPositionChanged() {
    // Port cursor position logic later
}

void MemEditor::onMemoryReady(Address addr, const SizedBytes& data) {
    if (addr != baseAddress_) return;
    
    if (memArea_) memArea_->setPlainText(QString::fromStdString(memoryToHex(data.getBytes(), data.getSize())));
    if (textArea_) textArea_->setPlainText(QString::fromStdString(memoryToString(data.getBytes(), data.getSize())));
    
    loadAddresses(addr);
    
    // Update view fields (simplified)
    if (viewInt32_) viewInt32_->setText(QString::fromStdString(MemOperator::toString(data.getBytes(), ScanType::Int32)));
    if (viewFloat32_) viewFloat32_->setText(QString::fromStdString(MemOperator::toString(data.getBytes(), ScanType::Float32)));
    if (viewFloat64_) viewFloat64_->setText(QString::fromStdString(MemOperator::toString(data.getBytes(), ScanType::Float64)));
}

void MemEditor::loadAddresses(Address address) {
    if (!addrArea_) return;
    std::string res;
    for (int i = 0; i < MEMORY_SIZE / 16; ++i) {
        res += MedUtil::intToHex(address + i * 16) + "\n";
    }
    addrArea_->setPlainText(QString::fromStdString(res));
}

std::string MemEditor::memoryToHex(const Byte* memory, size_t size) {
    std::string res;
    char buf[4];
    for (size_t i = 0; i < size; ++i) {
        sprintf(buf, "%02x ", memory[i]);
        res += buf;
        if ((i + 1) % 16 == 0) res += "\n";
    }
    return res;
}

std::string MemEditor::memoryToString(const Byte* memory, size_t size) {
    std::string res;
    for (size_t i = 0; i < size; ++i) {
        if (std::isprint(memory[i])) res += (char)memory[i];
        else res += ".";
        if ((i + 1) % 16 == 0) res += "\n";
    }
    return res;
}
