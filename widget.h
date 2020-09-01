#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QThread>
#include <myfile.h>
#include <worker.h>



QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
public slots:
    void printList(QFileInfoList files);

    void printInfo(QList<int> sizes);

private slots:
    void on_debugButton_clicked();

    void on_pushButton_clicked();

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_sortBox_currentIndexChanged(const QString &arg1);

    void processStarted();
    void processFinished();

signals:
    void changeDebug();
    void sendDirName(QString,QString);
    void sendSort(QString);
    void sendNulify(QString);

private:
    Ui::Widget *ui;
    QFileSystemModel *treeModel;
    QThread *thread;
    MyLib::MyFile *fileSystem;
    Worker *background;

    void setWindowView();
};
#endif // WIDGET_H
