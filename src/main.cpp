#include "ui/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    qunsetenv("QT_LOGGING_RULES");

    QThreadPool::globalInstance()->setMaxThreadCount(12);
    
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
