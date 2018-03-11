#ifndef ENCODING_MANAGER_HPP
#define ENCODING_MANAGER_HPP

#include <QObject>
#include "med/MedTypes.hpp"

class MedUi; // Forward declaration

class EncodingManager : public QObject {
  Q_OBJECT
public:
  explicit EncodingManager(MedUi* mainUi);
  EncodingType getEncodingType();
  string convertToUtf8(string text);
  string encode(string text);

private slots:
  void onDefaultEncodingTriggered(bool value);
  void onBig5EncodingTriggered(bool value);

private:
  MedUi* mainUi;
  QWidget* mainWindow;
  void setupSubmenu();
  void setupSignals();
  EncodingType encodingType;
};

#endif
