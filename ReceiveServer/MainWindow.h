#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QUdpSocket>
#include <QFile>

class QTcpSocket;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onClientConnected();  // 客户端连接时的槽函数
    void onDataReceived();     // 接收到数据时的槽函数
    void onSocketErrorOccurred(QAbstractSocket::SocketError error);  // 套接字错误时的槽函数

private:
    void resetTransferState();  // 重置文件传输状态

private:
    Ui::MainWindow *ui;  // UI 指针

    QFile m_receivedFile;   // 接收的文件
    qint64 m_totalFileSize; // 文件总大小
    QString m_receivedFileName;  // 接收的文件名

    QDataStream m_dataStream;  // 数据流对象
    QTcpServer m_tcpServer;    // TCP 服务器对象

    qint64 m_receivedBytes;    // 已接收的字节数

    QTcpSocket *m_clientSocket;  // 客户端套接字
    QString m_savePath; // 添加保存路径的成员变量
    QString rec_file_path; // 接收文件路径
};

#endif // MAINWINDOW_H
