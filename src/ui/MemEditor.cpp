#include <QVBoxLayout>
#include <QFile>
#include <QtUiTools/QUiLoader>
#include <QMessageBox>
#include <QFont>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QKeyEvent>
#include <cctype>

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
        QTextCursor cursor = memArea_->textCursor();
        int pos = cursor.position();

        if ((key >= Qt::Key_0 && key <= Qt::Key_9) || (key >= Qt::Key_A && key <= Qt::Key_F)) {
            if (pos < (int)memHex_.size() && !std::isspace(memHex_[pos])) {
                char ch = (key >= Qt::Key_A) ? (std::tolower((char)key)) : ((char)key);
                writeToProcessMemory(pos, ch);

                // Update the UI character directly
                cursor.deleteChar();
                cursor.insertText(QString(ch));

                // Move to next hex char, skipping whitespace
                pos = cursor.position();
                while (pos < (int)memHex_.size() && std::isspace(memHex_[pos])) {
                    pos++;
                }
                cursor.setPosition(pos);
                memArea_->setTextCursor(cursor);
                return true;
            }
        } else if (key == Qt::Key_Right) {
            int next = pos + 1;
            while (next < (int)memHex_.size() && std::isspace(memHex_[next])) {
                next++;
            }
            if (next < (int)memHex_.size()) {
                cursor.setPosition(next);
                memArea_->setTextCursor(cursor);
                return true;
            }
        } else if (key == Qt::Key_Left) {
            int prev = pos - 1;
            while (prev >= 0 && std::isspace(memHex_[prev])) {
                prev--;
            }
            if (prev >= 0) {
                cursor.setPosition(prev);
                memArea_->setTextCursor(cursor);
                return true;
            }
        } else if (key == Qt::Key_Up) {
            if (cursor.blockNumber() <= 1) {
                if (baseAddress_ >= 16) {
                    baseAddress_ -= 16;
                    lastCursorPos_ = pos;
                    refresh();
                    return true;
                }
            }
        } else if (key == Qt::Key_Down) {
            int currentBlock = cursor.blockNumber();
            int totalBlocks = memArea_->document()->blockCount();
            if (cursor.blockNumber() >= totalBlocks - 3) { // second last line
                baseAddress_ += 16;
                lastCursorPos_ = pos;
                refresh();
                return true;
            }
        } else if (key == Qt::Key_Backspace || key == Qt::Key_Delete) {
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MemEditor::writeToProcessMemory(int position, char ch) {
    int byteOffset = (position / 49) * 16 + (position % 49) / 3;
    int startOfByteInHex = (byteOffset / 16) * 49 + (byteOffset % 16) * 3;

    std::string hexByte = memHex_.substr(startOfByteInHex, 2);
    hexByte[position - startOfByteInHex] = ch;

    Address addr = baseAddress_ + byteOffset;

    QMetaObject::invokeMethod(mainWindow_->getWorker(), "writeMemory", Qt::QueuedConnection,
                              Q_ARG(Address, addr),
                              Q_ARG(QString, QString::fromStdString(std::to_string(MedUtil::hexToInt(hexByte)))),
                              Q_ARG(ScanType, ScanType::Int8));

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
    if (memArea_) {
        memArea_->setPlainText(QString::fromStdString(memHex_));
        if (lastCursorPos_ != -1) {
            QTextCursor cursor = memArea_->textCursor();
            if (lastCursorPos_ < (int)memHex_.size()) {
                cursor.setPosition(lastCursorPos_);
                memArea_->setTextCursor(cursor);
            }
            lastCursorPos_ = -1;
        }
    }
    if (textArea_) textArea_->setPlainText(QString::fromStdString(memoryToString(data.getBytes(), data.getSize())));

    loadAddresses(addr);
    onMemAreaCursorPositionChanged();
}

void MemEditor::onMemAreaCursorPositionChanged() {
    if (!memArea_ || !rawMemory_ || memHex_.empty()) return;
    if (isInsideCursorPositionChanged_) return;
    isInsideCursorPositionChanged_ = true;

    int pos = memArea_->textCursor().position();
    if (pos >= (int)memHex_.size()) {
        isInsideCursorPositionChanged_ = false;
        return;
    }

    // Fix position if it's on a whitespace
    if (std::isspace(memHex_[pos])) {
        int next = pos;
        while (next < (int)memHex_.size() && std::isspace(memHex_[next])) {
            next++;
        }
        if (next < (int)memHex_.size()) {
            QTextCursor cursor = memArea_->textCursor();
            cursor.setPosition(next);
            memArea_->setTextCursor(cursor);
            isInsideCursorPositionChanged_ = false;
            return;
        }
    }

    int byteOffset = (pos / 49) * 16 + (pos % 49) / 3;
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
    int start = (byteOffset / 16) * 49 + (byteOffset % 16) * 3;

    QTextCharFormat normalFormat;
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setBackground(Qt::yellow);

    int currentPos = cursor.position();
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(normalFormat);

    cursor.setPosition(start);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
    cursor.setCharFormat(boldFormat);

    // Restore original cursor position (without selection)
    cursor.setPosition(currentPos);
    memArea_->setTextCursor(cursor);

    isInsideCursorPositionChanged_ = false;
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
