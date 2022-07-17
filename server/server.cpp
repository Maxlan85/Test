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
    QString ipItem = "(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp RegExp ("^" + ipItem + "\\." + ipItem + "\\." + ipItem + "\\." + ipItem + "$");
    ui->hostLineEdit->setValidator(new QRegExpValidator(RegExp));
    QString ipItem2 = "(0|6[0-4][0-9][0-9][0-9]|65[0-4][0-9][0-9]|655[0-2][0-9]|6553[0-5]|[1-5][0-9][0-9][0-9][0-9]|[7-9][0-9][0-9][0-9]|6[0-9][0-9][0-9])";
    QRegExp RegExp2 ("^" + ipItem2 + "$");
    ui->portLineEdit->setValidator(new QRegExpValidator(RegExp2));
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
   if (!tcp_server.listen(QHostAddress(ui->hostLineEdit->text()), ui->portLineEdit->text().toInt()))
   {
        qDebug() << tcp_server.errorString();
        QMessageBox::warning(this, "Внимание","Адрес или порт недоступны");
        wrong_adress=true;
        return;
    }
   else
   {
    wrong_adress=false;
    all_bytes = 0;
    bytes_received = 0;
    image_size = 0;
    ui->serverStatusLabel->setText(tr("Ожидание клиента"));
    ui->portLineEdit->setEnabled(true);
   }
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
    QString hostLineEdit_text = ui->hostLineEdit->text();
    bool end_of_hostLineEdit_text=false;
    for(int i=0;i<hostLineEdit_text.size();i++)
    {
        if(hostLineEdit_text[hostLineEdit_text.size()-1]==".")
            end_of_hostLineEdit_text=true;
    }
    if(ui->hostLineEdit->text().size()<7 || end_of_hostLineEdit_text==true)
    {
        QMessageBox::warning(this, "Внимание","Неверный формат ip,введите все числа");
    }
    else if(ui->hostLineEdit->text()==""||ui->portLineEdit->text()=="")
    {
        QMessageBox::warning(this, "Внимание","Строка ip или порта не заполнена");
    }
    else
    {
    if(ui->startButton->text() == tr("Старт"))
    {

        start();
        if(wrong_adress==false)
        {
        ui->portLineEdit->setEnabled(false);
        if(is_ok==false)
        ui->startButton->setText(tr("Запустите клиент"));
        else
        {
            ui->startButton->setText(tr("Стоп"));
        }
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
