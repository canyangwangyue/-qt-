#include "TcpServer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServerPro w;
    w.show();
    return a.exec();
}
