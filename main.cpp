/*
Author: Aashish Parmar (parmaraashish3@gmail.com)

It creates an Android App using Qt cross compiler for client side communication for home automation project.
*/

#include "widget.h"
#include <QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mlayout w;
    QPixmap pixmap(":/on.png");
    QSplashScreen splash(pixmap);
    splash.showMessage("Smart Switch", Qt::AlignCenter);
    splash.show();
    QIcon icx;
    icx.addPixmap(pixmap);
    a.setWindowIcon(icx);
    w.setWindowTitle("Smart Switch");
    QTimer::singleShot(4000, &splash, SLOT(close()));
    QTimer::singleShot(4000, &w, SLOT(show()));

    return a.exec();
}
