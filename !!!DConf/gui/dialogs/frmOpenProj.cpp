#include "frmOpenProj.h"

#include <QDir>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>

#include "data_model/dc_project_description.h"
#include "data_model/dc_data_manager.h"
#include <db/dc_db_manager.h>
#include <utils/cfg_path.h>

using namespace Dpc::Gui;

frmOpenProj::frmOpenProj(DConf *parent)
	: QDialog(parent)
{
    pparent = 0;
	ui.setupUi(this);

	initTableView();
	collectData();
	ui.tblProjectSelect->horizontalHeader()->setStretchLastSection(true);
	ui.tblProjectSelect->setColumnHidden(0, true);
	ui.tblProjectSelect->resizeRowsToContents();
	ui.tblProjectSelect->setColumnWidth(2, 120);			// Колонка "Время изменения", иначе она обрезается и время показывается в две строки
	//ui.tblProjectSelect->resizeColumnsToContents();
	
	if (parent == nullptr)
		return;
	pparent = parent;
}

void frmOpenProj::initTableView(void)
{
	projmodel = new QStandardItemModel();

	QStringList horizontalHeader;
	horizontalHeader.append("Путь");
	horizontalHeader.append("Проект");
	horizontalHeader.append("Время изменения");
	horizontalHeader.append("Автор");
	horizontalHeader.append("Объект");
	projmodel->setHorizontalHeaderLabels(horizontalHeader);

	ui.tblProjectSelect->setModel(projmodel);
	ui.tblProjectSelect->verticalHeader()->hide();
	ui.tblProjectSelect->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tblProjectSelect->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.tblProjectSelect->horizontalHeader()->setStyleSheet("QHeaderView::section {\
			background-color: #7AB1C9;\
			padding-left: 4px;\
			border: 1px solid #999999;\
			height: 25px}");
}

void frmOpenProj::collectData(void)
{
	QDir file(cfg::path::projects());
	if (!file.exists()) {
		QString abs = file.absolutePath();
		return;
	}
	QFileInfoList list = file.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
	int num = 0;
	//for (int i = 0; i < list.size(); ++i) 
	for (int i = list.size()-1; i > -1; --i)
	{
		QFileInfo fileInfo = list.at(i);

		if (!cfg::path::isProjectDir(fileInfo))
			continue;

		getInfoFromFolder(fileInfo.absoluteFilePath());
	}
}

void frmOpenProj::raw(const QString &path, const QString &name, const QString &object, const QString &auth, const QString &desc, const QString &timeupdate)
{
	QList<QStandardItem*> row;
	QStandardItem *ppath = new QStandardItem(path); ppath->setEditable(false);
	QStandardItem *pname = new QStandardItem(name); pname->setEditable(false);
	QStandardItem *pobject = new QStandardItem(object); pobject->setEditable(false);
	QStandardItem *pauth = new QStandardItem(auth); pauth->setEditable(false);
	QStandardItem *ptimeupdate = new QStandardItem(timeupdate); ptimeupdate->setEditable(false);
	row.append(ppath);
	row.append(pname);
	row.append(ptimeupdate);
	row.append(pauth);
	row.append(pobject);
	projmodel->appendRow(row);
}

void frmOpenProj::getInfoFromFolder(const QString &foldername)
{
	QString projpath = QDir(foldername).filePath(cfg::path::getProjectInfoFile());
	DcProjectDescription project;
	if (project.init(projpath))
		raw(foldername, project.name(), project.object(), project.author(), project.desc(), project.timeUpdate());
}

void frmOpenProj::projOpen()
{
	QItemSelectionModel *select = ui.tblProjectSelect->selectionModel();
	if (!select->hasSelection()) {
		MsgBox::warning(QString("Не выбран проект"));
		return;
	}
	projOpen(ui.tblProjectSelect->selectionModel()->currentIndex());
}

void frmOpenProj::projRemove()
{
	QItemSelectionModel *select = ui.tblProjectSelect->selectionModel();
	if (!select->hasSelection()) {
		MsgBox::warning(QString("Не выбран проект для удаления"));
		return;
	}
	
	if (MsgBox::question(QString("Вы действительно хотите удалить выбранный проект?"))) {
		int rowidx = ui.tblProjectSelect->selectionModel()->currentIndex().row();
		QString projPath = projmodel->index(rowidx, 0).data().toString();
		if (cfg::path::curProject().compare(projPath) == 0) {
			gDataManager.topology()->clear();
            gDataManager.controllers()->clear();	// Закрываем все контроллеры
			gDbManager.closeAll();
			gDataManager.projectDesc()->clear();
			cfg::path::setCurProject("");
		}
        cfg::path::remove(projPath);
		projmodel->removeRow(rowidx);
		//this->close();							// Не будем закрывать окно при удалении, вдруг надо удалить несколько, или октрыть проект?
	}
}

void frmOpenProj::projOpen(QModelIndex index) {
	if (!index.isValid())
		return;
	hide();
	QString projPath = projmodel->index(index.row(), 0).data().toString();
	gDataManager.topology()->clear();
	gDataManager.controllers()->clear();	// Закрываем все контроллеры
	//gDbManager.closeAll();
	cfg::path::setCurProject(projPath);
	gDataManager.projectDesc()->clear();

	if (!gDataManager.topology()->fromJson()) {
		MsgBox::warning(QString("Ошибка загрузки конфигурации проекта"));
		cfg::path::setCurProject("");
		return;
	}
	if (!gDataManager.projectDesc()->init(cfg::path::getProjectInfo())) {
		MsgBox::warning(QString("Ошибка загрузки файла описания проекта"));
		cfg::path::setCurProject("");
		return;
	}
	pparent->initProject();
	this->close();
}
