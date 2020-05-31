#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QProcess>
#include <QDir>
#include <QTextCodec>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QIcon>
#include <DialogNewBranch.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void check_git();
    void get_branches();
    void append_branches_to_menu();
    void set_checked_action();
    bool check_git_in_folder();
    void get_all_commits();
    void append_commits_to_menu();
    void show_notification(const QString&,const QString&);
    void checkout_branch(const QString&);
    void revert(const QString&);
    ~MainWindow();

private slots:
    void on_open_folder_btn_clicked();

    void on_send_btn_clicked();

    void read_output();

    void on_actionCreate_branch_triggered();

private:
    Ui::MainWindow *ui;
    QSettings *settings;
    QProcess* process;
    QMap <QString, QString> commits;
    QString current_branch;
    QStringList current_output;
    QString folder_name;
    QSystemTrayIcon* tray;
};

#endif // MAINWINDOW_H
