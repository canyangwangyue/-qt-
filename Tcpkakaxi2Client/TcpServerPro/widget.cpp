#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Server");
        iniServer();
}

Widget::~Widget()
{
    //delete ui;
    closeServer();
        if (m_socket != NULL) {
            delete m_socket;
            m_socket = NULL;
        }
}
void Widget::iniServer() {
    m_fileName = "";
    m_readCnt = 0;
    m_lastSize = 0;
    m_socket = NULL;
    ui->clearBtn->setEnabled(false);
    m_server = new QTcpServer(this);
}
void Widget::closeServer() {
    m_server->close();
    if (m_socket == NULL) {
        return;
    }
    //断开与客户端的连接
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->abort();
        }
    }
}
void Widget::on_listenBtn_clicked() {
    if (m_server->isListening()) {
        closeServer();
        //关闭server后恢复界面状态
        ui->listenBtn->setText("Listen");
        ui->addressEt->setEnabled(true);
        ui->portEt->setEnabled(true);
    }
    else {
        //可以使用 QHostAddress::Any 监听所有地址的对应端口
        const QString address_text = ui->addressEt->text();
        const unsigned short port = ui->portEt->text().toUShort();
        const QHostAddress address = (address_text == "Any")
            ? QHostAddress::Any
            : QHostAddress(address_text);
        //开始监听，并判断是否成功
        if (m_server->listen(address, port)) {
            //连接成功就修改界面按钮提示，以及地址栏不可编辑
            ui->listenBtn->setText("Close");
            ui->addressEt->setEnabled(false);
            ui->portEt->setEnabled(false);
        }
        connect(m_server, &QTcpServer::newConnection, this, &Widget::newConnectionSlot);
    }
}
void Widget::on_clearBtn_clicked() {
    ui->receiveEt->clear();
}
void Widget::newConnectionSlot() {
    if (m_server->hasPendingConnections())
    {
        //nextPendingConnection返回下一个挂起的连接作为已连接的QTcpSocket对象
        //套接字是作为服务器的子级创建的，这意味着销毁QTcpServer对象时会自动删除该套接字。
        m_socket = m_server->nextPendingConnection();
        ui->receiveEt->append("connected.........");
        connect(m_socket, &QTcpSocket::readyRead, this, &Widget::readyReadSlot);
        ui->clearBtn->setEnabled(true);
    }
}
void Widget::readyReadSlot() {
    if (m_socket->bytesAvailable() <= 0)
        return;
    FileDate fdata;
    QByteArray array;
    //array.resize(sizeof(fdata));
    array = m_socket->readAll();
    memcpy(&fdata, array.data(), sizeof(fdata));//转化到结构体
    if (fdata.type == TransFileInfo) {
        parseFileData(fdata);
    }
    else if (fdata.type == TransFileData) {
        writeFile(fdata);
    }
}
void Widget::writeFile(FileDate& fdata) {
    m_readIndex++;
    ui->progressBar->setValue(m_readIndex);
    if (m_readIndex == m_readCnt)//最后一个包
    {
        m_file.write(fdata.data, m_lastSize);
        m_readIndex = 0;
        m_file.close();//传输完毕
        qDebug() << "transfer over------------------------";
        return;
    }
    else {
        m_file.write(fdata.data, sizeof(fdata.data));
        QByteArray array1;
        array1.resize(sizeof(fdata));
        FileDate buf;
        buf.state = CorrectState;
        buf.type = TransFileData;
        memcpy(array1.data(), &buf, sizeof(buf));
        qDebug() << "transfer progress-------" << double(1.0*m_readIndex / m_readCnt) * 100<<"%";
        m_socket->write(array1);
        m_socket->waitForBytesWritten(1000);
    }
}
void Widget::parseFileData(FileDate &fdata) {
    QString dirname = QFileDialog::getExistingDirectory(this, "SelectDirectory", "/");
    QString filename(fdata.fileName);
    //qDebug() << "dirname: " << dirname;
    QString filePath = dirname.append("/").append(filename);
    QFileInfo info(filePath);
    m_fileName = info.fileName();
    m_readCnt = fdata.readCnt;
    m_lastSize = fdata.lastSize;
    qDebug() << "TcpServerPro::parseFileData m_readCnt: " << m_readCnt << " m_lastSize: " << m_lastSize;
    m_file.setFileName(filePath);
    FileDate fdata1;
    if (m_file.open(QIODevice::WriteOnly)) {
        fdata1.type = TransFileInfoDone;
        fdata1.state = CorrectState;
        ui->progressBar->setMaximum(m_readCnt);
        ui->progressBar->setValue(0);
    }
    else {
        fdata1.type = TransFileData;
        fdata1.state = ErrorState;
    }
    m_readIndex = 0;
    QByteArray array1;
    array1.resize(sizeof(fdata1));
    memcpy(array1.data(), &fdata1 , sizeof(fdata1));
    m_socket->write(array1);
    m_socket->waitForBytesWritten();
}

