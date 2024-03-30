#include "online.h"
#include "ui_online.h"


Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    qDebug() << "online delete";
    delete ui;
}

void Online::setOnlineAllListWidget(QStringList nameList)
{
    ui->online_lw->clear();
    for(int i=0;i<nameList.length();i++)
    {
        ui->online_lw->addItem(nameList.at(i));
    }
}

void Online::on_addFriend_pb_clicked()
{
    QString friendName = ui->online_lw->currentItem()->text();
    if(!friendName.isEmpty())
         emit addFriendBtnClicked(friendName);
}

