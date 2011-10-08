#include "panelReg.h"
// #include "ui_panelReg.h"

#include "../iodev.h"
#include "../trendchar/trendchart.h"
#include <math.h>

#include <QString>
#include <QStringList>

#include <QtSql>
#include <QEvent>


RpanelReg::RpanelReg(IoDev &source,int n/*=0*/,QWidget *p/*=NULL*/ ,QString cfName,QString tableName) :QDialog(p),
    src(source)  ,
    RegNum(n) ,
    tblName(tableName),
    ui(new Ui::panelReg)
{
    ui->setupUi(this);
    QSettings set;


    int w=set.value("ranelReg/width",912).toInt();
    int h=set.value("ranelReg/height",562).toInt();
    QRect pos=QRect((qApp->desktop()->size().width()-w)/2,(qApp->desktop()->size().height()-h)/2,w,h);
    this->setGeometry(pos);

    // на замовлення Поліщученка
    ui->Trend->hide();
    //ui->label->hide();
    //ui->regRev->hide();

    ui->RegParm->hide(); // сховати область настройки регулятора
    //resize(size()-QSize(0,159)); // зменшити розмір вікна
    
    connect(ui->Exit,SIGNAL(clicked()),this,SLOT(close()));
    //connect(ui->Setting,SIGNAL(clicked()),this,SLOT(Control()));
    connect(ui->Trend,SIGNAL(clicked()),this,SLOT(runTrend()));

    // реакції користувацького інтерфейсу
    connect(ui->vsX,SIGNAL(valueChanged(int)),this,SLOT(setCtrlValue(int)));
    connect(ui->sbX,SIGNAL(valueChanged(double)),this,SLOT(setCtrlValue(double)));
    // SP_1
    connect(ui->vsSP_1,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    connect(ui->sbSP_1,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // SP_2
    connect(ui->vsSP_2,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    connect(ui->sbSP_2,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));
    // SP_3
    connect(ui->vsSP_3,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    connect(ui->sbSP_3,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // Kpr
    connect(ui->dialKpr,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    connect(ui->sbKpr,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // Ti
    connect(ui->dialTi,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    connect(ui->sbTi,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));
    // Td
    connect(ui->dialTd,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    connect(ui->sbTd,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));
    // Xmin
    connect(ui->sbXmin,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // Xmax
    connect(ui->sbXmax,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // K_1
    connect(ui->sbK_1,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // K_2
    connect(ui->sbK_2,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // K_3
    connect(ui->sbK_3,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // K_4
    connect(ui->sbK_4,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // Kkor
    connect(ui->sbKkor,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));

    // AM
    connect(ui->cbAM,SIGNAL(currentIndexChanged(int)),this,SLOT(setParmAM(int)));
    // Rej
    connect(ui->cbRej,SIGNAL(currentIndexChanged(int)),this,SLOT(setParmRej(int)));
    // pRev
    connect(ui->pRev,SIGNAL(clicked()),this,SLOT(setParmRev()));
    connect(ui->rRev,SIGNAL(clicked()),this,SLOT(setParmRev()));
    // Rev
    connect(ui->regRev,SIGNAL(stateChanged(int)),this,SLOT(setParmKprSig(int)));

    // P0
    connect(ui->sbP0,SIGNAL(valueChanged(double)),this,SLOT(setParmValue(double)));
    connect(ui->slP0,SIGNAL(valueChanged(int)),this,SLOT(setParmValue(int)));
    // Mode
    connect(ui->regMode,SIGNAL(clicked(bool)),this,SLOT(setParamMode(bool)));

    connect(ui->cbInterval,SIGNAL(currentIndexChanged(int)),this,SLOT(updateTrend(int)));

    // хеш для співставлення назв віджетів із індексами
    ctrlSearch["cbAM"]=Ri::AM;
    ctrlSearch["cbRej"]=Ri::Rej;

    ctrlSearch["vsSP_1"]=Ri::SP_1;
    ctrlSearch["sbSP_1"]=Ri::SP_1;

    ctrlSearch["vsSP_2"]=Ri::SP_2;
    ctrlSearch["sbSP_2"]=Ri::SP_2;

    ctrlSearch["vsSP_3"]=Ri::SP_3;
    ctrlSearch["sbSP_3"]=Ri::SP_3;

    ctrlSearch["sbKkor"]=Ri::Kkor;

    ctrlSearch["sbK_1"]=Ri::K_1;
    ctrlSearch["sbK_2"]=Ri::K_2;
    ctrlSearch["sbK_3"]=Ri::K_3;
    ctrlSearch["sbK_4"]=Ri::K_4;

    ctrlSearch["dialKpr"]=Ri::Kpr;
    ctrlSearch["sbKpr"]=Ri::Kpr;

    ctrlSearch["dialTi"]=Ri::TI;
    ctrlSearch["sbTi"]=Ri::TI;

    ctrlSearch["dialTd"]=Ri::Td;
    ctrlSearch["sbTd"]=Ri::Td;

    ctrlSearch["sbXmin"]=Ri::Xmin;
    ctrlSearch["sbXmax"]=Ri::Xmax;

    ctrlSearch["slP0"]=Ri::P0;
    ctrlSearch["sbP0"]=Ri::P0;
    ctrlSearch["cbMode"]=Ri::Mode;

    // --------------------------------------------------

    // завантажити дані
    QFile f(cfName);
    if(f.open(QIODevice::ReadOnly))
    {
        for(int i=0;!f.atEnd();++i)
        {
            QStringList sl=QString::fromUtf8(f.readLine()).trimmed().split("\t"); // читаємо дані із файла
            if(sl.size()> 22) // якщо прочитано всі рядки
            {
                ui->regList->addItem(sl[Ri::Deskritp]); // додати до списку регуляторів
                // sl.removeAt(0); // видалити перший елемент бо в ньому Опис регулятора
                RegDes << sl; // зберегти
                //qDebug() << i << sl.size() << sl ;
            }

            else
            {
                qDebug() << "Reg load error " << sl.size() << sl ;
            }
        }
    }
    else
    {
        qDebug() << "panelReg: file not open "  << f.errorString();
    }
    //qDebug() << RegDes;
    
    // запустити таймер поновлення надих. може краще переробити на сигнал ?
    QTimer *t=new QTimer(this);
    t->setInterval(1000);
    connect(t,SIGNAL(timeout()),this,SLOT(updateData()));
    t->start();

    // графік
    QVBoxLayout *tL=new QVBoxLayout(ui->trendChar);
    trChart=new TrendChart(ui->trendChar);
    tL->addWidget(trChart);
    ui->trendChar->setLayout(tL);
    tL->setContentsMargins(4,10,4,30);

    t1 = new QTimer(this);
    t1->setInterval(5000);
    t1->start();
    connect(t1,SIGNAL(timeout()),this,SLOT(setGraph()));

    ui->regList->setCurrentIndex(n);
    connect(ui->regList,SIGNAL(currentIndexChanged(int)),this,SLOT(changeReg(int)));
    changeReg(n);
}

RpanelReg::~RpanelReg()
{

    delete ui;
}

void RpanelReg::changeReg(int Index) // зміна регулятор
{
    RegNum=Index;
    //qDebug() << "RegNum"         << RegNum;

    ui->Value_1->setText(src.getDescription(RegDes[RegNum][Ri::PV_1]));
    ui->Value_2->setText(src.getDescription(RegDes[RegNum][Ri::PV_2]));
    ui->Value_3->setText(src.getDescription(RegDes[RegNum][Ri::PV_3]));
    ui->Valve->setText(src.getDescription(RegDes[RegNum][Ri::X]));

    //ui->min_PV1->setText(QString("%1").arg(src.scaleZero(RegDes[RegNum][Ri::PV_1]),3,'f',0));
    //ui->max_PV1->setText(QString("%1").arg(src.scaleFull(RegDes[RegNum][Ri::PV_1]),3,'f',0));

    ui->scalePV_1->setScaleMinMax(src.scaleZero(RegDes[RegNum][Ri::PV_1]),src.scaleFull(RegDes[RegNum][Ri::PV_1]));


    if(src.scaleFull(RegDes[RegNum][Ri::PV_1])<20)
    {
        ui->sbSP_1->setDecimals(2);
        ui->sbSP_1->setSingleStep(0.01);
    }
    else
    {
        ui->sbSP_1->setDecimals(0);
        ui->sbSP_1->setSingleStep(1);
    }

    // показати-сховати потрібне
    // PV_2
    if(src.getTags().contains(RegDes[RegNum][Ri::PV_2]))
    {
        ui->Parametr_2->show();
        //ui->min_PV2->setText(QString("%1").arg(src.scaleZero(RegDes[RegNum][Ri::PV_2]),3,'f',0));
        //ui->max_PV2->setText(QString("%1").arg(src.scaleFull(RegDes[RegNum][Ri::PV_2]),3,'f',0));
        ui->scalePV_2->setScaleMinMax(src.scaleZero(RegDes[RegNum][Ri::PV_2]),src.scaleFull(RegDes[RegNum][Ri::PV_2]));
    }
    else
    {
        ui->Parametr_2->hide();
    }

    // PV_3
    if(src.getTags().contains(RegDes[RegNum][Ri::PV_3]))
    {
        ui->Parametr_3->show();
        //ui->min_PV3->setText(QString("%1").arg(src.scaleZero(RegDes[RegNum][Ri::PV_3]),3,'f',0));
        //ui->max_PV3->setText(QString("%1").arg(src.scaleFull(RegDes[RegNum][Ri::PV_3]),3,'f',0));
        ui->scalePV_3->setScaleMinMax(src.scaleZero(RegDes[RegNum][Ri::PV_3]),src.scaleFull(RegDes[RegNum][Ri::PV_3]));
    }
    else
    {
        ui->Parametr_3->hide();
    }

    // SPR_1
    if(src.getTags().contains(RegDes[RegNum][Ri::SPR_1]))
    {
        ui->leSPR_1->show();
        ui->vsSPR_1->show();
        ui->labelSPR_1->show();
    }
    else
    {
        ui->leSPR_1->hide();
        ui->vsSPR_1->hide();
        ui->labelSPR_1->hide();
    }

    // SP_2
    if(src.getTags().contains(RegDes[RegNum][Ri::SP_2]))
    {
        ui->sbSP_2->show();
        ui->vsSP_2->show();

    }
    else
    {
        ui->sbSP_2->hide();
        ui->vsSP_2->hide();
	
    }

    // SP_3
    if(src.getTags().contains(RegDes[RegNum][Ri::SP_3]))
    {
        ui->sbSP_3->show();
        ui->vsSP_3->show();
    }
    else
    {
        ui->sbSP_3->hide();
        ui->vsSP_3->hide();

    }

    // K_1
    //qDebug() << RegDes[RegNum][Ri::K_1];
    if(src.getTags().contains(RegDes[RegNum][Ri::K_1]))
    {
        ui->sbK_1->show();
        ui->labelK1->show();
    }
    else
    {
        ui->sbK_1->hide();
        ui->labelK1->hide();
    }

    // K_2
    if(src.getTags().contains(RegDes[RegNum][Ri::K_2]))
    {
        ui->sbK_2->show();
        ui->labelK2->show();
    }
    else
    {
        ui->sbK_2->hide();
        ui->labelK2->hide();
    }

    // K_3
    if(src.getTags().contains(RegDes[RegNum][Ri::K_3]))
    {
        ui->sbK_3->show();
        ui->labelK3->show();
    }
    else
    {
        ui->sbK_3->hide();
        ui->labelK3->hide();
    }

    // K_4
    if(src.getTags().contains(RegDes[RegNum][Ri::K_4]))
    {
        ui->sbK_4->show();
        ui->labelK4->show();
    }
    else
    {
        ui->sbK_4->hide();
        ui->labelK4->hide();
    }

    // Kkor
    if(src.getTags().contains(RegDes[RegNum][Ri::Kkor]))
    {
        ui->sbKkor->show();
        ui->labelKkor->show();
    }
    else
    {
        ui->sbKkor->hide();
        ui->labelKkor->hide();
    }

    // cbRej
    //qDebug() << RegDes[RegNum][Ri::Rej] << src.getTags().contains(RegDes[RegNum][Ri::Rej]);
    if(src.getTags().contains(RegDes[RegNum][Ri::Rej]))
    {
        ui->cbRej->show();
        ui->labelRej->show();
    }
    else
    {
        ui->cbRej->hide();
        ui->labelRej->hide();
    }

// код перенесено із updateData
// ініціалізація контролів, це буде погано працювати коли в мережі будуть кількі клієнтів 
// одночасно працювати із одним регулятором. Ситуація малоймовірна але можлива


//     // SP_1
    ui->sbSP_1->blockSignals(true);

    ui->sbSP_1->setMinimum(src.scaleZero(RegDes[RegNum][Ri::SP_1]));
    ui->sbSP_1->setMaximum(src.scaleFull(RegDes[RegNum][Ri::SP_1]));

    ui->sbSP_1->setValue(src.getValueScaled(RegDes[RegNum][Ri::SP_1]));
    ui->sbSP_1->blockSignals(false);

    ui->vsSP_1->blockSignals(true);
    ui->vsSP_1->setValue(src.getValueFloat(RegDes[RegNum][Ri::SP_1]));
    ui->vsSP_1->blockSignals(false);

    // SP_2
    if(src.getTags().contains(RegDes[RegNum][Ri::SP_2]))
    {
        ui->sbSP_2->blockSignals(true);
        ui->sbSP_2->setMinimum(src.scaleZero(RegDes[RegNum][Ri::SP_2]));
        ui->sbSP_2->setMaximum(src.scaleFull(RegDes[RegNum][Ri::SP_2]));

        ui->sbSP_2->setValue(src.getValueScaled(RegDes[RegNum][Ri::SP_2]));
        ui->sbSP_2->blockSignals(false);
	
        ui->vsSP_2->blockSignals(true);
        ui->vsSP_2->setValue(src.getValueFloat(RegDes[RegNum][Ri::SP_2]));
        ui->vsSP_2->blockSignals(false);
    }
    
    // SP_3
    if(src.getTags().contains(RegDes[RegNum][Ri::SP_3]))
    {
        ui->sbSP_3->show();
        ui->sbSP_3->blockSignals(true);

        ui->sbSP_3->setMinimum(src.scaleZero(RegDes[RegNum][Ri::SP_3]));
        ui->sbSP_3->setMaximum(src.scaleFull(RegDes[RegNum][Ri::SP_3]));

        ui->sbSP_3->setValue(src.getValueScaled(RegDes[RegNum][Ri::SP_3]));
        ui->sbSP_3->blockSignals(false);

        ui->vsSP_3->show();
        ui->vsSP_3->blockSignals(true);
        ui->vsSP_3->setValue(src.getValueFloat(RegDes[RegNum][Ri::SP_2]));
        ui->vsSP_3->blockSignals(false);
    }
    else
    {
        ui->sbSP_3->hide();
        ui->vsSP_3->hide();
    }



    // Kpr
    double t=fabs(src.getValueFloat(RegDes[RegNum][Ri::Kpr]));
    //qDebug() << "Kpr"<< RegDes[RegNum][Ri::Kpr] << ":" << t;
    ui->sbKpr->blockSignals(true);
    ui->sbKpr->setValue(t);
    ui->sbKpr->blockSignals(false);
    if(t>=0.5)
    {
        if(ui->sbKpr->singleStep()!=0.1)
            ui->sbKpr->setSingleStep(0.1);
    }
    else if( t<0.5 && t>=0.2)
    {
        if(ui->sbKpr->singleStep()!=0.05)
            ui->sbKpr->setSingleStep(0.05);
    }
    else
    {
        if(ui->sbKpr->singleStep()!=0.01)
            ui->sbKpr->setSingleStep(0.01);
    }


    ui->dialKpr->blockSignals(true);
    ui->dialKpr->setValue(fabs(t*100.0));
    ui->dialKpr->blockSignals(false);

    ui->regRev->blockSignals(true);
    ui->regRev->setChecked(src.getValueFloat(RegDes[RegNum][Ri::Kpr])<0.0);
    ui->regRev->blockSignals(false);

    // KTi
    //qDebug() << "Ti"<< RegDes[RegNum][Ri::TI] << ":" << src.getValueFloat(RegDes[RegNum][Ri::TI]);
    ui->sbTi->blockSignals(true);
    ui->sbTi->setValue(src.getValueFloat(RegDes[RegNum][Ri::TI])/60.0);
    ui->sbTi->blockSignals(false);

    ui->dialTi->blockSignals(true);
    ui->dialTi->setValue(src.getValueFloat(RegDes[RegNum][Ri::TI])*100.0/60.0);
    ui->dialTi->blockSignals(false);

    // KTd
    //qDebug() << "Td"<< RegDes[RegNum][Ri::Td] << ":" << src.getValueFloat(RegDes[RegNum][Ri::Td]);
    ui->sbTd->blockSignals(true);
    ui->sbTd->setValue(src.getValueFloat(RegDes[RegNum][Ri::Td])/1000.0);
    ui->sbTd->blockSignals(false);

    ui->dialTd->blockSignals(true);
    ui->dialTd->setValue(src.getValueFloat(RegDes[RegNum][Ri::Td]));
    ui->dialTd->blockSignals(false);
    
    // Xmin
    ui->sbXmin->blockSignals(true);
    ui->sbXmin->setValue(src.getValueFloat(RegDes[RegNum][Ri::Xmin])/40.0);
    ui->sbXmin->blockSignals(false);
    
    // Xmax
    ui->sbXmax->blockSignals(true);
    ui->sbXmax->setValue(src.getValueFloat(RegDes[RegNum][Ri::Xmax])/40.0);
    ui->sbXmax->blockSignals(false);
    
    // K_1
    if(src.getTags().contains(RegDes[RegNum][Ri::K_1]))
    {

        kk_1=(src.scaleFull(RegDes[RegNum][Ri::PV_2]) -src.scaleZero(RegDes[RegNum][Ri::PV_2])) /
              (src.scaleFull(RegDes[RegNum][Ri::PV_1]) -src.scaleZero(RegDes[RegNum][Ri::PV_1])) / 100.0;

        ui->sbK_1->blockSignals(true);
        ui->sbK_1->setValue(src.getValueFloat(RegDes[RegNum][Ri::K_1])/kk_1); //треба шкалювати
        ui->sbK_1->blockSignals(false);
    }
    else
        kk_1=1;
    
    
    // K_2
    if(src.getTags().contains(RegDes[RegNum][Ri::K_2]))
    {
        ui->sbK_2->blockSignals(true);
        ui->sbK_2->setValue(src.getValueFloat(RegDes[RegNum][Ri::K_2])); //треба шкалювати
        ui->sbK_2->blockSignals(false);
    }

    // K_3
    if(src.getTags().contains(RegDes[RegNum][Ri::K_3]))
    {
        ui->sbK_3->blockSignals(true);
        ui->sbK_3->setValue(src.getValueFloat(RegDes[RegNum][Ri::K_3])); //треба шкалювати
        ui->sbK_3->blockSignals(false);
    }

    // K_4
    if(src.getTags().contains(RegDes[RegNum][Ri::K_4]))
    {
        ui->sbK_4->blockSignals(true);
        ui->sbK_4->setValue(src.getValueFloat(RegDes[RegNum][Ri::K_4]));
        ui->sbK_4->blockSignals(false);
    }

    // Kkor
    if(src.getTags().contains(RegDes[RegNum][Ri::Kkor]))
    {
        ui->sbKkor->blockSignals(true);
        ui->sbKkor->setValue(src.getValueFloat(RegDes[RegNum][Ri::Kkor]));
        ui->sbKkor->blockSignals(false);
    }

    // X
    ui->sbX->blockSignals(true);
    ui->sbX->setValue(src.getValueScaled(RegDes[RegNum][Ri::X]));
    ui->sbX->blockSignals(false);

    ui->vsX->blockSignals(true);
    ui->vsX->setValue(src.getValueFloat(RegDes[RegNum][Ri::X]));
    ui->vsX->blockSignals(false);


    // AM
    ui->cbAM->blockSignals(true);
    ui->cbAM->setCurrentIndex(src.getValue16(RegDes[RegNum][Ri::AM])?1:0);
    ui->cbAM->blockSignals(false);

    if(src.getValue16(RegDes[RegNum][Ri::AM])) // вимкнути чи ввімкнути управління змінною X
    {
        ui->sbX->blockSignals(true);
        ui->vsX->blockSignals(true);
    }
    else
    {
        ui->sbX->blockSignals(false);
        ui->vsX->blockSignals(false);
    }

    // Rej
    if(src.getTags().contains(RegDes[RegNum][Ri::Rej]))
    {
        ui->cbRej->blockSignals(true);
        ui->cbRej->setCurrentIndex(src.getValue16(RegDes[RegNum][Ri::Rej])?1:0?1:0);
        ui->cbRej->blockSignals(false);
    }

    // Rev
    if(src.getTags().contains(RegDes[RegNum][Ri::Rev]))
    {
        ui->gbRev->show();
        if(src.getValue16(RegDes[RegNum][Ri::Rev]))
        {
            ui->rRev->blockSignals(true);
            ui->rRev->setChecked(true);
            ui->rRev->blockSignals(false);
        }
        else
        {
            ui->pRev->blockSignals(true);
            ui->pRev->setChecked(true);
            ui->pRev->blockSignals(false);
        }
    }
    else
        ui->gbRev->hide();


    // P0
    //qDebug() << RegDes[RegNum][Ri::P0];

    if(src.getTags().contains(RegDes[RegNum][Ri::P0]))
    {
        ui->sbP0->show();
        ui->sbP0->blockSignals(true);
        ui->sbP0->setValue(src.getValueFloat(RegDes[RegNum][Ri::P0])/40.0);
        ui->sbP0->blockSignals(false);

        ui->slP0->show();
        ui->slP0->blockSignals(true);
        ui->slP0->setValue(src.getValueFloat(RegDes[RegNum][Ri::P0]));
        ui->slP0->blockSignals(false);
    }
    else
    {
        ui->slP0->hide();
        ui->sbP0->hide();
    }

    // Mode
    if(src.getTags().contains(RegDes[RegNum][Ri::Mode]))
    {
        ui->regMode->show();
        ui->regMode->setChecked(src.getValue16(RegDes[RegNum][Ri::Mode]));
    }
    else
    {
        ui->regMode->hide();

    }

    updateTrend(ui->cbInterval->currentIndex());
    
    updateData();
    trChart->addPoint(v); //
}

void RpanelReg::updateTrend(int len)
{
    // перезарядити таймер
    int ti[3]={5000,2500,1250};
    t1->stop();
    t1->setInterval(ti[len]);
    t1->start();

    // завантаження даних на графік із історії
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString conName;
    {
        QSettings s;

        if(s.value("/db/hostname","localhost").toString()=="QSQLITE")
        {
            QSqlDatabase dbs=QSqlDatabase::addDatabase("QSQLITE","panelreg");
            dbs.setDatabaseName(s.value("/db/dbname","test").toString());
        }
        else
        {
            QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","panelreg");
            // заточка під drizzle, який створює сокети в /tmp
            if(QFile::exists("/tmp/mysql.socket")) // якщо такий файл існує
            {
                dbs.setConnectOptions("UNIX_SOCKET=/tmp/mysql.socket");
            }

            dbs.setHostName(s.value("/db/hostname","localhost").toString());
            dbs.setDatabaseName(s.value("/db/dbname","test").toString());
            dbs.setUserName(s.value("/db/username","scada").toString());
            dbs.setPassword(s.value("/db/passwd","").toString());
        }

        QSqlDatabase dbs=QSqlDatabase::database("panelreg");

        conName=dbs.connectionName();
        //qDebug() << conName;

        if(  dbs.open())
        {
            const int tm[3]={3600,1800,900},nLen[3]={1,2,4};
            int i,sLen=nLen[len];
            // очистити поточний графік
            trChart->fill(0);

            QDateTime dt = QDateTime::currentDateTime();
            QSqlQuery qry(dbs);
            QString sQuery="SELECT Dt,%1 FROM %4 WHERE Dt BETWEEN %2 AND %3 ORDER BY Dt";
            QString fields=RegDes[RegNum][Ri::PV_1];

            for(i=2;i<9;++i)
            {
                fields+=",";
                if(RegDes[RegNum][i].size() > 0)
                    fields+=RegDes[RegNum][i];
                else
                    fields+="0";
            }
            //qDebug() << sQuery.arg(RegDes[RegNum].field).arg(dt.toTime_t()-3600).arg(dt.toTime_t());
            if(qry.exec(sQuery.arg(fields).arg(dt.toTime_t()-tm[len]).arg(dt.toTime_t()).arg(tblName)))
            {
                while(qry.next())
                {
                    v.clear(); // cюди будуть завантажуватися дані
                    for(i=1;i<9;++i)
                    {
                        v << qry.value(i).toDouble();
                    }
                    for(int n=0;n<sLen;++n) // заватажити точнку декілька раз, в залежністі від вибраної шкали
                        trChart->loadPoint(v);
                }
                qry.clear();
            }
            else
            {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
                QMessageBox::critical(this,tr("!!!Помилка виконання запиту"),qry.lastError().databaseText()+"\n!"+qry.lastQuery());
                qDebug() << qry.lastQuery();
            }


        }
        else
            QMessageBox::critical(this,tr("Не вдалося з\'єднатися із базою даних історії"),dbs.lastError().databaseText());
    }
        QSqlDatabase::removeDatabase(conName);

        QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void RpanelReg::runTrend() // зміна регулятор
{

}

void RpanelReg::Control() // відображення-приховувавння частини вікна з настройками регулятора
{
    if(ui->RegParm->isHidden()) // якщо сховано
    {
        ui->RegParm->show(); // показати область настройки регулятора
        //resize(size()+QSize(0,155)); // збільшити розмір вікна
    }
    else
    {
        ui->RegParm->hide(); // сховати область настройки регулятора
        //resize(size()-QSize(0,155)); // зменшити розмір вікна
    }
    //qDebug() << size().height();
}

void RpanelReg::updateData() // поновлення даних у віджетах
{
    v.clear();
    
    // PV_1
    if(src.scaleFull(RegDes[RegNum][Ri::PV_1])<50.0 && src.scaleFull(RegDes[RegNum][Ri::PV_1])>20.0)
        ui->lePV_1->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_1]),4,'f',1));
    else if (src.scaleFull(RegDes[RegNum][Ri::PV_1])<=20.0)
        ui->lePV_1->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_1]),4,'f',2));
    else
       ui->lePV_1->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_1]),3,'f',0));

    int t1 = src.getValueFloat(RegDes[RegNum][Ri::PV_1]);
    if(t1<0) t1=0;
    if(t1>4000) t1=4000;

    ui->pbPV_1->setValue(t1);

    v << src.getValueFloat(RegDes[RegNum][Ri::PV_1]);
    
    // PV_2
    if(src.getTags().contains(RegDes[RegNum][Ri::PV_2]))
    {
        if(src.scaleFull(RegDes[RegNum][Ri::PV_2])<50.0)
            ui->lePV_2->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_2]),4,'f',1));
	else
            ui->lePV_2->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_2]),3,'f',0));

        int t1 = src.getValueFloat(RegDes[RegNum][Ri::PV_2]);
        if(t1<0) t1=0;
        if(t1>4000) t1=4000;

        ui->pbPV_2->setValue(t1);
        v << src.getValueFloat(RegDes[RegNum][Ri::PV_2]);
    }
    else
        v << 0.0;

    // PV_3
    if(src.getTags().contains(RegDes[RegNum][Ri::PV_3]))
    {
        if(src.scaleFull(RegDes[RegNum][Ri::PV_3])<50.0)
            ui->lePV_3->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_3]),4,'f',1));
	else
            ui->lePV_3->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::PV_3]),3,'f',0));

        int t1 = src.getValueFloat(RegDes[RegNum][Ri::PV_3]);
        if(t1<0) t1=0;
        if(t1>4000) t1=4000;

        ui->pbPV_3->setValue(t1);

        v << src.getValueFloat(RegDes[RegNum][Ri::PV_3]);
    }
    else
        v << 0.0;

    
    // SPR_1
    if(src.getTags().contains(RegDes[RegNum][Ri::SPR_1]))
    {
        if(src.scaleFull(RegDes[RegNum][Ri::SPR_1])<50.0)
            ui->leSPR_1->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::SPR_1]),4,'f',1));
	else
            ui->leSPR_1->setText(QString("%1").arg(src.getValueScaled(RegDes[RegNum][Ri::SPR_1]),3,'f',0));
        ui->vsSPR_1->setValue(qint32(src.getValueFloat(RegDes[RegNum][Ri::SPR_1])));
        v << src.getValueFloat(RegDes[RegNum][Ri::SPR_1]);
    }
    else
        v << 0.0;

    
    // X
    if(ui->cbAM->currentIndex()==1) // якщо в автоматичному режимі то поновити дані
    {
        ui->sbX->setValue(src.getValueScaled(RegDes[RegNum][Ri::X]));
        ui->vsX->setValue(src.getValueFloat(RegDes[RegNum][Ri::X]));
    }
    v << src.getValueFloat(RegDes[RegNum][Ri::X]);

// малювати графіки 
//     // SP_1
    v << src.getValueFloat(RegDes[RegNum][Ri::SP_1]);

    // SP_2
    if(src.getTags().contains(RegDes[RegNum][Ri::SP_2]))
        v << src.getValueFloat(RegDes[RegNum][Ri::SP_2]);
    else
        v << 0.0;
	
    // SP_3
    if(src.getTags().contains(RegDes[RegNum][Ri::SP_3]))
        v << src.getValueFloat(RegDes[RegNum][Ri::SP_3]);
    else
        v << 0.0;

    // qDebug() << "to graph " <<  v;
}

void RpanelReg::setGraph()
{
    trChart->addPoint(v);
}

void RpanelReg::setCtrlValue(double v)
{
    //qDebug() << sender()->objectName() << v;

    src.sendValueScaled(RegDes[RegNum][Ri::X],v);

    ui->vsX->blockSignals(true);
    ui->vsX->setValue((double)v*40.0);
    ui->vsX->blockSignals(false);

}

void RpanelReg::setCtrlValue(int v)
{
    qDebug() << sender()->objectName() << v;
    double td=v;

    src.sendValue(RegDes[RegNum][Ri::X],td);

    ui->sbX->blockSignals(true);
    ui->sbX->setValue(td/40.0);
    ui->sbX->blockSignals(false);

}

void RpanelReg::setParmValue(double v) // слот відправки даних
{
    qDebug() << "setParmValue(double v)" << sender()->objectName() << ":" << v;


    if(ctrlSearch.contains(sender()->objectName())) // пошукати вадправника
  { // якщо не знайдено
    Ri::Index ix=ctrlSearch[sender()->objectName()];
    //qDebug() << "Tag" << RegDes[RegNum][ix];

    switch( ix )
    {
        case Ri::Kpr : // sbKpr
            src.sendValue(RegDes[RegNum][Ri::Kpr],v* (ui->regRev->isChecked()?-1.0:1.0));

            ui->dialKpr->blockSignals(true);
            ui->dialKpr->setValue(v*100.0);
            ui->dialKpr->blockSignals(false);
	    break;
        case Ri::TI : // sbTi
            src.sendValue(RegDes[RegNum][Ri::TI],v*60.0);

            ui->dialTi->blockSignals(true);
            ui->dialTi->setValue(abs(v*100.0));
            ui->dialTi->blockSignals(false);
	    break;
        case Ri::Td : // sbTd
            src.sendValue(RegDes[RegNum][Ri::Td],v*1000.0);

            ui->dialTd->blockSignals(true);
            ui->dialTd->setValue(v*1000.0);
            ui->dialTd->blockSignals(false);
	    break;

        case Ri::Xmin: // sbXmin
        case Ri::Xmax: // sbXmax
            src.sendValue(RegDes[RegNum][ix],v*40.0);
	    break;
        case Ri::K_1: // sbK_1
            src.sendValue(RegDes[RegNum][ix],v*kk_1);
            break;

        case Ri::K_2: // sbK_2
        case Ri::K_3: // sbK_3
        case Ri::K_4: // sbK_4
        case Ri::Kkor: // sbKkor
            //qDebug() << "sendPAram" << RegDes[RegNum][ix];
            src.sendValue(RegDes[RegNum][ix],v);
	    break;

        case Ri::SP_1: // sbSP_1
            src.sendValueScaled(RegDes[RegNum][ix],v);

            ui->vsSP_1->blockSignals(true);
            ui->vsSP_1->setValue((v-src.scaleZero(RegDes[RegNum][ix]))/(src.scaleFull(RegDes[RegNum][ix]) - src.scaleZero(RegDes[RegNum][ix])) *4000.0);
            ui->vsSP_1->blockSignals(false);

	    break;
        case Ri::SP_2: // sbSP_2
            src.sendValueScaled(RegDes[RegNum][ix],v);

            ui->vsSP_2->blockSignals(true);
            ui->vsSP_2->setValue((v-src.scaleZero(RegDes[RegNum][ix]))/(src.scaleFull(RegDes[RegNum][ix]) - src.scaleZero(RegDes[RegNum][ix])) *4000.0);
            ui->vsSP_2->blockSignals(false);
	    break;
        case Ri::SP_3: // sbSP_3
            src.sendValueScaled(RegDes[RegNum][ix],v);

            ui->vsSP_3->blockSignals(true);
            ui->vsSP_3->setValue((v-src.scaleZero(RegDes[RegNum][ix]))/(src.scaleFull(RegDes[RegNum][ix]) - src.scaleZero(RegDes[RegNum][ix])) *4000.0);
            ui->vsSP_3->blockSignals(false);
	    break;
        case Ri::P0: // sbP0
            src.sendValue(RegDes[RegNum][ix],v*40.0);

            ui->slP0->blockSignals(true);
            ui->slP0->setValue(v*40.0);
            ui->slP0->blockSignals(false);
            break;
	default: // якщо щось незрозуміле то не відправляти
            qDebug() << "Index not fount" << sender()->objectName();
	    break;
    }
    src.sendValue("Save",qint16(-1));

  }
  else
      qDebug() << "Sender not found" << sender()->objectName();
}

void RpanelReg::setParmValue(int v)
{
    qDebug() << "setParmValue(int v)" << sender()->objectName() << ":" << v;

  if(ctrlSearch.contains(sender()->objectName())) // пошукати вадправника
  { // якщо не знайдено
    Ri::Index ix=ctrlSearch[sender()->objectName()];
    //qDebug() << "Tag" << RegDes[RegNum][ix];
      switch( ix )
      {
        case Ri::SP_1: // vsSP_1
            src.sendValue(RegDes[RegNum][ix],(double)v);

            ui->sbSP_1->blockSignals(true);
            ui->sbSP_1->setValue(((double)v/4000.0*(src.scaleFull(RegDes[RegNum][ix])-src.scaleZero(RegDes[RegNum][ix])) + src.scaleZero(RegDes[RegNum][ix]) ));
            ui->sbSP_1->blockSignals(false);
	    break;
        case Ri::SP_2: // vsSP_2
            src.sendValue(RegDes[RegNum][ix],(double)v);

            ui->sbSP_2->blockSignals(true);
            ui->sbSP_2->setValue(((double)v/4000.0*(src.scaleFull(RegDes[RegNum][ix])-src.scaleZero(RegDes[RegNum][ix])) + src.scaleZero(RegDes[RegNum][ix]) ));
            ui->sbSP_2->blockSignals(false);
	    break;
        case Ri::SP_3: // vsSP_3
            src.sendValue(RegDes[RegNum][ix],(double)v);

            ui->sbSP_3->blockSignals(true);
            ui->sbSP_3->setValue(((double)v/4000.0*(src.scaleFull(RegDes[RegNum][ix])-src.scaleZero(RegDes[RegNum][ix])) + src.scaleZero(RegDes[RegNum][ix]) ));
            ui->sbSP_3->blockSignals(false);
	    break;
        case Ri::Kpr: // dialKpr
            src.sendValue(RegDes[RegNum][ix],(double)v/100.0*(ui->regRev->isChecked()?-1.0:1.0));

            ui->sbKpr->blockSignals(true);
            ui->sbKpr->setValue((double)v/100.0);
            ui->sbKpr->blockSignals(false);
	    break;
        case Ri::TI: // dialTi
            src.sendValue(RegDes[RegNum][ix],(double)v/100.0*60.0);

            ui->sbTi->blockSignals(true);
            ui->sbTi->setValue((double)v/100.0);
            ui->sbTi->blockSignals(false);
	    break;
        case Ri::Td: // dialTd
            src.sendValue(RegDes[RegNum][ix],(double)v);

            ui->sbTd->blockSignals(true);
            ui->sbTd->setValue((double)v/1000.0);
            ui->sbTd->blockSignals(false);
	    break;
        case Ri::P0:
            src.sendValue(RegDes[RegNum][ix],(double)v);

            ui->sbP0->blockSignals(true);
            ui->sbP0->setValue((double)v/40.0);
            ui->sbP0->blockSignals(false);
            break;

	default: // якщо щось незрозуміле то не відправляти
            qDebug() << sender()->objectName();
	    break;
    }
      src.sendValue("Save",qint16(-1));
  }
  else
      qDebug() << "Sender not found" << sender()->objectName();

}

void RpanelReg::setParmKprSig(int v)
{
    double tmp;

    tmp=ui->sbKpr->value() * (v?-1.0:1.0);

    //qDebug() << sender()->objectName() << tmp;

    src.sendValue(RegDes[RegNum][Ri::Kpr],tmp);
    src.sendValue("Save",qint16(-1));
}


// насту група функцій - заточка під міцубісі
// при створенні бібліотеки треба переробляти
void RpanelReg::setParmAM(int v)
{
    //qDebug() << sender()->objectName() << v;

    if(v==1)
    {
        ui->sbX->blockSignals(true);
        ui->vsX->blockSignals(true);
    }
    else
    {
        ui->sbX->blockSignals(false);
        ui->vsX->blockSignals(false);
    }

    src.sendValue(RegDes[RegNum][Ri::AM],qint16(-v));
    src.sendValue("Save",qint16(-1));
}

void RpanelReg::setParmRej(int v)
{
    //qDebug() << sender()->objectName() << v;
    src.sendValue(RegDes[RegNum][Ri::Rej],qint16(-v));
    src.sendValue("Save",qint16(-1));
}

void RpanelReg::setParmRev()
{
    //qDebug() << sender()->objectName();
    short v=0;

    if(sender()->objectName()=="rRev")
        v=1;

    src.sendValue(RegDes[RegNum][Ri::Rev],qint16(-v));
    src.sendValue("Save",qint16(-1));

}


void RpanelReg::setParamMode(bool v)
{
    src.sendValue(RegDes[RegNum][Ri::Mode],qint16(v?-1:0));
    src.sendValue("Save",qint16(-1));

}

void RpanelReg::closeEvent( QCloseEvent * event) // при закритті зберегти розміри вікна на майбутнє.
{
    QSettings set;
    set.setValue("ranelReg/width",size().width());
    set.setValue("ranelReg/height",size().height());

    event->accept();
}
