#include <QVBoxLayout>
#include <QFile>
#include <QtUiTools/QUiLoader>
#include "ui/ProcessDialog.hpp"

ProcessDialog::ProcessDialog(QWidget* parent) : QDialog(parent) {
    QUiLoader loader;
    QFile file("ui/process.ui");
    file.open(QFile::ReadOnly);
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
    for (const auto& p : processes) {
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
