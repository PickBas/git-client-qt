#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->ui->log_operations->setReadOnly(true);

    this->settings = new QSettings("Savings");

    QString prev_path = settings->value("path").toString();

    if (prev_path.length()) {
        if (!QDir::setCurrent(prev_path)) {
                qDebug() << "ERROR : Could not change the current working directory";
        } else {
            this->folder_name = prev_path;
            ui->label->setText(prev_path);
        }
    }

   this-> process = new QProcess(this);

    connect(this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(read_output()));
}

MainWindow::~MainWindow() {
    delete process;
    delete settings;
    delete ui;
}

void MainWindow::on_open_folder_btn_clicked(){
    this->folder_name = QFileDialog::getExistingDirectory(this, tr("Open folder"), "/home");

    if (!QDir::setCurrent(this->folder_name)) {
        qDebug() << "ERROR : Could not change the current working directory";
    }

    qDebug() << "Set folder_name : " << this->folder_name << '\n';
    this->ui->log_operations->appendPlainText("Set folder_name : " + this->folder_name + "\n");

    this->settings->setValue("path", this->folder_name);

    this->ui->label->setText(this->folder_name);

    qDebug() << "Set label : " << this->folder_name << '\n';
    this->ui->log_operations->appendPlainText("Set label : " + this->folder_name + "\n");
}

void MainWindow::on_send_btn_clicked() {
    this->process->setParent(this);

    QString commit = this->ui->commit_text->text();

    if (!commit.length()) {
        QMessageBox::warning(this, "Warning", "Write a commit!");
    }

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
}

void MainWindow::read_output(){
    QString data =  QString::fromStdString(this->process->readAllStandardOutput().toStdString());
    auto q = data.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
    for (auto& i : q) {
        qDebug() << i << '\n';
    }
}
