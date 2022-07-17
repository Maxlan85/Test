#include "client.h"
#include "ui_client.h"
#include <QtNetwork>
#include <QFileDialog>
#include <qvalidator.h>
#include <qmessagebox.h>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->hostLineEdit->setPlaceholderText(tr("Пример ввода:127.0.0.1"));
    ui->portLineEdit->setPlaceholderText(tr("Пример ввода:6666"));
    QString ipItem = "(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp RegExp ("^" + ipItem + "\\." + ipItem + "\\." + ipItem + "\\." + ipItem + "$");
    ui->hostLineEdit->setValidator(new QRegExpValidator(RegExp));
    QString ipItem2 = "(0|6[0-4][0-9][0-9][0-9]|65[0-4][0-9][0-9]|655[0-2][0-9]|6553[0-5]|[1-5][0-9][0-9][0-9][0-9]|[7-9][0-9][0-9][0-9]|6[0-9][0-9][0-9])";
    QRegExp RegExp2 ("^" + ipItem2 + "$");
    ui->portLineEdit->setValidator(new QRegExpValidator(RegExp2));
    all_bytes = 0;
    is_ok = false;

    ui->sendButton->setEnabled(false);
    ui->openButton->setEnabled(false);
    tcp_client = new QTcpSocket(this);

    connect(tcp_client, SIGNAL(connected()), this, SLOT(client_connected()));

    connect(this, SIGNAL(buildConnected()), this, SLOT(start_send()));

    connect(tcp_client, SIGNAL(disconnected()), this, SLOT(client_disconnected()));

    connect(tcp_client, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(display_error(QAbstractSocket::SocketError)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::open_file()
{
    file_name = QFileDialog::getOpenFileName(this,tr("Выберите файл"),QDir::currentPath(),"*.jpg *.bmp *.png");
    if (!file_name.isEmpty())
    {
        image_name = file_name.right(file_name.size() - file_name.lastIndexOf('/')-1);
        ui->clientStatusLabel->setText(tr("Открытие %1 успешно！").arg(image_name));
        if(is_ok == true)
        {
            ui->sendButton->setEnabled(true);
        }
    }
}

void Widget::send()
{
    if(!is_ok)
    {
        ui->clientStatusLabel->setText(tr("Пожалуйста, сначала подключитесь к серверу"));
        return;
    }
    else
    {
        emit buildConnected();
        qDebug() << "emit buildConnected()" << endl;
    }
}

void Widget::connect_server()
{
    ui->clientStatusLabel->setText(tr("Соединение"));
    tcp_client->connectToHost(ui->hostLineEdit->text(),ui->portLineEdit->text().toInt());
    is_ok = true;
    qDebug() << "connectServer: isOk is ok" << endl;
}


void Widget::start_send()
{
    QDataStream sendOut(&bytes_send, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);
    QImage image(file_name);
    QString imageData = get_image_data(image);
    sendOut << qint64(0) << qint64(0) << imageData;
    all_bytes += bytes_send.size();
    sendOut.device()->seek(0);
    sendOut << all_bytes << qint64((bytes_send.size() - sizeof(qint64)*2));
    tcp_client->write(bytes_send);
    bytes_send.resize(0);
    ui->clientStatusLabel->setText(tr("Передача %1 успешна").arg(image_name));
    all_bytes = 0;
}

void Widget::display_error(QAbstractSocket::SocketError)
{
    qDebug() << tcp_client->errorString();
    tcp_client->close();
    ui->clientStatusLabel->setText(tr("Не найден сервер"));
}

void Widget::client_connected()
{
    is_ok = true;
    ui->connectButton->setText(tr("Отключение от \nсервера"));
    ui->openButton->setEnabled(true);
    ui->portLineEdit->setEnabled(false);
    ui->hostLineEdit->setEnabled(false);
    ui->clientStatusLabel->setText(tr("Подключено"));
}

void Widget::client_disconnected()
{
    is_ok = false;
    tcp_client->abort();
    ui->connectButton->setText(tr("Соединение с \nсервером"));
    ui->sendButton->setEnabled(false);
    ui->openButton->setEnabled(false);
    ui->portLineEdit->setEnabled(true);
    ui->hostLineEdit->setEnabled(true);
    ui->clientStatusLabel->setText(tr("Нет соединения"));
}

QByteArray Widget::get_image_data(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    image.save(&buffer, "png");
    imageData = imageData.toBase64();
    return imageData;
}


void Widget::on_openButton_clicked()
{
    ui->clientStatusLabel->setText(tr("Статус: Ожидание открытия файла!"));
    open_file();
}


void Widget::on_sendButton_clicked()
{
    send();
}

void Widget::on_connectButton_clicked()
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
    if (ui->connectButton->text() == tr("Соединение с \nсервером"))
    {
        tcp_client->abort();
        connect_server();
    }
    else
    {
        tcp_client->abort();
    }
    }
}

