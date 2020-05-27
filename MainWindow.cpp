#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/pic/Git-Icon.png"));

    this->current_output.clear();

    this->ui->operations_comboBox->setEditable(true);
    this->ui->operations_comboBox->lineEdit()->setReadOnly(true);
    this->ui->operations_comboBox->lineEdit()->setAlignment(Qt::AlignCenter);

    this->ui->operations_comboBox->addItem("Commit");
    this->ui->operations_comboBox->addItem("Commit & push");
    this->ui->operations_comboBox->addItem("push");

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    this->settings = new QSettings("Savings");

    QString prev_path = settings->value("path").toString();
    this->process = new QProcess(this);
    this->tray = new QSystemTrayIcon(this);

    connect(this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(read_output()));
    this->tray->setIcon(QIcon(":/pic/Git-Icon.png"));
    this->tray->show();
    this->tray->setToolTip("Git client");

    check_git();

    if (prev_path.length()) {
        if (!QDir::setCurrent(prev_path)) {
            show_notification("Oops!", "ERROR : Could not change the current working directory");
        } else {
            this->folder_name = prev_path;
            ui->label->setText(prev_path);
            if(check_git_in_folder()) {
                get_branches();
            }
        }
    }
}

MainWindow::~MainWindow() {
    delete tray;
    delete process;
    delete settings;
    delete ui;
}

void MainWindow::on_send_btn_clicked() {
    this->process->setParent(this);

    QString commit = this->ui->commit_text->text();

    if (!commit.length()) {
        QMessageBox::warning(this, "Warning", "Write a commit!");
    } else {
        QStringList args;
        if(this->ui->operations_comboBox->currentText() == "Commit") {
            args << "add" << ".";
            this->process->start("git", args);
            this->process->waitForFinished(5000);
            this->current_output.clear();

            args.clear();
            args << "commit" << "-m" << this->ui->commit_text->text();
            this->process->start("git", args);
            this->process->waitForFinished(5000);

            if (this->current_output.size())
                show_notification("Done!", "Commited: " + this->ui->commit_text->text());
            else
                show_notification("Oops!", "Something went wrong!");


        } else if (this->ui->operations_comboBox->currentText() == "Commit & push") {
            quint8 answer = QMessageBox::question(this, "Check", "Commit: " + commit + "\nBranch: " + this->current_branch + "\n\nIs it corrent?", "Yes", "No");
            if (!answer) {
                args << "add" << ".";
                this->process->start("git", args);
                this->process->waitForFinished(5000);

                args.clear();
                args << "commit" << "-m" << this->ui->commit_text->text();
                this->process->start("git", args);
                this->process->waitForFinished(5000);

                args.clear();
                args << "push" << "origin" << this->current_branch;
                this->process->start("git", args);
                this->process->waitForFinished(5000);
                if (this->current_output.size())
                    show_notification("Done!", "Commited: " + this->ui->commit_text->text() + ";\nPushed: " + this->current_branch);
                else
                    show_notification("Oops!", "Something went wrong!");
                this->current_output.clear();
            }
        } else {
            args.clear();
            args << "push" << "origin" << this->current_branch;
            this->process->start("git", args);
            this->process->waitForFinished(5000);
            this->current_output.clear();
            show_notification("Done!", "Pushed: " + this->current_branch);
        }
    }
    this->current_output.clear();
}

void MainWindow::append_branches_to_menu(){
    QStringList branches;

    for (const QString& branch : this->current_output)
        branches << branch.split(QRegExp(" "), QString::SkipEmptyParts);

    branches.removeAll("->");

    for (int i = 0; i < branches.size(); ++i) {
        branches[i].replace("remotes/", "");
        branches[i].replace("origin/", "");

        if(branches[i] == "*") {
            this->current_branch = branches[i + 1];
            this->ui->menuBranches->setTitle(this->current_branch);
        }
    }

    branches.removeAll("*");

    branches.removeAll("HEAD");
    branches.removeDuplicates();

    this->ui->menuBranches->clear();

    for (const QString& branch : branches) {
        QAction* action = new QAction(branch);
        action->setCheckable(true);
        this->ui->menuBranches->addAction(action);
    }

    for (QAction* action : this->ui->menuBranches->actions()) {
        connect(action, &QAction::triggered, this, [=]() {
            this->checkout_branch(action->text());
            this->current_output.clear();
        });
    }

    set_checked_action();

    this->current_output.clear();
}

bool MainWindow::check_git_in_folder(){
    QStringList args;

    args << "status";
    this->process->start("git", args);
    this->process->waitForFinished(5000);

    if (!this->current_output.isEmpty()) {
        this->current_output.clear();
        return true;
    }

    quint8 answer = QMessageBox::warning(this, "Git", this->folder_name + " has no .git",
                                         "Initialize", "Change", "Quit");
    if (!answer) {
        args.clear();
        args << "init";
        this->process->start("git", args);
        this->process->waitForFinished(5000);

        QString notif_body;

        for(const QString& word: current_output) {
            notif_body += word + " ";
        }

        show_notification("Done!", notif_body);

        this->current_output.clear();

    } else if (answer == 1) {
        this->current_output.clear();
        on_open_folder_btn_clicked();
    } else {
        exit(0);
    }
    return true;
}

void MainWindow::check_git(){
    QStringList args;
    args << "--version";
    this->process->start("git", args);
    this->process->waitForFinished(5000);

    if (!this->current_output.isEmpty()) {
        this->current_output.clear();
        return;
    } else {
        quint8 answer = QMessageBox::warning(this, "Git", "You don't have git installed on your PC.",
                                             "Install", "Quit");
        if (!answer) {
            QDesktopServices::openUrl(QUrl("https://git-scm.com/", QUrl::TolerantMode));
            exit(0);
        } else {
            exit(0);
        }
    }
}

void MainWindow::on_actionCreate_branch_triggered(){
    DialogNewBranch d(this);
    if (d.exec() != QDialog::Rejected) {
        QStringList args;
        args << "checkout" << "-b" <<  d.get_branch_name();
        this->process->start("git", args);
        this->process->waitForFinished(5000);

        get_branches();

        if (d.get_branch_name() != this->ui->menuBranches->title()) {
            QMessageBox::critical(this, "ERROR", "Invalind branch name");
            on_actionCreate_branch_triggered();
        } else {
            this->current_output.clear();
        }
    }
}

void MainWindow::on_open_folder_btn_clicked(){
    this->folder_name = QFileDialog::getExistingDirectory(this, tr("Open folder"), "/home");

    if (!QDir::setCurrent(this->folder_name)) {
        show_notification("Oops!", "Could not change the current working directory");
    } else {
        this->settings->setValue("path", this->folder_name);
        this->ui->label->setText(this->folder_name);
        this->ui->menuBranches->clear();

        get_branches();
    }
}

void MainWindow::checkout_branch(const QString &branch){
    QStringList args;
    args << "checkout" << branch;
    this->process->start("git", args);
    this->process->waitForFinished(5000);
    this->current_branch = branch;
    set_checked_action();
    this->ui->menuBranches->setTitle(this->current_branch);
}

void MainWindow::set_checked_action(){
    for (QAction* action : this->ui->menuBranches->actions()) {
        if(action->text() == this->current_branch) {
            action->setChecked(true);
        } else {
            action->setChecked(false);
        }
    }
}

void MainWindow::get_branches(){
    QStringList args;
    args << "branch" << "-a";
    this->process->start("git", args);
    this->process->waitForFinished(5000);
    append_branches_to_menu();
}

void MainWindow::read_output(){
    QString data =  QString::fromStdString(this->process->readAllStandardOutput().toStdString());
    this->current_output = data.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
}

void MainWindow::show_notification(const QString& notification_header,const QString& notification_body){
    this->tray->showMessage(notification_header, notification_body, QIcon(":/pic/Git-Icon.png"), 3000);
}
