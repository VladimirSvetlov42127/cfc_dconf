#include "DConf.h"

#include <qdatetime.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdiriterator.h>
#include <qsettings.h>
#include <qevent.h>
#include <qthread.h>
#include <qdesktopservices.h>

#include <version.h>
#include <journal/JSource.h>
#include <data_model/dc_data_manager.h>
#include <utils/cfg_path.h>
#include <utils/qzipreader_p.h>
#include <utils/qzipwriter_p.h>
#include <db/dc_db_manager.h>

#include <gui/dialogs/frmCreateProj.h>
#include <gui/dialogs/frmOpenProj.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>
#include <dpc/gui/widgets/journal/JournalWidget.h>
#include <updater/updater.h>
#include <dpc/dep_about_box.h>
#include <QLabel>


using namespace Dpc::Gui;

namespace {

#ifdef QT_DEBUG
	const QString HELP_FILE = "../dconf_res/help/result/dconf.chm";
#else
	const QString HELP_FILE = "dconf.chm";
#endif    

//#define VER_PRE "pre.15"
#ifdef VER_PRE
    const QString VERSION = DCONF_VERSION_STRING "-" VER_PRE;
#else
    const QString VERSION = DCONF_VERSION_STRING;
#endif

	const QString APPLICATION_NAME = "DConf";
	const QString APPLICATION_COMPANY = "DEP";
	const QString DEFAULT_IMPORT_DIR = "/home";
	const QString ATTRIBUTE_IMPORT_DIR = "importDir";
	const QString disclaimer = "		ВНИМАНИЕ!!!\n"
		"	В связи с изменением структуры встроенных алгоритмов в устройствах (DEPROTEC, DEPRTU-LT-XX)"
		", конфигурации алгоритмов сделанные на Dconf версии 6.5.1 и ниже, не совместимы с текущей версией."
		" Для восстановления конфигурации встроенных алгоритмов, которые уже загружены в устройства,"
		" нужно обновить прошивки этих устройств и вычитать конфигурацию."
		" Для устройств DEPROTEC до версией ПО не ниже 1.70, для DEPRTU-LT-XX до версией ПО не ниже 2.57.\n\n"
		"	При открытии данного проекта конфигурация проекта будет безвозвратно изменена, рекомендуем сделать копию папки проекта(%1). Продолжить?";
}

DConf::DConf(QWidget *parent)
	: QMainWindow(parent),
	m_updater(new DcUpdater(this)),
	m_updaterSilent(true)
{
	ui.setupUi(this);

	setWindowIcon(QIcon(":/icons/64/d_conf.png"));
	QApplication::setWindowIcon(windowIcon());

	ui.mainToolBar->setIconSize(QSize(32, 32));
	ui.actionCreateProj->setIcon(QIcon(":/icons/32/folder_add.png"));
	ui.actionOpenProj->setIcon(QIcon(":/icons/32/folder.png"));
	ui.actionImport->setIcon(QIcon(":/icons/32/bd_out.png"));
	ui.actionExport->setIcon(QIcon(":/icons/32/bd_in.png"));

	ui.treeProject->setIconSize(QSize(24, 24));	
	auto palette = qApp->palette();
	auto color = palette.color(QPalette::Highlight);
	color.setAlpha(80);
	palette.setColor(QPalette::Highlight, color);
	qApp->setPalette(palette);

	initProjectTree();
	
	auto journalWidget = new JournalWidget;	
	ui.DebugTable->setWidget(journalWidget);
	journalWidget->start();
	gJournal.addInfoMessage(QString("Версия конфигуратора: %1").arg(VERSION));

	QAction *projectViewAction = ui.widgProjectTree->toggleViewAction();
	projectViewAction->setText("Обозреватель проекта");
	ui.menuView->addAction(projectViewAction);

	QAction *journalViewAction = ui.DebugTable->toggleViewAction();
	journalViewAction->setText("Журнал сообщений");
	ui.menuView->addAction(journalViewAction);	

	connect(ui.actionHelp, SIGNAL(triggered()), this, SLOT(openHelp()));

	ui.menu_2->addAction("Обновления", this, [=] { checkForUpdates(false); });

	QCoreApplication::setOrganizationName(APPLICATION_COMPANY);
	QCoreApplication::setApplicationName(APPLICATION_NAME);

	loadSettings();

	connect(m_updater, &DcUpdater::error, this, &DConf::onUpdaterError);
	connect(m_updater, &DcUpdater::updatesAvailableState, this, &DConf::onUpdaterState);
	QMetaObject::invokeMethod(m_updater, &DcUpdater::checkUpdates, Qt::QueuedConnection);

    connect(&gDbManager, &DcDbManager::error, this, &DConf::onErrorDb, Qt::QueuedConnection);

	return;
}

void DConf::initProject()
{
    auto version = gDataManager.projectDesc()->version();
	if (!version) {
		if (!MsgBox::question(disclaimer.arg(cfg::path::curProject()))) {
			projectExit();
			return;
		}

		gDataManager.projectDesc()->setVersion(++version);
		gDataManager.projectDesc()->save(cfg::path::getProjectInfo());
	}

	pmodel->clear();
	if (gDataManager.projectDesc() == nullptr)
		return;

	QString projname = gDataManager.projectDesc()->name();
	if (projname.isEmpty()) {
		MsgBox::error("Имя проекта пустое!");
		return;
	}

	if (gDataManager.topology()->size() < 1) { // если пустой проект
		DcConfItem *proot = new DcConfItem(1, 0, QString("project"), projname, QString(""));
		gDataManager.topology()->add(proot);
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	pmodel->fillTree();
    QApplication::restoreOverrideCursor();
}

void DConf::projCreateSlot()
{
	if (!cfg::path::curProject().isEmpty()) {
		bool exit = MsgBox::question(tr("Вы уверены, что хотите выйти из текущего проекта?"));
		if (exit == false) return;
		projectExit(); }

	frmCreateProj projCreate(this);
	projCreate.exec();
}

void DConf::projOpenSlot()
{
	if (!cfg::path::curProject().isEmpty()) {
		bool exit = MsgBox::question(tr("Вы уверены, что хотите выйти из текущего проекта?"));
		if (exit == false) return;
		projectExit();
	}
	frmOpenProj projOpen(this);
	projOpen.exec();
}

void DConf::appAboutSlot()			// При нажатии "О программе"
{
	DepSettings settings;
	settings.insert(about_icon, QString(":/icons/32/dconf.png"));
	settings.insert(about_name, QString("DConf (64bit)"));
	QString message = QString("Инструментально программное обеспечение DConf для ");
	message += QString("конфигурирования многофункциональных устройств depRTU, ") + '\n';
	message += QString("устройств релейной защиты и автоматики DeProtec, ") + '\n';
	message += QString("комплектов релейных защит и автоматики ExRZA.");
	settings.insert(about_version, VERSION);
	settings.insert(about_message, message);
	DepAboutBox* about_box = new DepAboutBox(settings);
	about_box->exec();
	delete about_box;

	return;
}

void DConf::projImportSlot()
{
	if (!cfg::path::curProject().isEmpty()) {
		bool exit = MsgBox::question(tr("Вы уверены, что хотите выйти из текущего проекта?"));
		if (exit == false) return;
		projectExit(); }

	QString filename = QFileDialog::getOpenFileName(this, "Импорт проекта",	m_importDir, "Проекты (*.dproj);;");
	if (filename.isNull()) {
		// MsgBox::error("Не выбран файл проекта. Импорт невозможен.");
		return; }

	m_importDir = QDir(filename).absolutePath();

	QString proj_path = cfg::path::createProj();
    if (proj_path.isEmpty())
		return;

	QZipReader zip_reader(filename);
	if (zip_reader.exists()) {
		// распаковка архива по указанному пути
		if (!zip_reader.extractAll(proj_path)) {
			QDir projDir(proj_path);
			if (projDir.removeRecursively()) {
				MsgBox::error("Невозможно распаковать архив проекта! \nВозможно, не все временные файлы были удалены. Проверьте путь удаления: \n" + proj_path);
			}
			else
				MsgBox::error("Невозможно распаковать архив проекта!");
			return;
		}
	}
	cfg::path::setCurProject(proj_path);

	gDataManager.topology()->clear();
	gDataManager.projectDesc()->clear();

	if (!gDataManager.topology()->fromJson()) return;
	gDataManager.projectDesc()->init(cfg::path::getProjectInfo());
	initProject();

	return;
}

void DConf::projExportSlot()
{
	QString projPath = cfg::path::curProject();

	if (projPath.isEmpty()) {
		MsgBox::error("Проект не загружен. Экспорт невозможен.");
		return;
	}
	projPath = QDir::fromNativeSeparators(QDir(projPath).absolutePath() + QDir::separator());

	QString filename = QFileDialog::getSaveFileName(
		this,
		"Экспорт проекта",
		m_importDir,
		"Проекты (*.dproj)");

	if (filename.isNull())
		return;
	m_importDir = QFileInfo(filename).dir().absolutePath();


	QZipWriter zip(filename);
	if (zip.status() != QZipWriter::NoError)
		return;

	zip.setCompressionPolicy(QZipWriter::AutoCompress);

	QDirIterator it(projPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString file_path = it.next();
		if (it.fileInfo().isDir()) {
			zip.setCreationPermissions(QFile::permissions(file_path));
			zip.addDirectory(file_path.remove(projPath));
		}
		else if (it.fileInfo().isFile()) {
			QFile file(file_path);
			if (!file.open(QIODevice::ReadOnly))
				continue;

			zip.setCreationPermissions(QFile::permissions(file_path));
			QByteArray ba = file.readAll();
			zip.addFile(file_path.remove(projPath), ba); }
	}
	zip.close();

	return;
}

void DConf::appExitSlot()
{
	bool exit = MsgBox::question(tr("Вы уверены, что хотите выйти из текущего проекта?"));
	if (exit == false) return;

	projectExit();
}

void DConf::openHelp(const QString &context)
{
	auto helpFilePath = QFileInfo(HELP_FILE).absoluteFilePath();
	QDesktopServices::openUrl(QUrl::fromLocalFile(helpFilePath));
}

void DConf::checkForUpdates(bool silent)
{
	m_updaterSilent = silent;
	m_updater->checkUpdates();
}

void DConf::onUpdaterError(const QString& errorMsg)
{
	auto msg = QString("Ошибка сервиса обновлений: %1").arg(errorMsg);
	setStatusBarMsg(msg, QIcon(":/icons/32/error.png"));
	if (!m_updaterSilent)
		MsgBox::error(msg);
}

void DConf::onUpdaterState(bool hasUpdates)
{
	if (!hasUpdates) {
		auto msg = QString("Нет обновлений");
		setStatusBarMsg(msg, QIcon(":/icons/32/ok.png"));
		if (!m_updaterSilent)
			MsgBox::info(msg);
		return;
	}

	setStatusBarMsg("Доступны обновления", QIcon(":/icons/32/info.png"));
	if (!MsgBox::question("Доступно обновление приложения или его компонентов, запустить мастер установки обновлений? Данное приложиение будет закрыто"))
		return;

	if (!m_updater->startUpdates())
		return;

	QThread::msleep(100);
	qApp->exit(0);
}

void DConf::onErrorDb(const QString &msg, const QString &filePath, DcController *device)
{
    if (!device) {
        gJournal.addErrorMessage(QString("%1: %2").arg(msg, filePath));
        return;
    }

    auto source = JSource::make(device);
    gJournal.addErrorMessage(QString("%1: %2").arg(msg, filePath), source);
    gJournal.select(source);
}

void DConf::projectExit()
{
	if (pmodel != nullptr) {
		pmodel->clear();
		delete pmodel;
		pmodel = nullptr;
	}

	this->setCentralWidget(new QWidget);	// Ставим "Затычку" на место пустого виджета mainToolBar


    for(size_t i = 0; i < gDataManager.controllers()->size(); ++i)
        gJournal.removeSource(JSource::make(gDataManager.controllers()->get(i)));

	gDataManager.clearCurrentProgect();
	initProjectTree();
}

void DConf::saveSettings() const
{
	QSettings settings;
	settings.setValue(ATTRIBUTE_IMPORT_DIR, m_importDir);
}

void DConf::loadSettings()
{
	QSettings settings;
	m_importDir = settings.value(ATTRIBUTE_IMPORT_DIR, DEFAULT_IMPORT_DIR).toString();
}

void DConf::setStatusBarMsg(const QString& text, const QIcon& icon)
{
	for (auto ch : ui.statusBar->findChildren<QLabel*>()) {
		ui.statusBar->removeWidget(ch);
		ch->deleteLater();
	}

	auto iconLabel = new QLabel();
	iconLabel->setPixmap(icon.pixmap(16, 16));
	ui.statusBar->addWidget(iconLabel);
	ui.statusBar->addWidget(new QLabel(text));
}

void DConf::closeEvent(QCloseEvent * event)
{
	saveSettings();
	QWidget::closeEvent(event);
}

void DConf::keyPressEvent(QKeyEvent* event)
{
	if (Qt::Key_F1 == event->key())
		openHelp(centralWidget()->objectName());

	QWidget::keyPressEvent(event);
}

void DConf::initProjectTree() {
	pmodel = new DcProjModel(ui.treeProject, this);
}
