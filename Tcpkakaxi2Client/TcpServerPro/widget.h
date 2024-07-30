#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets/QWidget>
//#include "ui_TcpServerPro.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QFileDialog>
#include <QFile>
#define  Maxsize  (1024*10)
enum  TransType {
    TransFileInfo,
    TransFileInfoDone,
    TransFileData
};
enum TransState {
    ErrorState,
    CorrectState

};
struct FileDate {
    //QString FileName;;
    char fileName[128];
    char data[Maxsize];
    TransType type;
    int readCnt;
    int lastSize;
    TransState state;
};
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void iniServer();
        void closeServer();
        void parseFileData(FileDate&);
        void writeFile(FileDate&);

private slots:
    void on_listenBtn_clicked();
    void on_clearBtn_clicked();
    void newConnectionSlot();
    void readyReadSlot();
private:
    Ui::Widget *ui;
    QTcpServer* m_server;
        QTcpSocket* m_socket;
        QString m_fileName;
        int m_readCnt, m_lastSize;
        int m_readIndex;
        QFile m_file;
};
#endif // WIDGET_H
