#include "widget.h"
#include "ui_widget.h"

#include <QMessageBox>
#include <QtWidgets>

enum { absoluteFileNameRole = Qt::UserRole + 1 };
Q_DECLARE_METATYPE(QFileInfoList);

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle(tr("Браузер файлов"));
    setWindowView();

    thread = new QThread;           //Создаем новый поток
    background = new Worker;        //и класс, который будет работать в этом потоке
    qRegisterMetaType<QFileInfoList>("QFileInfoList");          //Нужно чтобы передавать через сигналы данные типа QFileInfoList

    connect(this,SIGNAL(changeDebug()),                         //Связываем сигналы и слоты в двух потоках
            background,SLOT(getDebug()));

    connect(this,SIGNAL(sendDirName(QString,QString)),
            background,SLOT(getDirName(QString, QString)));

    connect(this,SIGNAL(sendSort(QString)),
            background,SLOT(getSort(QString)));

    connect(this,SIGNAL(sendNulify(QString)),
            background,SLOT(getNulify(QString)));

    connect(background,SIGNAL(sendList(QFileInfoList)),
            this,SLOT(printList(QFileInfoList)));

    connect(background,SIGNAL(sendInfo(QList<int>)),
            this,SLOT(printInfo(QList<int>)));

    connect(background,SIGNAL(started()),
            this,SLOT(processStarted()));
    connect(background,SIGNAL(finished()),
            this,SLOT(processFinished()));

    background->moveToThread(thread);       //Запуск потока
    thread->start();                        //
}

Widget::~Widget()
{
    delete ui;
}

void Widget::printList(QFileInfoList files)     //вывод списка файлов в окне вывода
{
    ui->tableWidget->setRowCount(0);            //Очистка окна вывода
    for (QFileInfo file : files) {                                          //Проходимся по списку

        const QString toolTip = QDir::toNativeSeparators(file.filePath());  //Вытаскиваем данные из элемента списка для создания элементов таблицы
        const QString name = file.fileName();                               //
        const qint64 size = file.size();                                    //
        const QDateTime time = file.lastModified();                         //

        QTableWidgetItem *fileNameItem = new QTableWidgetItem(name);
        fileNameItem->setData(absoluteFileNameRole, QVariant(toolTip));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);

        QTableWidgetItem *sizeItem = new QTableWidgetItem(QLocale().formattedDataSize(size));
        sizeItem->setData(absoluteFileNameRole, QVariant(toolTip));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        QTableWidgetItem *timeItem = new QTableWidgetItem(time.toString("hh:mm dd.MM"));
        timeItem->setData(absoluteFileNameRole, QVariant(toolTip));
        timeItem->setToolTip(toolTip);
        timeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        timeItem->setFlags(timeItem->flags() ^ Qt::ItemIsEditable);

        int row = ui->tableWidget->rowCount();                              //Вставляем созданные элементы в поле вывода в новую строку
        ui->tableWidget->insertRow(row);                                    //
        ui->tableWidget->setItem(row, 0, fileNameItem);                     //
        ui->tableWidget->setItem(row, 1, sizeItem);                         //
        ui->tableWidget->setItem(row, 2, timeItem);                         //
    }
}

void Widget::printInfo(QList<int> sizes)        //Вывод количества файлов и их общий размер
{
    QString bottomInfo = "%1 файлов общим размером %2 (для управления файлами нажмите пкм на выбранном файле) ";//
    ui->bottomLabel->setText(bottomInfo.arg(sizes[0]).arg(QLocale().formattedDataSize(sizes[1])));              //

}

void Widget::setWindowView()            //Настройка внешнего вида окна
{
    ui->bottomLabel->setText("");
    ui->findLine->setText("*");
    ui->browseBox->setEditable(true);
    ui->browseBox->addItem("/home");
    ui->browseBox->addItem(QDir::toNativeSeparators(QDir::currentPath()));
    ui->sortBox->addItems({"Имя ↓","Имя ↑","Размер ↓","Размер ↑","Дата ↓","Дата ↑"});

    treeModel = new QFileSystemModel();
    treeModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    treeModel->setRootPath("/home/ruslan");

    ui->treeView->setModel(treeModel);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->setHeaderHidden(true);

    ui->tableWidget->setColumnCount(3);
    QStringList labels;
    labels << tr("Имя") << tr("Размер") << tr("Дата");
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void Widget::on_debugButton_clicked()
{
    QMessageBox::warning(this,"Внимание","Эта кнопка увеличивает время выполнения фоновых процессов (через функцию Sleep)");
    emit changeDebug();
}

void Widget::on_pushButton_clicked()
{
    QString fileName = ui->findLine->text();                                //Вытаскиваем данные из окон ввода
    QString path = QDir::cleanPath(ui->browseBox->currentText());           //
    emit sendDirName(fileName,path);                                        //Отправляем сообщение второму потоку на вывод фильтрованого списка файлов
}

void Widget::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    const QTableWidgetItem *item = ui->tableWidget->itemAt(pos);            //Проверяем что мы щелкнули по файлу, а не пустому месту
    if (!item)
    {
//        QMenu menu;                                                             //Параметры всплывающего меню
//        QAction *newAction = menu.addAction("Новый файл");                      //
//        QAction *action = menu.exec(ui->tableWidget->mapToGlobal(pos));         //

//        if (!action)                                                            //Если кликаем не по пункту, то закрыть меню
//            return;                                                             //

//        if (action == newAction){                                               //Создать новый файл
//            QString fileName = fileSystem.getDir();                             //
//            fileName.append("/New file");                                       //
//            QFile file(fileName);                                               //
//            file.open(QIODevice::Append);                                       //
//            file.close();                                                       //
//            on_pushButton_clicked();                                            //
//        }
    }
    else
    {
        QMenu menu;                                                             //Параметры всплывающего меню
        QAction *openAction = menu.addAction("Открыть файл");                   //
        QAction *delAction = menu.addAction("Удалить");                         //
        QAction *action = menu.exec(ui->tableWidget->mapToGlobal(pos));         //
        if (!action)                                                            //Если кликаем не по пункту, то закрыть меню
            return;                                                             //

        if (action == openAction)                                               //Открыть файл
            QDesktopServices::openUrl(QUrl::fromLocalFile(item->toolTip()));    //

        if (action == delAction)                                                //Обнулить размер файла
        {                                                                       //
            QMessageBox::StandardButton reply =                                 //
                    QMessageBox::question(this,"Удалить файл",                  //
                                          "Данное действме сотрет данные в файле, но не удалит его полностью!",
                                          QMessageBox::Yes|QMessageBox::No);    //
            if (reply == QMessageBox::Yes)                                      //
            {                                                                   //
                emit sendNulify(item->toolTip());                               //Отправляем запрос на нулификацию
                on_pushButton_clicked();                                        //Заново отображаем список файлов
            }
        }
    }
}

void Widget::on_treeView_doubleClicked(const QModelIndex &index)
{
    QString sPath = treeModel->fileInfo(index).absoluteFilePath();
    if (ui->browseBox->findText(sPath) == -1)                       //Если в выпадающем окне нет пункта с этой директорией, то
        ui->browseBox->addItem(sPath);                              //      Добавляем новый пункт
    ui->browseBox->setCurrentIndex(ui->browseBox->findText(sPath)); //Выбираем этот пункт в качестве текущего
    on_pushButton_clicked();
}

void Widget::on_sortBox_currentIndexChanged(const QString &arg1)
{
    emit sendSort(arg1);
    on_pushButton_clicked();
}

void Widget::processStarted()
{
    this->setCursor(Qt::WaitCursor);
}

void Widget::processFinished()
{
    this->setCursor(Qt::ArrowCursor);
}
