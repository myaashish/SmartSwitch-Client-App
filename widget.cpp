/*
Author: Aashish Parmar (parmaraashish3@gmail.com)

This file communicates with micro controller for Home Automation Project on local network and Internet on Arduino and Raspberry Pi.
*/

#include "widget.h"
#include <QtWidgets>
#include <QtNetwork>
#include <QSplashScreen>

int xval;
QSqlDatabase db;
QVBoxLayout *mainLayout;
QVector<QPushButton*> image;
QPushButton *imgx;

/*
mlayout Constructor to intialize GUI elements to switch across various tools, connecting with database.
*/
mlayout::mlayout(QWidget *parent) : QWidget(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./room.db");
    db.open();
    QSqlQuery qry;
    qry.prepare( "SELECT * FROM number" );
    qry.exec();
    while (qry.next()) {
        xval = qry.value(0).toInt();
    }

    list = new QListWidget;

    list->setViewMode(QListView::IconMode);
    list->setIconSize(QSize(96, 84));
    list->setMovement(QListView::Static);
    list->setMaximumWidth(88);
    list->setSpacing(12);
    list->setCurrentRow(0);

    stack = new QStackedWidget;
    stack->addWidget(new Widget);
    stack->addWidget(new settings);
    stack->addWidget(new value);

    QListWidgetItem *configButton = new QListWidgetItem(list);
    configButton->setIcon(QIcon(":/off.png"));
    configButton->setText(tr("Switch"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *updateButton = new QListWidgetItem(list);
    updateButton->setIcon(QIcon(":/on.png"));
    updateButton->setText(tr("Settings"));
    updateButton->setTextAlignment(Qt::AlignHCenter);
    updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *valButton = new QListWidgetItem(list);
    valButton->setIcon(QIcon(":/off.png"));
    valButton->setText(tr("Names"));
    valButton->setTextAlignment(Qt::AlignHCenter);
    valButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(list);
    horizontalLayout->addWidget(stack, 1);

    quitButton = new QPushButton(tr("&Quit"));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    settingsbtn = new QPushButton;
    settingsbtn->setText("Enable Settings");
    connect(settingsbtn, SIGNAL(clicked()), this, SLOT(leftwidth()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(settingsbtn);
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(quitButton);
    setLayout(mainLayout);
}

/*
mlayout Destructor. Closes the database.
*/
mlayout::~mlayout()
{
    db.close();
}

void mlayout::leftwidth()
{
    if(list->maximumWidth() == 0)
    {
        list->setMaximumWidth(88);
    }
        else
    {
        list->setMaximumWidth(0);
    }
}

/*
Switches between different tools in order to create their GUI elements
*/
void mlayout::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    stack->setCurrentIndex(list->row(current));
}

/*
Widget constructor. It creates buttons for switches filled with their names and their last ON/OFF state.
*/
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    mainLayout = new QVBoxLayout;

    QSqlQuery qry;
    qry.prepare( "CREATE TABLE IF NOT EXISTS names (appnum INTEGER UNIQUE PRIMARY KEY, appname VARCHAR(50))" );
    qry.exec();
    qry.prepare("select appname from names");
    qry.exec();

    QPixmap img(":/off.png");
    QPixmap img1(":/on.png");
    statusLabel = new QLabel(tr("Switch to be changed"));
    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);

    udpSocketX = new QUdpSocket(this);
    udpSocketX->bind(80, QUdpSocket::ShareAddress);

    connect(udpSocketX, SIGNAL(readyRead()),this, SLOT(processPendingDatagrams()));

    master = new QPushButton;
    master->setIcon(img);
    master->setFlat(true);
    master->setText("Master Off");
    mainLayout->addWidget(master);
    connect(master, SIGNAL(clicked()),this,SLOT(masteroff()));

    int temp = xval;

    while (qry.next() && temp)
    {
        imgx = new QPushButton;
        imgx->setIcon(img);
        imgx->setText(qry.value(0).toString());
        imgx->setAccessibleName(QString::number(i+1));
        imgx->setAccessibleDescription("off");
        imgx->setFlat(true);
        mainLayout->addWidget(imgx);
        image << imgx;
        connect(imgx, SIGNAL(clicked()),this,SLOT(broadcastDatagram()));
        --temp;
    }

    udpSocket = new QUdpSocket(this);

    setLayout(mainLayout);

    setWindowTitle(tr("Broadcast Sender"));

    QByteArray datagram = "z";
    udpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress::Broadcast, 80);
}

/*
Sends data to microcontroller to switch ON or OFF a particular switch.
*/
void Widget::broadcastDatagram()
{
    QPixmap img(":/off.png");
    QPixmap img1(":/on.png");

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QByteArray datagram = button->accessibleName().toLatin1();
    udpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress::Broadcast, 80);
    if(button->accessibleDescription() == "off")
    {
        button->setIcon(img1);
        button->setAccessibleDescription("on");
        master->setIcon(img1);
    }
    else
    {
        button->setIcon(img);
        button->setAccessibleDescription("off");
        lcheck();
    }
}

/*
This would switch off all switches created by user.
*/
void Widget::masteroff()
{
    QByteArray datagram = "0";
    udpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress::Broadcast, 80);
    alloff();
}

/*
Updates state of all switches based on information available from micro controller or server.
*/
void Widget::processPendingDatagrams()
{
    QPixmap img(":/off.png");
    QPixmap img1(":/on.png");

    while (udpSocketX->hasPendingDatagrams()) {
        datagramX.resize(udpSocket->pendingDatagramSize());
        udpSocketX->readDatagram(datagramX.data(), datagramX.size());
        text = datagramX.data();
        val = text.at(0);
        text.remove(0,1);
        if(val == "a")
        {
            num = text.toInt();
            image[num]->setIcon(img1);
            image[num]->setAccessibleDescription("on");
        }
        else if(val == "b")
        {
            if(num != 0)
            {
                num = text.toInt();
                image[num]->setIcon(img1);
                image[num]->setAccessibleDescription("off");
                lcheck();
            }
            else
            {
                alloff();
            }
        }
        else
        {
            status();
        }
    }
}

/*
Sets all switches icons to OFF
*/
void Widget::alloff()
{
    QPixmap img(":/off.png");
    master->setIcon(img);
    for (int i = 0; i < xval; i++)
    {
        image[i]->setIcon(img);
    }
}

/*
Sets the button icon for master off switch based on status of all individual switches.
*/
void Widget::lcheck()
{
    flag = 0;
    QPixmap img(":/off.png");
    for(int i = 0; i < xval; i++)
    {
        if(image[i]->accessibleDescription() == "on")
        {
            flag = 1;
            break;
        }
    }
    if(flag == 0)
        master->setIcon(img);
}

/*
Initializes the switch buttons to a default value
*/
void Widget::status()
{
    QPixmap img1(":/on.png");
    while(text.at(0) != '\0')
    {
        QString temp = text.at(0);
        text.remove(0,1);
        while(text.at(0) != 'a')
        {
            temp.append(text[0]);
            text.remove(0,1);
        }
        num = temp.toInt();
        text.remove(0,1);
        image[num]->setIcon(img1);
        image[num]->setAccessibleDescription("on");
    }
}

/*
setting constructor. This creates a GUI window to set number of switches.
*/
settings::settings(QWidget *parent) : QWidget(parent)
{
    layout1 = new QVBoxLayout;
    QLabel *lbl = new QLabel;
    lbl->setText("Set Number of Switches");
    numbr = new QSpinBox;
    layout1->addWidget(lbl);
    layout1->addWidget(numbr);
    ok = new QPushButton;
    ok->setText("Accept");
    layout1->addWidget(ok);
    connect(ok,SIGNAL(clicked()),this,SLOT(changestate()));
    setLayout(layout1);
}

/*
Changes the number of switches to be used and save it in database.
*/
void settings::changestate()
{
    QSqlQuery qry;
    qry.prepare( "CREATE TABLE IF NOT EXISTS number (num INTEGER)" );
    qry.exec();
    qry.prepare("DELETE FROM number");
    qry.exec();
    qry.prepare("INSERT INTO number VALUES (?)");
    qry.addBindValue(numbr->value());
    qry.exec();
    xval = numbr->value();
}

/*
value Constructor. This class handles functionality of database which stores names of switches for user.
Function creates GUI for database modification and entries already exisiting from database.
*/
value::value(QWidget *parent) : QWidget(parent)
{
    QSqlQuery qry;
    qry.prepare( "select * from names" );
    qry.exec();

    layout2 = new QVBoxLayout;
    for(i = 0; i < xval; i++)
    {
        line = new QLineEdit;
        layout2->addWidget(line);
        linex << line;
    }
    change = new QPushButton;
    change->setText("Change Names");
    layout2->addWidget(change);
    setLayout(layout2);
    connect(change,SIGNAL(clicked()),this,SLOT(modify()));
    }

/*
Function to create a new entry (name) for database.
*/
void value::modify()
{
    QSqlQuery qry;
    for(i = 0; i < xval; i++)
    {
        qry.prepare("INSERT INTO names VALUES (?, ?)");
        qry.addBindValue(i);
        qry.addBindValue(linex[i]->text());
        qry.exec();
    }
}
