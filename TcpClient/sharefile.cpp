#include "sharefile.h"
#include "ui_sharefile.h"

#include <QMessageBox>
ShareFile::ShareFile(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShareFile)
{
    ui->setupUi(this);
}

ShareFile::~ShareFile()
{
    deleteButtonGroup();
    delete ui;
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::flushButtonGroup(QStringList friendList)
{
    deleteButtonGroup();
    QCheckBox *cb = NULL;
    for(int i=0;i<friendList.size();i++)
    {
            cb = new QCheckBox(friendList.at(i),ui->m_friendsCB);
        ui->m_friendLayout->addWidget(cb);
        checkBoxs.append(cb);
    }

}

void ShareFile::deleteButtonGroup()
{
    while(!checkBoxs.isEmpty())
    {
        QCheckBox *cb = checkBoxs.takeFirst();
        delete cb;
    }
}

void ShareFile::on_m_SelectAllPB_clicked()
{
    // 全选
    for(QCheckBox* cb:checkBoxs)
    {
        cb->setChecked(true);
    }

}


void ShareFile::on_m_cancelSelectPB_clicked()
{
    // 取消选择
    for(QCheckBox* cb:checkBoxs)
    {
        cb->setChecked(false);
    }
}


void ShareFile::on_m_cancelPB_clicked()
{
    this->hide();
}


void ShareFile::on_m_okPB_clicked()
{
    // 选择了在线好友
    QStringList friendList;
    for(QCheckBox* btn:checkBoxs)
    {
        if(btn->isChecked())
        {
            friendList.append(btn->text());
        }
    }
    if(friendList.size()<=0)
    {
        QMessageBox::warning(this,"分享文件","没有选择好友");
        return ;
    }

    emit selectedFriendsSignal(friendList);
}

