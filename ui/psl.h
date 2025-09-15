#ifndef PSL_H
#define PSL_H

#include <QMainWindow>
#include <filesystem>

QT_BEGIN_NAMESPACE
namespace Ui
{
class psl;
}
QT_END_NAMESPACE

class psl : public QMainWindow
{
    Q_OBJECT

public:
    psl(QWidget *parent = nullptr);
    ~psl();

private slots:
    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionSaveAs_triggered();

    void on_actionExit_triggered();

    void on_compileButton_clicked();

    void output_contents_of_file(const std::filesystem::path& path);

private:
    Ui::psl *ui;
    QString source_path{};
};
#endif
