#ifndef DIALOGNEWBRANCH_H
#define DIALOGNEWBRANCH_H

#include <QDialog>

namespace Ui {
class DialogNewBranch;
}

class DialogNewBranch : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewBranch(QWidget *parent = nullptr);
    QString get_branch_name();
    ~DialogNewBranch();

private slots:

    void on_create_btn_clicked();

    void on_cancel_btn_clicked();

private:
    Ui::DialogNewBranch *ui;
    QString branch_name;
};

#endif // DIALOGNEWBRANCH_H
