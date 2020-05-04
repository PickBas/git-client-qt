#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->ui->log_operations->setReadOnly(true);

    settings = new QSettings("Savings");

    QString prev_path = settings->value("path").toString();

    if (prev_path.length()) {
        if (!dir->setCurrent(prev_path)) {
                qDebug() << "ERROR : Could not change the current working directory";
        } else {
            this->folder_name = prev_path;
            ui->label->setText(prev_path);
        }
    }

    process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(read_output()));
    process->setProgram("git");
}

MainWindow::~MainWindow() {
    process->kill();
    delete settings;
    delete process;
    delete dir;
    delete ui;
}

void MainWindow::on_open_folder_btn_clicked(){
    this->folder_name = QFileDialog::getExistingDirectory(this, tr("Open folder"), "/home");

    if (!dir->setCurrent(this->folder_name))
            qDebug() << "ERROR : Could not change the current working directory";

    qDebug() << "Set folder_name : " << this->folder_name << '\n';
    this->ui->log_operations->appendPlainText("Set folder_name : " + this->folder_name + "\n");

    settings->setValue("path", this->folder_name);

    this->ui->label->setText(this->folder_name);

    qDebug() << "Set label : " << this->folder_name << '\n';
    this->ui->log_operations->appendPlainText("Set label : " + this->folder_name + "\n");
}

void MainWindow::on_send_btn_clicked() {
    QStringList args;
    args << "status";
    process->setArguments(args);
    process->start();
    process->waitForFinished();
    QString first_command = "git add .";
    this->ui->log_operations->appendPlainText("First command : " + first_command + "\n");
    qDebug() << first_command << '\n';

    QString second_command = "git commit -m \"" + this->ui->commit_text->text() + "\"";
    this->ui->log_operations->appendPlainText("Second command : " + second_command + "\n");
    qDebug() << second_command << '\n';

    QString third_command = "git push";
    this->ui->log_operations->appendPlainText("Third command : " + third_command + "\n");
    qDebug() << third_command << '\n';

    this->ui->log_operations->appendPlainText("Done!\n");
    qDebug() << "Done!\n";
}

void MainWindow::read_output(){
    QString data =  QString::fromStdString(process->readAllStandardOutput().toStdString());
    auto q = data.split(QRegExp("\n|\r\n|\r"), QString::SkipEmptyParts);
    for (auto& i : q) {
        qDebug() << i << '\n';
    }
}
