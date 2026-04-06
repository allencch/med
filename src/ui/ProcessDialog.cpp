#include <QVBoxLayout>
#include <QFile>
#include <QKeyEvent>
#include <QtUiTools/QUiLoader>
#include "ui/ProcessDialog.hpp"

ProcessDialog::ProcessDialog(QWidget* parent) : QDialog(parent) {
    QUiLoader loader;
    QFile file(":/ui/process.ui");
    if (!file.open(QFile::ReadOnly)) {
        // Handle the error, e.g., log it or show a message box
        qWarning() << "Could not open file for reading:" << file.errorString();
        return;
    }
    QWidget* widget = loader.load(&file, this);
    file.close();

    treeWidget_ = widget->findChild<QTreeWidget*>("processTreeWidget");
    if (treeWidget_) {
        connect(treeWidget_, &QTreeWidget::itemDoubleClicked, this, &ProcessDialog::onDoubleClicked);
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(widget);
    setLayout(layout);
    setWindowTitle("Select Process");
    resize(400, 500);
}

void ProcessDialog::setProcessList(const std::vector<Process>& processes) {
    if (!treeWidget_) return;
    treeWidget_->clear();
    for (auto it = processes.rbegin(); it != processes.rend(); ++it) {
        const auto& p = *it;
        auto* item = new QTreeWidgetItem(treeWidget_);
        item->setText(0, QString::number(p.getPid()));
        item->setText(1, QString::fromStdString(p.getCmdline()));
    }
}

void ProcessDialog::onDoubleClicked(QTreeWidgetItem* item, int) {
    pid_t pid = item->text(0).toInt();
    QString name = item->text(1);
    emit processSelected(pid, name);
    accept();
}

void ProcessDialog::keyPressEvent(QKeyEvent* event) {
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && treeWidget_) {
        QTreeWidgetItem* item = treeWidget_->currentItem();
        if (item) {
            onDoubleClicked(item, 0);
            return;
        }
    }
    QDialog::keyPressEvent(event);
}
