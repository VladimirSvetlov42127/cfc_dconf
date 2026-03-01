#include "DeviceWidget.h"
#include "gui/forms/main/filesystem_page.h"

#include <qheaderview.h>
#include <qtableview.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qtabwidget.h>
#include <qthread.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qpointer.h>
#include <qtablewidget.h>
#include <qscrollarea.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qsettings.h>

#include <dpc/sybus/channel/channel_types/TcpChannel.h>
#include <journal/JSource.h>
#include <data_model/dc_data_manager.h>

#include <device_operations/DcReadConfigOperation.h>
#include <device_operations/DcWriteConfigOperation.h>
#include <device_operations/DcFormatDriveOperation.h>
#include <device_operations/DcRestartOperation.h>
#include <device_operations/DcInfoOperation.h>
#include <device_operations/DcResetOperation.h>
#include <device_operations/DcUpdateDeviceOperation.h>

#include <gui/editors/EditorsManager.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

#include <gui/forms/main/DcPassportDialog.h>
#include <gui/forms/DcMenu.h>

#include <gui/forms/main/DateTimeWidget.h>

using namespace Dpc::Sybus;
using namespace Dpc::Gui;

namespace {
	const QString STYLESHEET_ERROR = "QLineEdit { background-color: red; }";
	const QString STYLESHEET_WARNING = "QLineEdit { background-color: #ab9f35; }";

	const QSet<int> gSystemManagementIndexs{ 6, 7, 8, 9, 11, 14, 15, 16, 17, 18, 19, 20, 24, 25, 27, 28, 30 };

	class ReportDialog : public QDialog
	{
	public:
		ReportDialog(const QStringList &reportList, QWidget *parent = nullptr)
		{
			setWindowTitle("Предупреждения");
			setWindowIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning)));

			QTableWidget *table = new QTableWidget(this);
			table->setRowCount(reportList.size());
			table->setColumnCount(1);
			table->horizontalHeader()->setStretchLastSection(true);
			table->horizontalHeader()->setHighlightSections(false);
			table->horizontalHeader()->setVisible(false);
			table->setWordWrap(false);

			for (size_t i = 0; i < reportList.size(); i++) {
				auto item = new QTableWidgetItem(reportList.at(i));
				item->setFlags(item->flags() ^ Qt::ItemIsEditable);
				table->setItem(i, 0, item);
			}

			QPushButton *okButton = new QPushButton("Ok");
			connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

			QHBoxLayout *buttonsLayout = new QHBoxLayout;
			buttonsLayout->addStretch();
			buttonsLayout->addWidget(okButton);

			QVBoxLayout *mainLayout = new QVBoxLayout(this);
			mainLayout->addWidget(table);
			mainLayout->addLayout(buttonsLayout);

			resize(800, 400);
		}
	};

	void destroyThread(QThread *thread) {
		if (!thread)
			return;

		thread->quit();
		thread->wait();
		thread->deleteLater();
	}
}

DeviceWidget::DeviceWidget(DcController *controller, QWidget *parent) :
	QWidget(parent),
	m_device(controller),
	m_toolBar(new QToolBar),
	m_deviceInfoWidget(new QWidget),
	m_progressBar(new QProgressBar),
	m_tabWidget(new QTabWidget),
	m_ipAddressEdit(nullptr),
	m_isBusy(false)
{
	m_toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	m_writeAction = m_toolBar->addAction(QIcon(":/icons/32/import.png"), "Записать", this, &DeviceWidget::onWriteButton);
	m_writeAction->setToolTip("Записать конфигурацию в устройство");
	QAction* act = m_toolBar->addAction(QIcon(":/icons/32/rreload.png"), "Обновить", this, &DeviceWidget::onReloadButton);
	act->setToolTip("Обновить информацию об устройстве");
	m_readAction = m_toolBar->addAction(QIcon(":/icons/32/export.png"), "Прочитать", this, &DeviceWidget::onReadButton);
	m_readAction->setToolTip("Прочитать конфигурацию с устройства");


	act = m_toolBar->addAction(QIcon(":/icons/32/factoryset.png"), "Заводские настройки", this, &DeviceWidget::onResetButton);
	act->setToolTip("Сбросить устройство в заводские настройки");

	addToolBarMenu(QIcon(":/icons/32/loading.png"), "Перезагрузка", &DeviceWidget::onRestartButton,
		{ { "Обычный", Channel::HardReset}, {"Минимальный режим", Channel::MinimalModeReset} },
		"Перезагрузка устройсвта в выбраном режиме");

	addToolBarMenu(QIcon(":/icons/32/formatting.png"), "Форматировать", &DeviceWidget::onFormatButton,
		{ { "Том 0", 0}, {"Том 1", 1} }, "Форматировать выбраный том");

	act = m_toolBar->addAction(QIcon(":/icons/32/list.png"), "Паспорт конфигурации", this, &DeviceWidget::onPassport);
	act->setToolTip("Создание отчёта об конфигурации устройства");

    act = m_toolBar->addAction(QIcon(":/icons/32/folder.png"), "Файловая система", this, &DeviceWidget::onFileSystem);
    act->setToolTip("Просмотр файловой системы устройства");

	act = m_toolBar->addAction(QIcon(":/icons/32/factoryset.png"), "Обновление", this, &DeviceWidget::slotUpdateSystem);
	act->setToolTip("Обновление системы устройства");

	auto deviceInfoScrollArea = new QScrollArea;
	deviceInfoScrollArea->setWidgetResizable(true);
	deviceInfoScrollArea->setFrameShape(QFrame::NoFrame);
	deviceInfoScrollArea->setWidget(m_deviceInfoWidget);

	auto deviceInfoGroupBox = new QGroupBox("Информация об устройстве");
	deviceInfoGroupBox->setMinimumWidth(400);
	auto deviceInfoGroupBoxLayout = new QVBoxLayout(deviceInfoGroupBox);
	deviceInfoGroupBoxLayout->setContentsMargins(0, 0, 0, 0);
	deviceInfoGroupBoxLayout->addWidget(deviceInfoScrollArea);

	m_progressBar->setVisible(false);
	m_progressBar->setAlignment(Qt::AlignCenter);
	m_progressBar->setRange(0, 0);	

	auto mainLayout = new QGridLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_toolBar, 0, 1);
	mainLayout->addWidget(deviceInfoGroupBox, 1, 0, 2, 1);
	mainLayout->addWidget(m_progressBar, 1, 1);	
	mainLayout->addWidget(m_tabWidget, 2, 1);
	mainLayout->setColumnStretch(1, 1);

	auto* w = new DateTimeWidget(controller);
	connect(w, &IDeviceTabWidget::newDeviceOperation, this, &DeviceWidget::execOperation);
	m_tabWidget->addTab(w, "Дата/Время");

	setUpdatesEnabled(true);
}

DeviceWidget::~DeviceWidget()
{
}

void DeviceWidget::updateInfo()
{
	for (auto child : m_deviceInfoWidget->findChildren<QWidget*>())
		child->deleteLater();
	delete m_deviceInfoWidget->layout();
	m_ipAddressEdit = nullptr;

	execOperation(new DcInfoOperation(device()), true, false);
}

void DeviceWidget::onReloadButton()
{
	updateInfo();
}

void DeviceWidget::onReadButton()
{
	if (!MsgBox::question(QString("Вы уверены что хотите продолжить? Текущая конфигурация устройства в папке проекта будет полностью удалена, "
		"включая все конфигурационные файлы")))
		return;

	execOperation(new DcReadConfigOperation(device()), true, true);
}

void DeviceWidget::onWriteButton()
{
	if (!MsgBox::question(QString("Вы уверены что хотите продолжить? Все параметры и конфигурационные файлы на устройстве будут перезаписаны")))
		return;

    // Защита от случайного перезаписывания ip-адреса соединения
    if (ChannelType::TCP == channel()->type()) {
        auto connectionIp = channel()->settings()[IpAddrSetting].toString().trimmed();
        if (auto ipParam = device()->getParam(SP_IP4_ADDR, 0); ipParam && ipParam->value().trimmed() != connectionIp)
            if (!MsgBox::question(
                        QString("Текущий IP-адрес устройства %1, отличается от адреса в записываемой конфигурации %2, продолжить?")
                        .arg(connectionIp)
                        .arg(ipParam->value())))
                return;
    }

	execOperation(new DcWriteConfigOperation(device()), true, true);
}

void DeviceWidget::onRestartButton()
{
	auto action = dynamic_cast<QAction*>(sender());
	if (!action)
		return;

	execOperation(new DcRestartOperation(device(), action->data().toInt()), false, false);
}

void DeviceWidget::onFormatButton()
{
	auto action = dynamic_cast<QAction*>(sender());
	if (!action)
		return;

	int drive = action->data().toInt();
	if (!MsgBox::question(QString("Вы уверены что хотите продолжить? все данные на томе %1 будут утеряны безвозвратно").arg(drive)))
		return;

	execOperation(new DcFormatDriveOperation(device(), drive), false, false);
}

void DeviceWidget::onResetButton()
{
	if (!MsgBox::question(QString("Вы уверены что хотите продолжить? Вся конфигурация на устройстве будет сброшена к заводским настройкам")))
		return;

	execOperation(new DcResetOperation(device()), false, false);
}

void DeviceWidget::onProgress(int value)
{
	m_progressBar->setValue(value);
	emit progress(value);
}

void DeviceWidget::onPassport()
{
	DcPassportDialog dlg(device(), this);
	if (QDialog::Rejected == dlg.exec()) return;


	QSettings settings;
	auto filePathKey = QString("PassportConfig/FilePath");
	auto dir = settings.value(filePathKey, "/home").toString();

	//=====================================================================================================================================
	//	05.05.2023	//	Добавление вывода в PDF файлы
	//=====================================================================================================================================
	//QString fileName = QFileDialog::getSaveFileName(this, "Сохранить в файл", dir, "*.html;;*.odt");
	QString fileName = QFileDialog::getSaveFileName(this, "Сохранить в файл", dir, "*.html;;*.odt;;*.pdf");
	//=====================================================================================================================================

	if (fileName.isEmpty()) return;
	settings.setValue(filePathKey, QFileInfo(fileName).absolutePath());

	execOperation(new DcPassportOperation(device(), dlg.structure(), fileName), true, false);

	return;
}

void DeviceWidget::onFileSystem()
{
    auto fsp = FilesystemPage(m_device, this);
    fsp.exec();
    //fsp->deleteLater();
}

void DeviceWidget::slotUpdateSystem()
{
    auto fileName = QFileDialog::getOpenFileName(0, "Выбрать файлы", "", "*.img");
	if (fileName.isEmpty()) {
		return;
	}

	execOperation(new DcUpdateDeviceOperation(device(), fileName), true, true);
}

void DeviceWidget::execOperation(DcIDeviceOperation * operation, bool withProgress, bool lockDeviceTemplate)
{
	if (isBusy())
		return;

	QPointer<QThread> thread = new QThread;
	operation->moveToThread(thread);
	channel()->moveToThread(thread);
	
	connect(operation, &DcIDeviceOperation::infoMsg, this, [](const QString& m, DcController* d) { gJournal.addInfoMessage(m, JSource::make(d)); });
	connect(operation, &DcIDeviceOperation::debugMsg, this, [](const QString& m, int l, DcController* d) { gJournal.addDebugMessage(m, l, JSource::make(d)); });
	connect(operation, &DcIDeviceOperation::warningMsg, this, [](const QString& m, DcController* d) { gJournal.addWarningMessage(m, JSource::make(d)); });
	connect(operation, &DcIDeviceOperation::errorMsg, this, [](const QString& m, DcController* d) { gJournal.addErrorMessage(m, JSource::make(d)); });
	connect(operation, &DcIDeviceOperation::progress, this, &DeviceWidget::onProgress);
	connect(operation, &DcIDeviceOperation::finished, this, [=](int state) {
		channel()->moveToThread(qApp->thread());
		setBusy(false, false, false);

		switch (operation->state()) {
		case DcIDeviceOperation::ErrorState: MsgBox::error("Операция была прервана из-за наличия ошибок"); break;
		case DcIDeviceOperation::WarningState: ReportDialog(operation->reportList(), this).exec(); // no break!
		case DcIDeviceOperation::NoErrorState: onOperationFinished(operation); break;
		}

		operation->deleteLater();
		destroyThread(thread);
	});
	connect(this, &QWidget::destroyed, this, [thread]() { destroyThread(thread);  });

	setBusy(true, withProgress, lockDeviceTemplate);
	thread->start();
	QMetaObject::invokeMethod(operation, &DcIDeviceOperation::start, Qt::QueuedConnection);
}

void DeviceWidget::onOperationFinished(DcIDeviceOperation *operation)
{
	for (size_t i = 0; i < m_tabWidget->count(); i++)
		if (auto w = dynamic_cast<IDeviceTabWidget*>(m_tabWidget->widget(i)); w)
			w->deviceOperationFinished(operation);

	if (auto infoOperation = dynamic_cast<DcInfoOperation*>(operation); infoOperation) {
		onUpdateInfoFinished(infoOperation);
		return;
	}

	if (auto readOperation = dynamic_cast<DcReadConfigOperation*>(operation); readOperation) {
		return;
	}

	if (auto writeOperation = dynamic_cast<DcWriteConfigOperation*>(operation); writeOperation) {
		updateIpAddress(writeOperation->ipAddress());
	}

	if (auto passportOperation = dynamic_cast<DcPassportOperation*>(operation); passportOperation) {
		return;
	}
	
	updateInfo();
}

void DeviceWidget::onUpdateInfoFinished(DcIDeviceOperation * operation)
{
	QVBoxLayout *layout = new QVBoxLayout(m_deviceInfoWidget);
	auto params = dynamic_cast<DcInfoOperation*>(operation)->params();	
	EditorsManager mg(device(), new QGridLayout);

	using EqualFuncType = std::function<bool(const QString&, const QString&)>;
	auto addParamWidget = [=, &params, &mg](int32_t addr, uint16_t index, const QString &label, const QString &stlSheet,
		EqualFuncType equalFunc = EqualFuncType()) {
		auto deviceValue = params[addr].values->value<QString>(index).trimmed();
		auto templateValue = device()->getParam(addr, index)->value().trimmed();
		auto editor = mg.addLineConst(label, deviceValue);
		bool equal = equalFunc ? equalFunc(deviceValue, templateValue) : deviceValue == templateValue;
		if (!equal) {
			editor->setStyleSheet(stlSheet);
			return false;
		}

		return true;
	};

	auto softwareIdEqual = [](const QString &id1, const QString &id2) { return gDataManager.isCompatibleDevices(id1.toInt(), id2.toInt()); };
	
	mg.addLineConst("Название устройства", params[SP_DEVICE_NAME].values->value<QString>());
	addParamWidget(SP_HDWPARTNUMBER, 0, "Код устройства", STYLESHEET_ERROR);
	if (!addParamWidget(SP_SOFTWARE_ID, 0, "Код ПО", STYLESHEET_ERROR, softwareIdEqual)) {
		m_readAction->setEnabled(false);
		m_writeAction->setEnabled(false);
	}
	addParamWidget(SP_SOFTWARE_VERTION, 1, "Версия ПО", STYLESHEET_WARNING);
	addParamWidget(SP_SOFTWARE_VERTION, 0, "Подверсия ПО", STYLESHEET_WARNING);
	addParamWidget(SP_CFGVER, 0, "Версия конфигурации", STYLESHEET_WARNING);
	mg.addLineConst("Серийный номер", params[SP_SERNUM].values->value<QString>());
	auto deviceIp = params[SP_IP4_ADDR].values;
	if (deviceIp)
		m_ipAddressEdit = mg.addLineConst("IP-адрес", deviceIp->value<QString>());

	layout->addLayout(mg.layout());

	mg.setLayout(new QGridLayout);
	mg.setEditorAlignment(Qt::AlignCenter);
	auto smInfo = params[SP_SM_ENTITY_STATUS];
	auto smInfoPack = smInfo.values;
	for (size_t i = 0; i < smInfoPack->count(); i++) {
		if (!smInfoPack->value<uint8_t>(i) || !gSystemManagementIndexs.contains(i))
			continue;

		auto label = new QLabel;
		label->setPixmap(QPixmap(":/icons/32/warning.png").scaled(16, 16));
		mg.addWidget(label, smInfo.names.value(i));
		mg.layout()->setRowMinimumHeight(mg.layout()->rowCount() - 1, 20);
	}

	auto appInfo = params[SP_APP_ENTITY_STATUS];
	auto appInfoPack = appInfo.values;
	for (size_t i = 0; i < appInfoPack->count(); i++) {
		uint32_t val = appInfoPack->value<uint32_t>(i);
		if (!val)
			continue;

		auto editor = mg.addLineConst(appInfo.names.value(i), QString("0b%1").arg(QString::number(val, 2)));
		editor->setAlignment(Qt::AlignCenter);
		mg.layout()->setRowMinimumHeight(mg.layout()->rowCount() - 1, 20);
	}
	layout->addLayout(mg.layout());

	layout->addStretch();
}

void DeviceWidget::updateIpAddress(const QString &newIpAddress)
{
	if (!m_ipAddressEdit)
		return;

	m_ipAddressEdit->setText(newIpAddress);
	//auto connectionIp = channel()->settings()[TcpChannel::IpAddrSetting].toString().trimmed();
	  auto connectionIp = channel()->settings()[IpAddrSetting].toString().trimmed();
	  if (connectionIp != newIpAddress) {
		m_ipAddressEdit->setStyleSheet(STYLESHEET_ERROR);
		m_ipAddressEdit->setToolTip("IP-Адрес устройства и соединения отличаются. Для работы с устройством по TCP создайте новое соединение");
	}
}

DcController * DeviceWidget::device() const
{
	return m_device;
}

ChannelPtr DeviceWidget::channel()
{
	return device()->channel();
}

void DeviceWidget::setBusy(bool busy, bool withProgress, bool lockDeviceTemplate)
{
	m_isBusy = busy;
	m_toolBar->setEnabled(!busy);
	m_tabWidget->setEnabled(!busy);
	m_progressBar->setVisible(busy);
	m_progressBar->setRange(0, withProgress ? 100 : 0);
	if (busy)
		emit beforeOperation(withProgress, lockDeviceTemplate);
	else
		emit afterOperation();
}

bool DeviceWidget::isBusy() const
{
	return m_isBusy;
}

template<class Functor>
void DeviceWidget::addToolBarMenu(const QIcon &icon, const QString &text, Functor functor, const ToolBarMenuItemList &menuItems, const QString &toolTip)
{
	auto button = new QToolButton;
	button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	button->setText(text);
	button->setToolTip(toolTip);
	button->setIcon(icon);
	button->setPopupMode(QToolButton::InstantPopup);
	QMenu *m = new QMenu(button);
	for (auto &it: menuItems) {
		auto action = m->addAction(it.first, this, functor); 
		action->setData(it.second);
		button->setMenu(m);
	}

	m_toolBar->addWidget(button);
}
