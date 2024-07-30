#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QtWidgets/QWidget>
//#include "ui_TcpClientPro.h"
#include <QTcpSocket>
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
    void initClient();
    void readFileData();

private slots:
    void on_connectBtn_clicked();
    void on_selectBtn_clicked();
    void on_sendBtn_clicked();
    void connectedSlot();
    void disconnectedSlot();
    void readyReadSlot();

private:
    Ui::Widget *ui;
    QTcpSocket* m_client;
        QString m_fileName;
        int m_readNum, m_lastSize;
        int m_readIndex;
        QFile m_file;
};
#endif // WIDGET_H
