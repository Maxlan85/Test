#include "server.h"
#include "ui_server.h"
#include <QtNetwork>
#include <qmessagebox.h>
#include <qvalidator.h>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->portLineEdit->setPlaceholderText(tr("Пример ввода:6666"));
    ui->portLineEdit->setValidator(new QIntValidator);
    ui->portLineEdit->setValidator(new QRegExpValidator(QRegExp("0|[1-9]\\d{0,4}"),this));
    is_ok=false;
    connect(&tcp_server, SIGNAL(newConnection()),this, SLOT(accept_connection()));
    ui->imageLabel->show();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::start()
{
   if (!tcp_server.listen(QHostAddress::LocalHost, ui->portLineEdit->text().toInt()))
   {
        qDebug() << tcp_server.errorString();
        close();
        return;
    }
    all_bytes = 0;
    bytes_received = 0;
    image_size = 0;
    ui->serverStatusLabel->setText(tr("Ожидание клиента"));
    ui->portLineEdit->setEnabled(true);
}

void Widget::accept_connection()
{
    tcp_server_connection = tcp_server.nextPendingConnection();

    connect(tcp_server_connection, SIGNAL(readyRead()),this, SLOT(update_server_progress()));

    connect(tcp_server_connection, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(display_error(QAbstractSocket::SocketError)));

    ui->serverStatusLabel->setText(tr("Подключено"));
    is_ok==true;
    ui->startButton->setText(tr("Стоп"));
    ui->portLineEdit->setEnabled(false);
}


void Widget::update_server_progress()
{
    QDataStream in(tcp_server_connection);
    in.setVersion(QDataStream::Qt_5_6);

    if (bytes_received <= sizeof(qint64)*2)
    {
        if((tcp_server_connection->bytesAvailable() >= sizeof(qint64)*2) && (image_size == 0))
        {

            in >> all_bytes  >> image_size;
            bytes_received += sizeof(qint64) * 2;

            if(image_size == 0)
            {
                  ui->serverStatusLabel->setText(tr("Нет изображения!"));
            }
        }
        if((tcp_server_connection->bytesAvailable() >= image_size) && (image_size != 0))
        {

            in >> image;

            ui->serverStatusLabel->setText(tr("Получение изображения"));

            QImage imageData = get_image(image);

            QPixmap resImage = QPixmap::fromImage(imageData);
            QPixmap* imgPointer = &resImage;
            imgPointer->scaled(ui->imageLabel->size(), Qt::IgnoreAspectRatio);

            ui->imageLabel->setScaledContents(true);
            ui->imageLabel->setPixmap(*imgPointer);

            bytes_received += image_size;

            if(bytes_received == all_bytes)
            {
                 ui->serverStatusLabel->setText(tr("Успешно получен файл"));
                 all_bytes = 0;
                 bytes_received = 0;
                 image_size = 0;
            }

         }
     }
}

void Widget::display_error(QAbstractSocket::SocketError socketError)
{
    qDebug() <<"errorString()" <<tcp_server_connection->errorString();
    tcp_server.close();
    tcp_server_connection->disconnectFromHost();
    ui->serverStatusLabel->setText(tr("Нет связи"));
    ui->portLineEdit->setEnabled(true);
    ui->startButton->setText(tr("Старт"));

}

QImage Widget::get_image(const QString &data)
{
    QByteArray imageData = QByteArray::fromBase64(data.toLatin1());
    QImage image;
    image.loadFromData(imageData);
    return image;
}


void Widget::on_startButton_clicked()
{
    if(ui->portLineEdit->text()=="")
    {
        QMessageBox::warning(this, "Внимание","Поле порта пустое");
    }
    else
    {
    if(ui->startButton->text() == tr("Старт"))
    {

        start();
        ui->portLineEdit->setEnabled(false);
        if(is_ok==false)
        ui->startButton->setText(tr("Запустите клиент"));
        else
        {
            ui->startButton->setText(tr("Стоп"));
        }
    }
    else if(ui->startButton->text() == tr("Запустите клиент"))
    {
        if(is_ok==false)
        {
            QMessageBox::warning(this, "Внимание","Присоединитесь клиентом к серверу");
        }
    }
    else if(ui->startButton->text() == tr("Стоп"))
    {
        ui->startButton->setText(tr("Старт"));
        tcp_server.close();
        tcp_server_connection->disconnectFromHost();
        is_ok=false;
        ui->serverStatusLabel->setText(tr("Отключено"));
        ui->portLineEdit->setEnabled(true);
    }
    }
}
