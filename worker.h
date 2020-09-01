#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include "myfile.h"

class Worker : public QObject
{
    Q_OBJECT
private:
    MyLib::MyFile *fileSystem;
public:
    Worker();
    virtual ~Worker() {}
public slots:
    void getDebug();
    void getDirName(QString filter, QString dir);
    void getSort(QString type);
    void getNulify(QString filePath);
signals:
    void sendList(QFileInfoList);
    void sendInfo(QList<int>);
    void started();
    void finished();
};


#endif // WORKER_H
