#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->ui->log_operations->setReadOnly(true);

    this->current_output.clear();

    this->settings = new QSettings("Savings");

    QString prev_path = settings->value("path").toString();
    this->process = new QProcess(this);

    connect(this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(read_output()));

    if (prev_path.length()) {
        if (!QDir::setCurrent(prev_path)) {
            this->ui->log_operations->appendPlainText("ERROR : Could not change the current working directory\n");
        } else {
            this->folder_name = prev_path;
            ui->label->setText(prev_path);
            get_branches();
        }
    }
}

void MainWindow::get_branches(){
    QStringList args;
    args << "branch" << "-a";
    this->process->start("git", args);
    this->process->waitForFinished(5000);
    if (!this->current_output.isEmpty())
        append_branches_to_menu();
}

void MainWindow::append_branches_to_menu(){
    this->current_output.removeAll("->");
    this->current_output.removeAll("*");

    for (QString& branch : this->current_output) {
        branch.replace("remotes/", "");
        branch.replace("origin/", "");
    }

    this->current_output.removeAll("HEAD");
    this->current_output.removeDuplicates();

    for (const QString& branch : this->current_output) {
        this->ui->menubranch->addAction(branch);
    }

    for (QAction* action : this->ui->menubranch->actions()) {
        connect(action, &QAction::triggered, this, [=]() {
            this->checkout_branch(action->text());
            this->current_output.clear();
        });
    }

    this->current_output.clear();
}

void MainWindow::checkout_branch(const QString &branch){
    QStringList args;
    args << "checkout" << branch;
    this->process->start("git", args);
    this->process->waitForFinished(5000);
    this->current_branch =  branch;
    this->ui->menubranch->setTitle(this->current_branch);
}

MainWindow::~MainWindow() {
    delete process;
    delete settings;
    delete ui;
}

void MainWindow::on_open_folder_btn_clicked(){
    this->folder_name = QFileDialog::getExistingDirectory(this, tr("Open folder"), "/home");

    if (!QDir::setCurrent(this->folder_name)) {
        this->ui->log_operations->appendPlainText("ERROR : Could not change the current working directory\n");
    } else {

        this->ui->log_operations->appendPlainText("Set folder_name : " + this->folder_name + "\n");

        this->settings->setValue("path", this->folder_name);

        this->ui->label->setText(this->folder_name);

        this->ui->log_operations->appendPlainText("Set label : " + this->folder_name + "\n");

        this->ui->menubranch->clear();

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
        args << "status";
        this->process->start("git", args);
        this->process->waitForFinished(5000);

        args.clear();
        args << "add" << ".";
        this->process->start("git", args);
        this->process->waitForFinished(5000);

        args.clear();
        QString com_text = "\"" + this->ui->commit_text->text() + "\"";
        args << "commit" << "-m" << com_text;
        this->process->start("git", args);
        this->process->waitForFinished(5000);

        args.clear();
        args << "push";
        this->process->start("git", args);
        this->process->waitForFinished(5000);
        this->current_output.clear();
    }
}

void MainWindow::read_output(){
    QString data =  QString::fromStdString(this->process->readAllStandardOutput().toStdString());
    this->current_output = data.split(QRegExp("\n|\r\n|\r| "), QString::SkipEmptyParts);
    //    for (const QString& i : this->current_output) {
    //        qDebug() << i << '\n';
    //    }
}
