#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTcpSocket>
#include <QDataStream>
#include <QPointer>
#include <QFileInfo>
#include <QStyleFactory>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
                                          m_clientSocket(nullptr),
                                          m_totalFileSize(0),
                                          m_receivedBytes(0)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle(QApplication::applicationName());

// 判断当前系统类型
#if defined(Q_OS_ANDROID)
    ui->label_2->setText(QStringLiteral("当前系统：Android"));
    m_savePath = "/storage/emulated/0/DCIM/Camera/";
#elif defined(Q_OS_WIN)
    ui->label_2->setText(QStringLiteral("当前系统：Windows"));
#elif defined(Q_OS_MACOS)
    ui->label_2->setText(QStringLiteral("当前系统：macOS"));
#elif defined(Q_OS_LINUX)
    ui->label_2->setText(QStringLiteral("当前系统：Linux"));
#else
    ui->label_2->setText(QStringLiteral("当前系统：未知"));
#endif

    // 设置应用程序样式
    qApp->setStyle(QStyleFactory::create("fusion"));

    // 启动服务器监听
    if (m_tcpServer.listen(QHostAddress::Any, 5000))
    {
        ui->statusBar->showMessage(QStringLiteral("状态：正在监听！"));
    }
    else
    {
        ui->statusBar->showMessage(QStringLiteral("状态：监听失败！"));
    }
    
    // 显示监听端口
    ui->labelListenPort->setText(QString::number(m_tcpServer.serverPort()));

    // 连接服务器新连接信号到槽函数
    connect(&m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::onClientConnected);

    // 连接取消按钮信号到关闭窗口槽函数
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClientConnected()
{
    // 获取新的客户端连接
    m_clientSocket = m_tcpServer.nextPendingConnection();

    // 连接断开信号到删除对象槽函数
    connect(m_clientSocket, &QTcpSocket::disconnected, m_clientSocket, &QTcpSocket::deleteLater);

    // 连接接收数据信号到槽函数
    connect(m_clientSocket, &QIODevice::readyRead, this, &MainWindow::onDataReceived);

    // 连接错误信号到错误处理槽函数
    connect(m_clientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketErrorOccurred(QAbstractSocket::SocketError)));

    // 设置数据流的设备为客户端套接字
    m_dataStream.setDevice(m_clientSocket);
    m_dataStream.setVersion(QDataStream::Qt_5_0);
}

void MainWindow::onDataReceived()
{
    while (m_clientSocket->bytesAvailable())
    {
        // 如果文件大小尚未获取，且可用数据大于文件大小字段
        if (0 == m_totalFileSize && m_clientSocket->bytesAvailable() > sizeof(qint64))
        {
            // 读取文件大小和文件名
            m_dataStream >> m_totalFileSize >> m_receivedFileName;


            // 关闭当前文件
            m_receivedFile.close();

            // 设置新文件名
            rec_file_path = m_savePath + m_receivedFileName;
            m_receivedFile.setFileName(rec_file_path);

            // 打开文件，若失败则输出错误
            if (!m_receivedFile.open(QIODevice::WriteOnly))
            {
                qCritical() << m_receivedFile.errorString();
                return;
            }
            ui->plainTextEditLog->appendPlainText(QStringLiteral("正在接收【%1】 ...").arg(m_receivedFileName));
        }
        else
        {
            // 计算要读取的字节数
            qint64 size = qMin(m_clientSocket->bytesAvailable(), m_totalFileSize - m_receivedBytes);
            if (size == 0)
            {
                resetTransferState();
                continue;
            }

            // 读取数据并写入文件
            QByteArray data(size, 0);
            m_dataStream.readRawData(data.data(), size);
            m_receivedFile.write(data);

            // 更新已接收字节数
            m_receivedBytes += size;

            // 如果已接收字节数等于文件总大小，表示接收完成
            if (m_receivedBytes == m_totalFileSize)
            {
                QFileInfo fileInfo(rec_file_path);
                qDebug() << "接收文件路径：" << rec_file_path;
                ui->plainTextEditLog->appendPlainText(QStringLiteral("成功接收【%1】 -> %2").arg(m_receivedFileName).arg(fileInfo.absoluteFilePath()));
                resetTransferState();
            }
        }
    }
}

void MainWindow::resetTransferState()
{
    // 重置传输状态
    m_receivedFile.close();
    m_receivedFileName.clear();
    m_totalFileSize = 0;
    m_receivedBytes = 0;
}

void MainWindow::onSocketErrorOccurred(QAbstractSocket::SocketError error)
{
    // 根据不同的错误类型输出错误信息
    switch (error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(m_clientSocket->errorString()));
        qDebug() << __FUNCTION__ << "QAbstractSocket::ConnectionRefusedError";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        qDebug() << __FUNCTION__ << "QAbstractSocket::RemoteHostClosedError";
        ui->plainTextEditLog->appendPlainText(QStringLiteral("文件传输终止！"));
        resetTransferState();
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(m_clientSocket->errorString()));
        qDebug() << __FUNCTION__ << "QAbstractSocket::HostNotFoundError";
        break;
    case QAbstractSocket::SocketTimeoutError:
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(m_clientSocket->errorString()));
        qDebug() << __FUNCTION__ << "QAbstractSocket::SocketTimeoutError";
        break;
    case QAbstractSocket::AddressInUseError:
        qDebug() << __FUNCTION__ << "QAbstractSocket::AddressInUseError";
        break;
    default:
        break;
    }
}
