#include <QApplication>
#include <QMetaType>
#include <vector>
#include <sys/types.h>
#include "ui/MainWindow.hpp"
#include "med/MemScanner.hpp"
#include "ui/MedWorker.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    qRegisterMetaType<std::vector<ScanResult>>();
    qRegisterMetaType<std::vector<WatchedAddress>>();
    qRegisterMetaType<ScanType>();
    qRegisterMetaType<ScanParser::OpType>();
    qRegisterMetaType<pid_t>();

    MainWindow window;
    window.show();
    return app.exec();
}
