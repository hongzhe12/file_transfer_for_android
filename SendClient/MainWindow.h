#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcombobox.h"
#include <QMainWindow>
#include <QFile>
#include <QTcpSocket>
#include <QDataStream>
#include <QElapsedTimer>
#include <QQueue>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void sendFile();    //点击发送按钮
    void addFile();     //点击文件添加按钮
    void delFile();     //点击删除按钮
    void clearFile();   //点击清空按钮

    void onSocketStateChanged(QAbstractSocket::SocketState state);
    void onSocketError(QAbstractSocket::SocketError error);
    void onBytesWritten(const qint64 &bytes);

    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    void send();        //发送文件，文件会自动从队列获取
    void reset();       //重置部分成员变量
    void updateProgress(const int &size);   //更新进度
    void populateComboBox(QComboBox *comboBox); // 加载IP列表

private:
    Ui::MainWindow *ui;

    QFile m_file;
    qint64 m_currentFileSize;   //当前文件大小
    qint64 m_totalFileSize;     //所有文件大小

    QTcpSocket m_socket;
    QDataStream m_outStream;

    qint64 m_totalFileBytesWritten;     //所有文件已写入字节数
    qint64 m_currentFileBytesWritten;   //当前文件已写入字节数
    qint64 m_blockSize;                 //每次读取文件数据块大小

    QQueue<QString> m_fileQueue;    //文件队列
    QElapsedTimer m_timer;          //计时器


};

#endif // MAINWINDOW_H
