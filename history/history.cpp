#include "history.h"
#include <iodev.h>
#include <trendconstruct.h>
#include "IoNetClient.h"
#include <QUiLoader>

#include <QDebug>
#include <QHBoxLayout>
#include <QList>


UHistorySelect::UHistorySelect(QVector<IoNetClient*> &src,struct trendinfo *tp,QWidget *p /*=NULL*/,QString fileName) :
        QDialog(p),
        s(src),
        TrendParam(tp)
{
    initUi(fileName);

}

// цей конструктор залишено для сумісності із старими програма
UHistorySelect::UHistorySelect(IoNetClient  &source,struct trendinfo *tp,QWidget *p,QString fileName) :
    QDialog(p),
    TrendParam(tp)

{
    s << &source ;
    initUi(fileName);
}

void UHistorySelect::initUi(QString fileName)
{
    QUiLoader *puil= new QUiLoader(this);
    QFile file(fileName);
    QWidget *pwgtForm=puil->load(&file); // завантажити файл із мордою

    if(pwgtForm)
    {
        resize(pwgtForm->size()); // підігнати розміри

        connect(pwgtForm->findChild<QPushButton*>("Exit"),SIGNAL(clicked()),this,SLOT(reject()));
        // QList<QPushButton*> bnts = pwgtForm->findChildren<QPushButton*> ();
        foreach(QPushButton *p,pwgtForm->findChildren<QPushButton*> ())
        {
            if(p->objectName() != "Exit" || p->objectName() != "RunConstruct" )
            {
                connect(p,SIGNAL(clicked()),this,SLOT(slotAccept()));
            }
        }


        QHBoxLayout *bxLayout =new QHBoxLayout;
        bxLayout->addWidget(pwgtForm);
        bxLayout->setMargin(0);
        setLayout(bxLayout);
    }
    setWindowTitle(tr("Вибір графіків"));
}

UHistorySelect::~UHistorySelect()
{

}

void UHistorySelect::slotAccept()
{
    QStringList  param = sender()->objectName().split('_');
    int NnetDev=0,NioDev=0; // значення за замовчанням
    QString table="trend"; // таблиця за замовчанням
    if(param.size()>3) // ящоє не всі потя то заповнити значеннями за намовчанням
    {
        nameTrend=param[0];
        table=param[1];
        NnetDev = param[2].toInt();
        NioDev=param[3].toInt();
    }
    else
    {
        nameTrend=sender()->objectName();
    }
    // тут треба завантажити дані в структуру 

    QFile f(QString(":/text/%1").arg(nameTrend));
    QString t;
    QStringList sl;

  if((s[NnetDev])->myName()=="")
  {
      QMessageBox::critical(this,tr("Проблема"),tr("Невідома назва NetIoDev\nПотрібно встановити параметер myName"));
      reject();
  }
  else
  {
    QCoreApplication::setApplicationName("viparka");
    QSettings set;

    TrendParam->host=set.value("/db/hostname","localhost").toString();
    TrendParam->db=set.value("/db/dbname","viparka").toString();
    TrendParam->user=set.value("/db/username","scada").toString();
    TrendParam->passwd=set.value("/db/passwd","").toString();


    if(f.open(QIODevice::ReadOnly))
    {
        int i;
        sl.clear();

        for(i=0;!f.atEnd() && i<8;++i) // обмежети зчитування із файла кінцем файла або не більше як 8 рядків
	{
                TrendParam->fields[i]=t=QString::fromUtf8(f.readLine()).trimmed(); // прочитати назву поля
                if((*s[NnetDev])[NioDev]->getTags().contains(t)) // якщо задане поле знайдено
		{
                    sl<< /*s.getText()[t].size() > 0 ? */(*s[NnetDev])[NioDev]->getText()[t] /*: t */; // завантажити назву поля, якщо не знайдено - назву тега


                    TrendParam->fScale[i][0]=(*s[NnetDev])[NioDev]->scaleZero(t); // спробувати розпізнати тип поля та/чи значення шкали мінімуму
                    TrendParam->fScale[i][1]=(*s[NnetDev])[NioDev]->scaleFull(t); // спробувати розпізнати тип поля та/чи значення шкали мінімуму

                     if((*s[NnetDev])[NioDev]->fieldType(t)==1) // якщо дискретний сигнал
                    {
			    // змінити тип поля
                            TrendParam->fields[i]=QString("((%1!=0)*454+%2)").arg(t).arg(i*499);
                            // дискретні шкали
			    TrendParam->fScale[i][0]=0.0-1.1*(double)i;
			    TrendParam->fScale[i][1]=8.8-1.1*(double)i;
			}
		}
		else
		{--i;} // можливо і поганий варіант яле якщо такого поля не знайдено тоді змінити лічильник циклів
	}

	TrendParam->numPlot=i; // завантажити кількість графіків
	TrendParam->table=table;
	TrendParam->trend=sender()->objectName(); // Завантажити ім’я тренда
	
	TrendParam->trendHead=qobject_cast<QPushButton*>(sender())->text(); // заголовок тренда - те, що написано на кнопці
	TrendParam->fieldHead = sl;

	//qDebug() << "1 TrendParam->numPlot=" << TrendParam->numPlot;
	//qDebug() << "2 TrendParam->table  =" << TrendParam->table;
	//qDebug() << "3 TrendParam->trend  =" << TrendParam->trend; // Завантажити ім’я тренда
	//for(i=0;i<TrendParam->numPlot;++i)
	//    qDebug() << "4 TrendParam->fields[" << i << "]=" << TrendParam->fields[i];
	
	//for(i=0;i<TrendParam->numPlot;++i)
	//    qDebug() << "5 TrendParam->fScale["<<i<<"] =" << TrendParam->fScale[i][0] << TrendParam->fScale[i][1];
	    
	//qDebug() << "6 TrendParam->trendHead=" << TrendParam->trendHead; // заголовок тренда - те, що написано на кнопці
	//qDebug() << "7 TrendParam->fieldHead =" << TrendParam->fieldHead;

	f.close();
	accept(); // для завершення роботи
    }
    else
	reject(); // якщо не вдалося відкрити відповідний файл
  }

}

void UHistorySelect::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    default:
        break;
    }
}

#ifdef __CONSTRUCTOR__
void UHistorySelect::slotConstruct()
{
    TrendConstruct p(s,this);
    p.exec();
    QStringList list=p.tegList();

    if(list.size())
    {
        int i=0;
        QStringList sl;

        foreach(QString t,list)
        {
            qDebug() << t;
                if(s[0]->getTags().contains(t)) // якщо задане поле знайдено
                {
                    TrendParam->fields[i]=t;
                    sl<< /*s.getText()[t].size() > 0 ? */s[0]->getText()[t] /*: t */; // завантажити назву поля, якщо не знайдено - назву тега

                    TrendParam->fScale[i][0]=s[0]->scaleZero(t); // спробувати розпізнати тип поля та/чи значення шкали мінімуму
                    TrendParam->fScale[i][1]=s[0]->scaleFull(t); // спробувати розпізнати тип поля та/чи значення шкали мінімуму

                     if(s[0]->fieldType(t)==1) // якщо дискретний сигнал
                    {
                            // змінити тип поля
                            TrendParam->fields[i]=QString("((%1!=0)*454+%2)").arg(t).arg(i*499);
                            // дискретні шкали
                            TrendParam->fScale[i][0]=0.0-1.1*(double)i;
                            TrendParam->fScale[i][1]=8.8-1.1*(double)i;
                        }
                     ++i;
                }
                else
                {--i;} // можливо і поганий варіант яле якщо такого поля не знайдено тоді змінити лічильник циклів
        }

        TrendParam->numPlot=i; // завантажити кількість графіків
        TrendParam->table="trend";
        TrendParam->trend="constract"; // Завантажити ім’я тренда

        TrendParam->trendHead=tr("Конструктор"); // заголовок тренда - те, що написано на кнопці
        TrendParam->fieldHead = sl;


        accept(); // для завершення роботи

    }
    else
    {
        reject();
    }

}

#endif
