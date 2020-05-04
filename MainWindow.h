#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QTextCodec>
#include <QSettings>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_open_folder_btn_clicked();

    void on_send_btn_clicked();

    void read_output();

private:
    Ui::MainWindow *ui;
    QSettings *settings;
    QProcess* process;
    QString folder_name;
};

#endif // MAINWINDOW_H
