/* ui/psl.h by David Filiks */
/* The UI header for the PsL compiler */

#ifndef PSL_H
#define PSL_H

#include <QMainWindow>
#include <filesystem>

#include "ui_psl.h"
#include "../src/utils/cliargs.hpp"
#include "../src/compiler/compiler.hpp"

/* Macro which begins a Qt namespace */
QT_BEGIN_NAMESPACE

/* Namespace holding all classes generated from ".ui" files */
namespace Ui
{
    /* Forward declaration of the psl class */
    class psl;
}

/* Macro which ends a Qt namespace */
QT_END_NAMESPACE

/* The main PsL UI object, inherits from QMainWindow */
class psl : public QMainWindow
{
    Q_OBJECT /* Macro which enables signals, slots and meta-object features */

/* Public members */
public:

    /* Functions */

    /* Constructs a psl object */
    /* Param: QWidget* - a pointer to the parent widget */
    psl(QWidget *parent = nullptr);

    /* Destructs a psl object */
    ~psl();

/* Private slots */
private slots:

    /* Functions */

    /* Executed upon the "New" action being triggered */
    void on_actionNew_triggered();

    /* Executed upon the "Open" action being triggered */
    void on_actionOpen_triggered();

    /* Executed upon the "Save" action being triggered */
    void on_actionSave_triggered();

    /* Executed upon the "Save As" action being triggered */
    void on_actionSaveAs_triggered();

    /* Executed upon the "Exit" action being triggered */
    void on_actionExit_triggered();

    /* Executed upon the "Compile" button being clicked */
    void on_compileButton_clicked();

    /* Sets the output text edit to the contents of a given file */
    /* Param: const std::filesystem::path& - the path of the file */
    void output_contents_of_file(const std::filesystem::path& path);

/* Private members */
private:

    /* Variables */

    Ui::psl *ui; /* Holds a pointer to the UI object */
    QString source_path{}; /* Stores the path of the currently opened source file */
};

#endif
