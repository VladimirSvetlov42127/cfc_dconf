#include "DcDefaultForm.h"
#include <QLabel>
#include <QGridLayout>

namespace {
}

DcDefaultForm::DcDefaultForm(DcController *controller, const QString &path) : DcForm(controller, path, "Справочная информация")
{
	QGridLayout* layout = new QGridLayout(centralWidget());
    QLabel* label = new QLabel;
    label->setPixmap(QIcon(":/icons/256/dep_logo.png").pixmap(256, 256));
    layout->addWidget(label, 0, 0, Qt::AlignCenter);
}