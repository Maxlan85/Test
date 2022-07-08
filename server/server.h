#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QAbstractSocket>
#include <QTcpServer>

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
    QTcpServer tcp_server;
    QTcpSocket *tcp_server_connection;
    qint64 all_bytes;
    qint64 bytes_received;
    qint64 image_size;//Проверка на получение изображения
    QString image;
    bool is_ok;//Проверка соединения с клиентом

private slots:
    void start();
    void accept_connection();
    void update_server_progress();
    void display_error(QAbstractSocket::SocketError socketError);
    QImage get_image(const QString &);
    void on_startButton_clicked();
};
#endif // WIDGET_H
