/* ui/psl.cpp by David Filiks */
/* The UI implementation for the PsL compiler */

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

#include "psl.h"

psl::psl(QWidget *parent)
/* Initialize the main window and UI object */
: QMainWindow(parent),
  ui(new Ui::psl)
{
    /* Run the setup function for the UI */
    ui->setupUi(this);
}

psl::~psl()
{
    /* Delete the UI object */
    delete ui;
}

void psl::on_actionNew_triggered()
{
    /* Clear the source path as no valid file is currently opened */
    source_path.clear();

    /* Set the input text edit to empty */
    ui->inputEdit->setText("");
}

void psl::on_actionOpen_triggered()
{
    /* Open a file dialog to let the user pick the source path */
    source_path = QFileDialog::getOpenFileName(this, "Open");

    /* Create a file object from the source path */
    QFile file{source_path};

    /* Open the file to only read text */
    /* If the file did not open correctly */
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        /* Open an error box indicating the issue */
        QMessageBox::critical(this, "Error", "File could not be opened");

        /* Return from the function early */
        return;
    }

    /* Open a text stream to the file */
    QTextStream in{&file};

    /* Store all of the text in the source file */
    QString source{in.readAll()};

    /* Set the input text edit to the contents of the source file */
    ui->inputEdit->setText(source);

    /* Close the file handle */
    file.close();
}

void psl::on_actionSave_triggered()
{
    /* Create a file object from the source path */
    QFile file{source_path};

    /* If no source path has been set */
    if (source_path.isEmpty())
    {
        /* Open a warning box indicating the issue */
        QMessageBox::warning(this, "Warning", "No source file has been set, please save as first");

        /* Force the user to save the file as something first */
        on_actionSaveAs_triggered();

        /* Return from the function early */
        return;
    }

    /* Open the file to only write text */
    /* If the file did not open correctly */
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        /* Open an error box indicating the issue */
        QMessageBox::critical(this, "Error", "File could not be opened");

        /* Return from the function early */
        return;
    }

    /* Open a text stream to the file */
    QTextStream out{&file};

    /* Store all of the text in the input text edit */
    QString source{ui->inputEdit->toPlainText()};

    /* Write the input text edit source to the file */
    out << source;

    /* Close the file handle */
    file.close();
}

void psl::on_actionSaveAs_triggered()
{
    /* Open a file dialog to let the user pick where to save the input text edit contents */
    source_path = QFileDialog::getSaveFileName(this, "Save As");

    /* Create a file object from the new source path */
    QFile file{source_path};

    /* Open the file to only write text */
    /* If the file did not open correctly */
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        /* Open an error box indicating the issue */
        QMessageBox::critical(this, "Error", "File could not be opened");

        /* Return from the function early */
        return;
    }

    /* Open a text stream to the file */
    QTextStream out{&file};

    /* Store all of the text in the input text edit */
    QString source{ui->inputEdit->toPlainText()};

    /* Write the input text edit source to the file */
    out << source;

    /* Close the file handle */
    file.close();
}

void psl::on_actionExit_triggered()
{
    /* Quit (or Exit) from the application using Qt API */
    QApplication::quit();
}

void psl::on_compileButton_clicked()
{
    /* If the source file extension is not ".pseudo" */
    if (std::filesystem::path{source_path.toStdString()}.extension() != ".pseudo")
    {
        /* Open an error box indicating the issue */
        QMessageBox::critical(this, "Error", "File lacks the .pseudo extension");

        /* Return from the function early */
        return;
    }

    /* If no source path has been set */
    if (source_path.isEmpty())
    {
        /* Open a warning box indicating the issue */
        QMessageBox::warning(this, "Warning", "No source file has been set, please save as first");

        /* Force the user to save the file as something first */
        on_actionSaveAs_triggered();

        /* Return from the function early */
        return;
    }

    /* Store the source path as UTF-8 so we can call the constData() member later */
    QByteArray source_path_utf8{source_path.toUtf8()};

    /* Create an argument vector array */
    char* argv[3]
    {
        nullptr, /* First element does not matter */
        const_cast<char*>(source_path_utf8.constData()), /* Second element needs to contain the source path */
        nullptr, /* Third element will be set soon, when compilation target is checked */
    };

    /* If the user has chosen the compilation target of an executable from the drop-down */
    if (ui->targetBox->currentText() == "Executable")
    {
        /* Set the third element in the argument vector to "-exe" */
        argv[2] = const_cast<char*>("-exe");
    }

    /* If the user has chosen the compilation target of C++ from the drop-down */
    else if (ui->targetBox->currentText() == "C++")
    {
        /* Set the third element in the argument vector to "-cpp" */
        argv[2] = const_cast<char*>("-cpp");
    }

    /* If the user has chosen the compilation target of assembly language from the drop-down */
    else if (ui->targetBox->currentText() == "Assembly")
    {
        /* Set the third element in the argument vector to "-asm" */
        argv[2] = const_cast<char*>("-asm");
    }

    /* If the user has chosen the compilation target of an object file from the drop-down */
    else if (ui->targetBox->currentText() == "Object")
    {
        /* Set the third element in the argument vector to "-obj" */
        argv[2] = const_cast<char*>("-obj");
    }

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

    /* If the user has just compiled their AQA pseudocode to an executable */
    if (ui->targetBox->currentText() == "Executable")
    {
        QFile file{"__psl_out.txt"};

        /* Open the file to only read text */
        /* If the file did not open correctly */
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            /* Open an error box indicating the issue */
            QMessageBox::critical(this, "Error", "File could not be opened");

            /* Return from the function early */
            return;
        }

        /* Open a text stream to the file */
        QTextStream in{&file};

        /* Store all of the text in the source file */
        QString source{in.readAll()};

        /* Set the input text edit to the contents of the source file */
        ui->outputEdit->setText(source);

        /* Close the file handle */
        file.close();
    }

    /* If the user has just compiled their AQA pseudocode to C++ */
    else if (ui->targetBox->currentText() == "C++")
    {
        /* Store a path object to the source path */
        std::filesystem::path file{source_path.toStdString()};

        /* Change the file extension to ".cpp" */
        /* This is because the generated C++ file will have the same name, but a different extension */
        file.replace_extension(".cpp");

        /* Output the contents of the C++ file in the output text edit */
        output_contents_of_file(file);
    }

    /* If the user has just compiled their AQA pseudocode to assembly language */
    else if (ui->targetBox->currentText() == "Assembly")
    {
        /* Store a path object to the source path */
        std::filesystem::path file{source_path.toStdString()};

        /* Change the file extension to ".asm" or ".s" */
        /* This is because the generated assembly file will have the same name, but a different extension */
        file.replace_extension
        (
            /* If the user is using the MSVC compiler */
            #if defined(_MSC_VER)
                /* Change the file extension to ".asm" as this is what MSVC generates */
                ".asm"
            /* If the user is not using the MSVC compiler */
            #else
                /* Change the file extension to ".s" as this is what GCC and Clang generate */
                ".s"
            #endif
        );

        /* Output the contents of the assembly language file in the output text box */
        output_contents_of_file(file);
    }

    /* If the user has just compiled their AQA pseudocode to an object file */
    else if (ui->targetBox->currentText() == "Object")
    {
        /* Store a path object to the source path */
        std::filesystem::path file{source_path.toStdString()};

        /* Change the file extension to ".obj" or ".o" */
        /* This is because the generated object file will have the same name, but a different extension */
        file.replace_extension
        (
            /* If the user is using the MSVC compiler */
            #if defined(_MSC_VER)
                /* Change the file extension to ".obj" as this is what MSVC generates */
                ".obj"
            /* If the user is not using the MSVC compiler */
            #else
                /* Change the file extension to ".o" as this is what GCC and Clang generate */
                ".o"
            #endif
        );

        /* Output the contents of the object file in the output text box */
        output_contents_of_file(file);
    }
}

void psl::output_contents_of_file(const std::filesystem::path& path)
{
    /* Create a file object from the path provided */
    QFile file{QString::fromStdString(path.string())};

    /* Open the file to only read text */
    /* If the file did not open correctly */
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        /* Open an error box indicating the issue */
        QMessageBox::critical(this, "Error", "The generated output file could not be opened");

        /* Return from the function early */
        return;
    }

    /* Open a text stream to the file */
    QTextStream in{&file};

    /* Store all of the text in the output file */
    QString source{in.readAll()};

    /* Set the output text edit to the contents of the output file */
    ui->outputEdit->setText(source);

    /* Close the file handle */
    file.close();
}
