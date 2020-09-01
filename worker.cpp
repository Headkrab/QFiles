#include "worker.h"
#include <QFileInfoList>


Worker::Worker()
{
    fileSystem = new MyLib::MyFile;
}

void Worker::getDebug()
{
    emit started();
    fileSystem->changeDebug();
    emit finished();
}

void Worker::getDirName(QString filter, QString dir)
{
    emit started();
    if (!fileSystem->setDir(dir))                                           //Проверяем существование директории
        {
        //Здесь должно выводиться сообщение об ошибке
        }
    QFileInfoList files = fileSystem->getDirList(filter,dir);
    emit sendList(files);
    emit sendInfo(fileSystem->getSize());
    emit finished();
}

void Worker::getSort(QString arg1)
{
    emit started();
    if      (arg1 == "Имя ↓")
        fileSystem->setSort(0,false);
    else if (arg1 == "Имя ↑")
        fileSystem->setSort(0,true);
    else if (arg1 == "Размер ↓")
        fileSystem->setSort(2,true);
    else if (arg1 == "Размер ↑")
        fileSystem->setSort(2,false);
    else if (arg1 == "Дата ↓")
        fileSystem->setSort(1,true);
    else if (arg1 == "Дата ↑")
        fileSystem->setSort(1,false);
    emit finished();
}

void Worker::getNulify(QString filePath)
{
    emit started();
    sleep(1);
    fileSystem->nulify(filePath);
    emit finished();
}


