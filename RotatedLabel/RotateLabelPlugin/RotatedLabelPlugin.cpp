#include <QtPlugin>
#include "RotatedLabelPlugin.h"
#include "../rotatedlabel.h"


RotatedLabelPlugin::RotatedLabelPlugin(QObject *parent /*= 0*/) : QObject(parent)
{
}

QString RotatedLabelPlugin::name() const
{
    return "RotatedLabel";
}

QString RotatedLabelPlugin::includeFile() const
{
    return "rotatedlabel.h";
}

QString RotatedLabelPlugin::group() const
{
    return "My widgets";
}

QIcon RotatedLabelPlugin::icon() const
{
    return QIcon(":/images/icon.png");
}

QString RotatedLabelPlugin::toolTip() const
{
    return tr("An rotated label");
}

QString RotatedLabelPlugin::whatsThis() const
{
    return tr("This widget is rotated label");
}

bool RotatedLabelPlugin::isContainer() const
{
    return false;
}

QWidget *RotatedLabelPlugin::createWidget(QWidget *parent)
{
    return new RotatedLabel(parent);
}

Q_EXPORT_PLUGIN2(rotatedlabelplugin,RotatedLabelPlugin)

