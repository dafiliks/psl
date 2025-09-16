/* ui/main.cpp by David Filiks */
/* The UI entry point implementation for the PsL compiler */

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>

#include "psl.h"

/* UI entry point function */
/* Param: int - argument count */
/* Param: char*[] - argument vector */
int main(int argc, char *argv[])
{
    /* Create an application object using argc and argv */
    QApplication app{argc, argv};

    /* Create a psl window object */
    psl window{};

    /* Create a psl icon object */
    QIcon psl_icon{"assets/psl_icon.png"};

    /* Set the window icon */
    window.setWindowIcon(psl_icon);

    /* Display the window to the user */
    window.show();

    /* Enter the main event loop and exit when closed */
    return app.exec();
}
