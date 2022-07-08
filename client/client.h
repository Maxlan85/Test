#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QAbstractSocket>

class QTcpSocket;
class QFile;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    QTcpSocket *tcp_client;
    qint64 all_bytes;
    QString file_name;
    QByteArray bytes_send;
    QString image_name;
    volatile bool is_ok;

private slots:
    void open_file();
    void send();
    void connect_server();
    void start_send();
    void display_error(QAbstractSocket::SocketError);
    void client_connected();
    void client_disconnected();
    QByteArray get_image_data(const QImage&);
    void on_openButton_clicked();
    void on_sendButton_clicked();
    void on_connectButton_clicked();
signals:
    void buildConnected();
};
#endif // WIDGET_H
