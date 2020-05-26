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

    check_git();

    if (prev_path.length()) {
        if (!QDir::setCurrent(prev_path)) {
            show_notification("Oops!", "ERROR : Could not change the current working directory");
        } else {
            this->folder_name = prev_path;
            ui->label->setText(prev_path);
            get_branches();
        }
    }
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

void MainWindow::get_branches(){
    QStringList args;
    args << "branch" << "-a";
    this->process->start("git", args);
    this->process->waitForFinished(5000);

    if (!this->current_output.isEmpty()) {
        append_branches_to_menu();
    } else {
        quint8 answer = QMessageBox::warning(this, "Git", this->folder_name + " has no .git",
                                             "Change", "Quit");
        if (!answer) {
            this->current_output.clear();
            on_open_folder_btn_clicked();
        } else {
            exit(0);
        }
    }
}

void MainWindow::append_branches_to_menu(){
    this->current_output.removeAll("->");

    for (int i = 0; i < this->current_output.size(); ++i) {
        this->current_output[i].replace("remotes/", "");
        this->current_output[i].replace("origin/", "");

        if(this->current_output[i] == "*") {
            this->current_branch = this->current_output[i + 1];
            this->ui->menuBranches->setTitle(this->current_branch);
        }
    }

    this->current_output.removeAll("*");

    this->current_output.removeAll("HEAD");
    this->current_output.removeDuplicates();

    this->ui->menuBranches->clear();

    for (const QString& branch : this->current_output)
        this->ui->menuBranches->addAction(branch);

    for (QAction* action : this->ui->menuBranches->actions()) {
        connect(action, &QAction::triggered, this, [=]() {
            this->checkout_branch(action->text());
            this->current_output.clear();
        });
    }

    this->current_output.clear();
}

void MainWindow::show_notification(const QString& notification_header,const QString& notification_body){
    this->tray->showMessage(notification_header, notification_body, QIcon(":/pic/Git-Icon.png"), 3000);
}

void MainWindow::checkout_branch(const QString &branch){
    QStringList args;
    args << "checkout" << branch;
    this->process->start("git", args);
    this->process->waitForFinished(5000);
    this->current_branch =  branch;
    this->ui->menuBranches->setTitle(this->current_branch);
}

MainWindow::~MainWindow() {
    delete tray;
    delete process;
    delete settings;
    delete ui;
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

void MainWindow::read_output(){
    QString data =  QString::fromStdString(this->process->readAllStandardOutput().toStdString());
    this->current_output = data.split(QRegExp("\n|\r\n|\r| "), QString::SkipEmptyParts);
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
