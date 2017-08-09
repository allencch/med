#include <QWidget>
#include <QtUiTools>

#include "gui/MemEditor.hpp"

MemEditor::MemEditor(QWidget* parent, Med* med) : QWidget(parent, Qt::Tool) {
  this->med = med;

  QUiLoader loader;
  QFile file("./mem-editor.ui");
  file.open(QFile::ReadOnly);
  mainChild = loader.load(&file, this);
  file.close();
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(mainChild);
  this->setLayout(layout);
  this->setWindowModality(Qt::WindowModal);
  this->hide();
}
