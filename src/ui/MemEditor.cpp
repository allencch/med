#include <QVBoxLayout>
#include <QFile>
#include <QtUiTools/QUiLoader>
#include <QMessageBox>
#include <QFont>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QKeyEvent>

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
    font.setStyleHint(QFont::Monospace);
    font.setPixelSize(14);

    if (memArea_) {
      memArea_->setFont(font);
      memArea_->setTextInteractionFlags(memArea_->textInteractionFlags() | Qt::TextSelectableByKeyboard);
    }
    if (addrArea_) addrArea_->setFont(font);
    if (textArea_) textArea_->setFont(font);

    if (refreshButton_) connect(refreshButton_, &QPushButton::clicked, this, &MemEditor::onRefreshButtonClicked);
    if (enterButton_) connect(enterButton_, &QPushButton::clicked, this, &MemEditor::onEnterClicked);
    if (valueEdit_) connect(valueEdit_, &QLineEdit::returnPressed, this, &MemEditor::onEnterClicked);
    if (currAddressEdit_) connect(currAddressEdit_, &QLineEdit::editingFinished, this, &MemEditor::onCurrAddressEdited);
    if (memArea_) {
        connect(memArea_, &QPlainTextEdit::cursorPositionChanged, this, &MemEditor::onMemAreaCursorPositionChanged);
        memArea_->installEventFilter(this);
    }
}

void MemEditor::setBaseAddress(Address addr) {
    baseAddress_ = MedUtil::addressRoundDown(addr);
    if (currAddressEdit_) currAddressEdit_->setText(QString::fromStdString(MedUtil::intToHex(addr)));
    refresh();
}

void MemEditor::refresh() {
    if (baseAddress_ == 0 || mainWindow_->getPid() == 0) return;

    // Request memory from worker via mainWindow
    QMetaObject::invokeMethod(mainWindow_->getWorker(), "requestMemory", Qt::QueuedConnection,
                              Q_ARG(Address, baseAddress_),
                              Q_ARG(size_t, (size_t)MEMORY_SIZE));
}

void MemEditor::onRefreshButtonClicked() {
    refresh();
}

bool MemEditor::eventFilter(QObject* obj, QEvent* event) {
    if (obj == memArea_ && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();
        if ((key >= Qt::Key_0 && key <= Qt::Key_9) || (key >= Qt::Key_A && key <= Qt::Key_F)) {
            int pos = memArea_->textCursor().position();
            if (pos < (int)memHex_.size() && !std::isspace(memHex_[pos])) {
                writeToProcessMemory(pos, (char)key);

                // Move to next hex char
                QTextCursor cursor = memArea_->textCursor();
                int next = pos + 1;
                if (next < (int)memHex_.size() && std::isspace(memHex_[next])) next++;
                cursor.setPosition(next);
                memArea_->setTextCursor(cursor);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MemEditor::writeToProcessMemory(int position, char ch) {
    // Get the 2-char hex string for the byte
    int byteStart = (position / 3) * 3;
    std::string hexByte = memHex_.substr(byteStart, 2);

    // Update the character
    hexByte[position % 3] = ch;

    Address addr = baseAddress_ + (position / 3);

    // Write as int8 (1 byte)
    QMetaObject::invokeMethod(mainWindow_->getWorker(), "writeMemory", Qt::QueuedConnection,
                              Q_ARG(Address, addr),
                              Q_ARG(QString, QString::fromStdString(std::to_string(MedUtil::hexToInt(hexByte)))),
                              Q_ARG(ScanType, ScanType::Int8));

    // We don't refresh immediately to avoid flicker,
    // but the local memHex_ should be updated for the next key press
    memHex_[position] = ch;
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

    QMetaObject::invokeMethod(mainWindow_->getWorker(), "writeMemory", Qt::QueuedConnection,
                              Q_ARG(Address, addr),
                              Q_ARG(QString, val),
                              Q_ARG(ScanType, type));
    refresh();
}

void MemEditor::onMemoryReady(Address addr, const SizedBytes& data) {
    if (addr != baseAddress_) return;

    rawMemorySize_ = data.getSize();
    rawMemory_ = data.getBytePtr(); // share the pointer

    memHex_ = memoryToHex(data.getBytes(), data.getSize());
    if (memArea_) memArea_->setPlainText(QString::fromStdString(memHex_));
    if (textArea_) textArea_->setPlainText(QString::fromStdString(memoryToString(data.getBytes(), data.getSize())));

    loadAddresses(addr);
    onMemAreaCursorPositionChanged();
}

void MemEditor::onMemAreaCursorPositionChanged() {
    if (!memArea_ || !rawMemory_ || memHex_.empty()) return;

    int pos = memArea_->textCursor().position();
    if (pos >= (int)memHex_.size()) return;

    // Each byte is "XX " (3 chars)
    int byteOffset = pos / 3;
    Address currAddr = baseAddress_ + byteOffset;

    if (currAddressEdit_) {
        currAddressEdit_->setText(QString::fromStdString(MedUtil::intToHex(currAddr)));
    }

    // Update value views based on the selected byte offset
    if (byteOffset < (int)rawMemorySize_) {
        const Byte* ptr = rawMemory_.get() + byteOffset;
        size_t remaining = rawMemorySize_ - byteOffset;

        if (valueEdit_) {
            ScanType type = MedUtil::stringToScanType(scanTypeCombo_->currentText().toStdString());
            if (remaining >= MedUtil::scanTypeToSize(type)) {
                valueEdit_->setText(QString::fromStdString(MemOperator::toString(ptr, type)));
            } else {
                valueEdit_->setText("??");
            }
        }

        if (viewInt32_) {
            if (remaining >= 4) viewInt32_->setText(QString::fromStdString(MemOperator::toString(ptr, ScanType::Int32)));
            else viewInt32_->setText("??");
        }
        if (viewFloat32_) {
            if (remaining >= 4) viewFloat32_->setText(QString::fromStdString(MemOperator::toString(ptr, ScanType::Float32)));
            else viewFloat32_->setText("??");
        }
        if (viewFloat64_) {
            if (remaining >= 8) viewFloat64_->setText(QString::fromStdString(MemOperator::toString(ptr, ScanType::Float64)));
            else viewFloat64_->setText("??");
        }
    }

    // Bold current byte
    QTextCursor cursor = memArea_->textCursor();
    int start = byteOffset * 3;

    // Simple way to clear bold: reset entire block?
    // Or just re-apply normal to everything and bold one.
    // For performance, maybe just track last bolded.

    QTextCharFormat normalFormat;
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setBackground(Qt::yellow);

    // This is a bit expensive for large text, but MEMORY_SIZE is small (384)
    int currentPos = cursor.position();
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(normalFormat);

    cursor.setPosition(start);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
    cursor.setCharFormat(boldFormat);

    // Restore original cursor position (without selection)
    cursor.setPosition(currentPos);
    memArea_->setTextCursor(cursor);
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
