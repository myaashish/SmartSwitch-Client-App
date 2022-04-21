#ifndef WIDGET_H
#define WIDGET_H

#include <QtSql/QSql>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QTimer;
class QUdpSocket;
QT_END_NAMESPACE

class settings : public QWidget
{
Q_OBJECT

public:
    settings(QWidget *parent = 0);

private:
    QVBoxLayout *layout1;
    QSpinBox *numbr;
    QPushButton *ok;

private slots:
        void changestate();
};

class mlayout : public QWidget
{
    Q_OBJECT

public:
    mlayout(QWidget *parent = 0);
    ~mlayout();

private:
    QPushButton *quitButton;
    QPushButton *settingsbtn;
    QListWidget *list;
    QStackedWidget *stack;

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void leftwidth();
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    int i;
    void alloff();
    void lcheck();
    void status();

private slots:
    void broadcastDatagram();
    void masteroff();
    void processPendingDatagrams();

private:
    QLabel *statusLabel;
    QByteArray datagramX, datagram;
    QCheckBox *check;
    int num, flag;
    QString text, val, temp;
    QPushButton *master;
    QUdpSocket *udpSocket, *udpSocketX;
};

class value : public QWidget
{
    Q_OBJECT

public:
    value(QWidget *parent = 0);

private:
    QVBoxLayout *layout2;
    QPushButton *change;
    QVector<QLineEdit*> linex;
    QDir directory;
    QString filepath;
    QSqlDatabase db;
    int i;
    QLineEdit *line;

private slots:
    void modify();
};

#endif
