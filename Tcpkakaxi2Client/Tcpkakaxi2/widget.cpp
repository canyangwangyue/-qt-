#include "widget.h"
#include "ui_widget.h"
#include <QHostAddress>
#include <QDebug>
#include <QFileDialog>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    initClient();
}

Widget::~Widget()
{
    //delete ui;
    qDebug() << "~ClientPro()----------------------------";
        if (m_client->state() == QAbstractSocket::ConnectedState) {
            //如果使用disconnectFromHost()不会重置套接字，isValid还是会为true
            m_client->abort();
        }
}

void Widget::initClient() {
    //创建client对象
    m_client = new QTcpSocket(this);
    ui->selectBtn->setEnabled(false);
    ui->sendBtn->setEnabled(false);
}
void Widget::on_connectBtn_clicked() {
    if (m_client->state() == QAbstractSocket::ConnectedState) {
        //如果使用disconnectFromHost()不会重置套接字，isValid还是会为true
        m_client->abort();
    }
    else if (m_client->state() == QAbstractSocket::UnconnectedState) {
        //从界面上读取ip和端口
        const QHostAddress address = QHostAddress(ui->addressEt->text());
        const unsigned short port = ui->portEt->text().toUShort();
        //连接服务器
        m_client->connectToHost(address, port);
    }
    else {

    }
    connect(m_client, &QTcpSocket::connected, this, &Widget::connectedSlot);
    connect(m_client, &QTcpSocket::disconnected, this, &Widget::disconnectedSlot);
    connect(m_client, &QTcpSocket::readyRead, this, &Widget::readyReadSlot);
}
void Widget::on_selectBtn_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "open", "../");
    if (!filePath.isEmpty()) {
        m_fileName = "";
        QFileInfo info(filePath);
        m_fileName = info.fileName();
        qDebug() << "m_fileName----------" << m_fileName;
        m_file.setFileName(filePath);
        if (m_file.open(QIODevice::ReadOnly)) {

            FileDate fdata;
            int size = m_file.size();
            if (size == 0) {
                m_file.close();
                return;
            }
            m_lastSize = size % sizeof(fdata.data);
            m_readNum = size / sizeof(fdata.data) + (m_lastSize > 0 ? 1 : 0);
            QByteArray array1 = m_fileName.toStdString().c_str();//
            strncpy(fdata.fileName, array1.data(), sizeof(fdata.fileName));//
            //fdata.FileName = m_fileName;
            fdata.readCnt = m_readNum;
            fdata.lastSize = m_lastSize;
            fdata.type = TransFileInfo;
            qDebug() << "sizeof(fdata)-----------------------" << sizeof(fdata);
            QByteArray array;
            array.resize(sizeof(fdata));
            memcpy(array.data(), &fdata, sizeof(fdata));//把结构体存入数组
            m_client->write(array);
            m_client->waitForBytesWritten();
        }
    }
}
void Widget::on_sendBtn_clicked() {

}
void Widget::connectedSlot() {
    ui->connectBtn->setText("Disconnect");
    ui->selectBtn->setEnabled(true);
    ui->addressEt->setEnabled(false);
    ui->portEt->setEnabled(false);
}
void Widget::disconnectedSlot() {
    ui->connectBtn->setText("Connect");
    ui->addressEt->setEnabled(true);
    ui->portEt->setEnabled(true);
    ui->selectBtn->setEnabled(false);
    ui->sendBtn->setEnabled(false);
}
void Widget::readyReadSlot() {
    if (m_client->bytesAvailable() <= 0)
        return;
    FileDate fdata;
    QByteArray array;
    array = m_client->readAll();
    memcpy(&fdata, array.data(), sizeof(fdata));//转化到结构体
    if (fdata.state == CorrectState) {
        if (fdata.type == TransFileInfoDone) {
            m_readIndex = 0;
            readFileData();
        }
        else if (fdata.type == TransFileData) {
            readFileData();
        }
        if (m_readIndex == m_readNum) {
            m_file.close();
        }
    }
    else {
        m_file.close();
    }
}
void Widget::readFileData() {
    QByteArray r_array;
    FileDate r_fdata;
    r_array.resize(sizeof(r_fdata));
    r_fdata.type = TransFileData;
    m_file.read(r_fdata.data, sizeof(r_fdata.data));
    memcpy(r_array.data(), &r_fdata, sizeof(r_fdata));//把结构体存入数组
    m_client->write(r_array);
    m_client->waitForBytesWritten(1000);
    m_readIndex++;
    qDebug() << "TcpClientPro::readFileData----" << m_readIndex;
}
