#include "DcMainForm.h"

#include <qpushbutton.h>
#include <qtoolbar.h>
#include <qfileinfo.h>
#include <qformlayout.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

#include <dpc/gui/dialogs/channel/ChannelsDialog.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

#include <journal/JSource.h>
#include <gui/editors/EditorsManager.h>
#include <gui/forms/main/DeviceWidget.h>

#include <QThread>
#include <QGuiApplication>
#include <QCursor>


using namespace Dpc::Sybus;
using namespace Dpc::Journal;
using namespace Dpc::Gui;

namespace {
	const QString CONTROLLER_CHANNEL_SETTING = "channel";

	namespace Settings
	{
		const QString Company = "company";
		const QString Object = "object";
		const QString Place = "place";
		const QString Connection = "connection";
		const QString Cabinet = "cabinet";
		const QString Implementer = "implementer";
		const QString Date = "date";
		const QString Period = "period";
		const QString Reason = "reason";
	}

	QString settingsFile(DcController *device) 
	{
		return QString("%1/purpose.json").arg(QFileInfo(device->path()).absolutePath());
	}

	QJsonObject loadPurposeSettings(DcController *device)
	{
		QJsonObject obj;
		QFile file(settingsFile(device));
		if (file.open(QIODevice::ReadOnly)) {
			auto jsonDoc = QJsonDocument::fromJson(file.readAll());
			if (!jsonDoc.isNull()) obj = jsonDoc.object(); }

		return obj;
	}
}

namespace Text
{
	const QString Company = "Предприятие";
	const QString Object = "Объект";
	const QString Place = "Место установки";
	const QString Connection = "Присоединение";
	const QString Cabinet = "Устройство/шкаф";
	const QString Implementer = "Исполнитель";
	const QString Date = "Дата выдачи";
	const QString Period = "Срок реализации";
	const QString Reason = "Причина выдачи";
}

DcMainForm::DcMainForm(DcController *controller, const QString &path) :	DcForm(controller, path, "Управление устройством"),
	m_toolBar(new QToolBar),
	m_deviceWidget(new QWidget),
	m_companyEdit(new QLineEdit),
	m_objectEdit(new QLineEdit),
	m_placeEdit(new QLineEdit),
	m_connectionEdit(new QLineEdit),
	m_cabinetEdit(new QLineEdit),
	m_implementerEdit(new QLineEdit),
	m_dateEdit(new QLineEdit),
	m_periodEdit(new QLineEdit),
	m_reasonEdit(new QLineEdit)
{
	m_toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	m_toolBar->addAction(QIcon(":/icons/32/connect_on.png"), "Cоединение", this, &DcMainForm::onConnectButton);
	m_disconnectAction = m_toolBar->addAction(QIcon(":/icons/32/connect_off.png"), "Разъединить",this, &DcMainForm::onDisconnectButton);
	m_disconnectAction->setVisible(false);

	connect(m_companyEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_objectEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_placeEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_connectionEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_cabinetEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_implementerEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_dateEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_periodEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	connect(m_reasonEdit, &QLineEdit::editingFinished, this, &DcMainForm::savePurposeFile);
	loadPurposeFile();

	QGroupBox *templateInfoGroupBox = new QGroupBox("Информация об шаблоне конфигурации");
	EditorsManager mg(controller, new QGridLayout(templateInfoGroupBox));

	auto makeReadOnly = [](ParamEditor *editor) { if (editor) editor->setReadOnly(true); };
	makeReadOnly(mg.addLineEditor(SP_DEVICE_NAME, 0, "Название устройства"));	
	makeReadOnly(mg.addLineEditor(SP_HDWPARTNUMBER, 0, "Код устройства"));
	makeReadOnly(mg.addLineEditor(SP_SOFTWARE_ID, 0, "Код ПО"));
	makeReadOnly(mg.addLineEditor(SP_SOFTWARE_VERTION, 1, "Версия ПО"));
	makeReadOnly(mg.addLineEditor(SP_SOFTWARE_VERTION, 0, "Подверсия ПО"));
	makeReadOnly(mg.addLineEditor(SP_CFGVER, 0, "Версия конфигурации"));
	mg.addVerticalStretch();

	QGroupBox *purposeInfo = new QGroupBox("Назначение устройства");
	QFormLayout *purposeLayout = new QFormLayout(purposeInfo);

	purposeLayout->addRow(Text::Company, m_companyEdit);
	purposeLayout->addRow(Text::Object, m_objectEdit);
	purposeLayout->addRow(Text::Place, m_placeEdit);
	purposeLayout->addRow(Text::Connection, m_connectionEdit);
	purposeLayout->addRow(Text::Cabinet, m_cabinetEdit);
	purposeLayout->addRow(Text::Implementer, m_implementerEdit);
	purposeLayout->addRow(Text::Date, m_dateEdit);
	purposeLayout->addRow(Text::Period, m_periodEdit);
	purposeLayout->addRow(Text::Reason, m_reasonEdit);

	QGridLayout *mainLayout = new QGridLayout(centralWidget());
	mainLayout->addWidget(m_toolBar, 0, 0);	
	mainLayout->addWidget(templateInfoGroupBox, 1, 0, 2, 1);
	mainLayout->addWidget(purposeInfo, 4, 0, 2, 1);
	mainLayout->addWidget(m_deviceWidget, 0, 1, 5, 2);

	mainLayout->setRowStretch(3, 1);
	mainLayout->setColumnStretch(2, 1);
}

bool DcMainForm::isAvailableFor(DcController *controller)
{
	return true;
}

void DcMainForm::fillReport(DcIConfigReport * report)
{
	QFont font("Times New Roman", 16, QFont::Thin);
	report->insertSection("Информация об устройстве", false, &font);
	report->nextLine(2);

	report->insertSection("Назначение");
	auto obj = loadPurposeSettings(report->device());
	DcReportTable table(report->device());
	table.addRow({ Text::Company, obj[Settings::Company].toString() });
	table.addRow({ Text::Object, obj[Settings::Object].toString() });
	table.addRow({ Text::Place, obj[Settings::Place].toString() });
	table.addRow({ Text::Connection, obj[Settings::Connection].toString() });
	table.addRow({ Text::Cabinet, obj[Settings::Cabinet].toString() });
	table.addRow({ Text::Implementer, obj[Settings::Implementer].toString() });
	table.addRow({ Text::Date, obj[Settings::Date].toString() });
	table.addRow({ Text::Period, obj[Settings::Period].toString() });
	table.addRow({ Text::Reason, obj[Settings::Reason].toString() });
	report->insertTable(table);

	report->insertSection("Характеристики");
	table.clear();
	table.addRow(DcReportTable::ParamRecord{ SP_DEVICE_NAME, 0 });
	table.addRow(DcReportTable::ParamRecord{ SP_HDWPARTNUMBER, 0 });
	table.addRow(DcReportTable::ParamRecord{ SP_SOFTWARE_ID, 0 });
	table.addRow(DcReportTable::ParamRecord{ SP_SOFTWARE_VERTION, 1 });
	table.addRow(DcReportTable::ParamRecord{ SP_SOFTWARE_VERTION, 0 });
	table.addRow(DcReportTable::ParamRecord{ SP_CFGVER, 0 });
	table.addRow(DcReportTable::ParamRecord{ SP_SERNUM, 0 });
	report->insertTable(table);
	report->nextLine(2);

	report->insertSection("Уставки", false, &font);
	report->nextLine(2);
};

void DcMainForm::activate()
{
	//if (!controller()->channel())
	//	onConnectButton();
	//else
	//	updateDeviceWidget();
}

void DcMainForm::onConnectButton()
{
    SettingsType channelSettings;
    auto controllerChannelSetting = controller()->settings()->get(CONTROLLER_CHANNEL_SETTING);
    if (controllerChannelSetting) {
        channelSettings = settingsFromString(controllerChannelSetting->value());
    }

    ChannelsDialog dlg(channelSettings, JSource::make(controller()), this);
    if (QDialog::Accepted != dlg.exec()) return;
    controller()->setChannel(dlg.channel());

    channelSettings = controller()->channel()->settings();
    channelSettings[PasswordSetting] = QString();
    QString value = settingsToString(channelSettings);
    controllerChannelSetting = controller()->settings()->get(CONTROLLER_CHANNEL_SETTING);
    if (controllerChannelSetting) {
        controllerChannelSetting->updateValue(value);
    }
    else {
        controllerChannelSetting = new DcSetting(controller()->index(), CONTROLLER_CHANNEL_SETTING, value);
        controller()->settings()->add(controllerChannelSetting, false);
    }
    gJournal.addInfoMessage("Канал связи с устройством успешно инициализирован", JSource::make(controller()));

    //	27.02.2024	//	Соединение с контроллером
    controller()->channel()->connect(true);
    uint8_t rtu_mode = 255;
    Dpc::Sybus::ParamPackPtr pack = controller()->channel()->param(SP_ALTERNATE_MODE);
    if (pack) rtu_mode = pack->value<uint8_t>();
    if (rtu_mode == 0) {
        QString message = "Контроллер работает в режиме RTU-3M. Режим RTU-3M не поддерживается текущей версией DConf. Вы хотите перевести устройство в режим depRTU?";
        bool ask = MsgBox::question(message);
        if (!ask) {
            controller()->channel()->disconnect();
            controller()->setChannel(ChannelPtr());
            auto newDeviceWidget = new QWidget;
            centralWidget()->layout()->replaceWidget(m_deviceWidget, newDeviceWidget);
            m_deviceWidget->deleteLater();
            m_deviceWidget = newDeviceWidget;
            m_disconnectAction->setVisible(false);
            return;
        }

        //	Перевод контроллера в режим depRTU
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        auto pItem = ParamPack::create(T_BYTE, 0x0FE3, 0);
        pItem->appendValue(0);
        auto res = controller()->channel()->setParam(pItem);

        pItem = ParamPack::create(T_BYTE, SP_ALTERNATE_MODE, 0);
        pItem->appendValue(1);
        res = controller()->channel()->setParam(pItem);

        pItem = ParamPack::create(T_BYTE, SP_SAVECFG, 0);
        pItem->appendValue(1);
        res = controller()->channel()->setParam(pItem);
        QThread::sleep(20);

        pItem = ParamPack::create(T_BYTE, SP_RESET_DEVICE, 0);
        pItem->appendValue(1);
        res = controller()->channel()->setParam(pItem);
        QThread::sleep(10);
    }
    QGuiApplication::restoreOverrideCursor();
    controller()->channel()->disconnect();
    //	27.02.2024	//	Отключение от контроллера

    auto newDeviceWidget = new DeviceWidget(controller());
    connect(newDeviceWidget, &DeviceWidget::progress, this, &DcMainForm::onProgress);
    connect(newDeviceWidget, &DeviceWidget::beforeOperation, this, &DcMainForm::onBeforeOperation);
    connect(newDeviceWidget, &DeviceWidget::afterOperation, this, &DcMainForm::onAfterOperation);

    centralWidget()->layout()->replaceWidget(m_deviceWidget, newDeviceWidget);
    m_deviceWidget->deleteLater();
    m_deviceWidget = newDeviceWidget;

    m_disconnectAction->setVisible(true);
    updateDeviceWidget();
}

void DcMainForm::onDisconnectButton()
{
	auto newDeviceWidget = new QWidget;
	centralWidget()->layout()->replaceWidget(m_deviceWidget, newDeviceWidget);
	m_deviceWidget->deleteLater();
	m_deviceWidget = newDeviceWidget;

	controller()->setChannel(ChannelPtr());
	m_disconnectAction->setVisible(false);
}

void DcMainForm::onProgress(int value)
{
	emit progress(value);
}

void DcMainForm::onBeforeOperation(bool withProgress, bool deviceTemplateLocked)
{
	m_toolBar->setEnabled(false);
	emit stateChanged(true, withProgress, deviceTemplateLocked);
}

void DcMainForm::onAfterOperation()
{
	m_toolBar->setEnabled(true);
	emit stateChanged(false, false, false);
	loadPurposeFile();
}

void DcMainForm::loadPurposeFile()
{
	auto obj = loadPurposeSettings(controller());
	m_companyEdit->setText(obj.value(Settings::Company).toString());
	m_objectEdit->setText(obj.value(Settings::Object).toString());
	m_placeEdit->setText(obj.value(Settings::Place).toString());
	m_connectionEdit->setText(obj.value(Settings::Connection).toString());
	m_cabinetEdit->setText(obj.value(Settings::Cabinet).toString());
	m_implementerEdit->setText(obj.value(Settings::Implementer).toString());
	m_dateEdit->setText(obj.value(Settings::Date).toString());
	m_periodEdit->setText(obj.value(Settings::Period).toString());
	m_reasonEdit->setText(obj.value(Settings::Reason).toString());
}

void DcMainForm::savePurposeFile()
{
	QFile file(settingsFile(controller()));
	if (!file.open(QIODevice::WriteOnly)) return;

	QJsonObject obj;
	obj[Settings::Company] = m_companyEdit->text();
	obj[Settings::Object] = m_objectEdit->text();
	obj[Settings::Place] = m_placeEdit->text();
	obj[Settings::Connection] = m_connectionEdit->text();
	obj[Settings::Cabinet] = m_cabinetEdit->text();
	obj[Settings::Implementer] = m_implementerEdit->text();
	obj[Settings::Date] = m_dateEdit->text();
	obj[Settings::Period] = m_periodEdit->text();
	obj[Settings::Reason] = m_reasonEdit->text();
	file.write(QJsonDocument(obj).toJson());
}

void DcMainForm::updateDeviceWidget()
{
	auto dw = dynamic_cast<DeviceWidget*>(m_deviceWidget);
	if (!dw) return;
	dw->updateInfo();
}
