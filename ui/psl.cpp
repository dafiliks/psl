#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

#include "psl.h"
#include "ui_psl.h"
#include "../src/utils/cliargs.hpp"
#include "../src/compiler/compiler.hpp"

psl::psl(QWidget *parent)
: QMainWindow(parent),
  ui(new Ui::psl)
{
    ui->setupUi(this);
}

psl::~psl()
{
    delete ui;
}

void psl::on_actionNew_triggered()
{
    source_path = "";
    ui->inputEdit->setText("");
}


void psl::on_actionOpen_triggered()
{
    source_path = QFileDialog::getOpenFileName(this, "Open");
    QFile file{source_path};

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(this, "Error", "File could not be opened");
        return;
    }

    QTextStream in{&file};
    QString source{in.readAll()};

    ui->inputEdit->setText(source);

    file.close();
}


void psl::on_actionSave_triggered()
{
    QFile file{source_path};

    if (source_path.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No source file has been set, please save as first");
        on_actionSaveAs_triggered();
        return;
    }

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::critical(this, "Error", "File could not be opened");
        return;
    }

    QTextStream out{&file};
    QString source{ui->inputEdit->toPlainText()};
    out << source;

    ui->inputEdit->setText(source);

    file.close();
}


void psl::on_actionSaveAs_triggered()
{
    source_path = QFileDialog::getSaveFileName(this, "Save As");
    QFile file{source_path};

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::critical(this, "Error", "File could not be opened");
        return;
    }

    QTextStream out{&file};
    QString source{ui->inputEdit->toPlainText()};
    out << source;

    ui->inputEdit->setText(source);

    file.close();
}


void psl::on_actionExit_triggered()
{
    QApplication::quit();
}


void psl::on_compileButton_clicked()
{
    if (source_path.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No source file has been set, please save as first");
        on_actionSaveAs_triggered();
        return;
    }

    QByteArray source_path_utf8{source_path.toUtf8()};

    char* argv[3]
    {
        const_cast<char*>("psl"),
        const_cast<char*>(source_path_utf8.constData()),
        nullptr,
    };

    const char* mode{nullptr};

    if (ui->targetBox->currentText() == "Executable")
    {
        mode = "-exe";
    }

    else if (ui->targetBox->currentText() == "C++")
    {
        mode = "-cpp";
    }

    else if (ui->targetBox->currentText() == "Assembly")
    {
        mode = "-asm";
    }

    else if (ui->targetBox->currentText() == "Object")
    {
        mode = "-obj";
    }

    argv[2] = const_cast<char*>(mode);

    /* Try to run the following code */
    try
    {
        /* Construct a CLIArgs object, passing argc (3), and argv */
        CLIArgs args{3, argv};

        /* Handle CLI arguments */
        args.handle();

        /* Construct a Compiler object */
        Compiler compiler{args};

        /* Fully compile the source into desired output */
        compiler.compile();
    }

    /* If an Error is thrown */
    catch (const Error& e)
    {
        /* Handle any compilation process error */
        std::cerr << e.what();

        /* Set text in the output edit to show the compilation error */
        this->ui->outputEdit->setText(QString::fromUtf8(e.what()));
    }

    /* If a standard exception is thrown */
    catch (const std::exception& e)
    {
        /* Handle any standard exception */
        std::cerr << "Standard Exception: " << e.what() << "\n";

        /* Set text in the output edit to show the standard error */
        this->ui->outputEdit->setText(QString::fromUtf8(e.what()));
    }

    /* If any other exception is thrown */
    catch (...)
    {
        /* Handle all other (unknown) exceptions */
        std::cerr << "Unknown Exception Thrown" << std::endl;

        /* Set text in the output edit to show the unknown error */
        this->ui->outputEdit->setText("An unknown error has occured");
    }

    if (ui->targetBox->currentText() == "Executable")
    {
        argv[2] = const_cast<char*>("-exe");
    }
    else if (ui->targetBox->currentText() == "C++")
    {
        std::filesystem::path file{source_path.toStdString()};

        file.replace_extension(".cpp");

        output_contents_of_file(file);
    }
    else if (ui->targetBox->currentText() == "Assembly")
    {
        std::filesystem::path file{source_path.toStdString()};

        file.replace_extension
        (
            #ifdef _MSC_VER
                ".asm"
            #else
                ".s"
            #endif
        );

        output_contents_of_file(file);
    }
    else if (ui->targetBox->currentText() == "Object")
    {
        std::filesystem::path file{source_path.toStdString()};

        file.replace_extension
        (
            #ifdef _MSC_VER
                ".obj"
            #else
                ".o"
            #endif
        );

        output_contents_of_file(file);
    }
}

void psl::output_contents_of_file(const std::filesystem::path& path)
{
    QFile output_file{QString::fromStdString(path.string())};
    if (!output_file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(this, "Error", "The generated output file could not be opened");
    }

    QTextStream source_stream{&output_file};
    QString output_source{source_stream.readAll()};

    ui->outputEdit->setText(output_source);

    output_file.close();
}
