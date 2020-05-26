#include "DialogNewBranch.h"
#include "ui_DialogNewBranch.h"
#include <QDebug>

DialogNewBranch::DialogNewBranch(QWidget *parent) : QDialog(parent), ui(new Ui::DialogNewBranch){
    ui->setupUi(this);
}

QString DialogNewBranch::get_branch_name(){
    return this->branch_name;
}

DialogNewBranch::~DialogNewBranch(){
    delete ui;
}

void DialogNewBranch::on_create_btn_clicked(){
    this->branch_name = this->ui->branch_name->text();
    accept();
}

void DialogNewBranch::on_cancel_btn_clicked(){
    reject();
}
