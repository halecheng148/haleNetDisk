#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QButtonGroup>
#include <QCheckBox>
#include <QVBoxLayout>
namespace Ui {
class ShareFile;
}

class ShareFile : public QWidget
{
    Q_OBJECT

public:
    explicit ShareFile(QWidget *parent = nullptr);
    ~ShareFile();

    static ShareFile &getInstance();

    /**
     * 刷新好友列表
     * @brief flushButtonGroup
     */
    void flushButtonGroup(QStringList friendList);

    void deleteButtonGroup();

signals:
    void selectedFriendsSignal(QStringList friends);

private slots:
    void on_m_SelectAllPB_clicked();

    void on_m_cancelSelectPB_clicked();

    void on_m_cancelPB_clicked();

    void on_m_okPB_clicked();

private:
    Ui::ShareFile *ui;

    QList<QCheckBox*> checkBoxs;
};

#endif // SHAREFILE_H
