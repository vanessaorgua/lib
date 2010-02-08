#include <QObject>
#include <QWidget>
#include <QString>
#include <QIcon>
#include <QDesignerCustomWidgetInterface>

class RotatedLabelPlugin : public QObject , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    RotatedLabelPlugin(QObject *parent = 0);
    
    QString name() const;
    QString includeFile() const;
    QString group() const;
    QIcon icon() const;
    QString toolTip() const;
    QString whatsThis() const;
    bool isContainer() const;
    QWidget *createWidget(QWidget *parent);
};

