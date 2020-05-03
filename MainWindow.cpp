#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->ui->log_operations->setReadOnly(true);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_open_folder_btn_clicked(){
    this->folder_name = QFileDialog::getExistingDirectory(this, tr("Open folder"), "/home");
    qDebug() << "Set folder_name : " << this->folder_name << '\n';
    this->ui->log_operations->appendPlainText("Set folder_name : " + this->folder_name + "\n");

    this->ui->label->setText(this->folder_name);

    qDebug() << "Set label : " << this->folder_name << '\n';
    this->ui->log_operations->appendPlainText("Set label : " + this->folder_name + "\n");
}

void MainWindow::on_send_btn_clicked() {
    QString first_command = "git add " + this->folder_name;
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
