#include "psl.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    psl window;

    QIcon psl_icon{"assets/psl_icon.png"};
    window.setWindowIcon(psl_icon);

    window.show();
    return app.exec();
}
