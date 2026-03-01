#pragma once

#include <QObject>
#include <qtreeview.h>
#include <map>
#include <list>
#include <QStandardItemModel>
#include <qitemselectionmodel.h>
#include "data_model/dc_conf_item.h"
#include "qprocess.h"
#include <gui/forms/DcForm.h>

class DcMenu;

class DcProjModel : public QObject
{
	Q_OBJECT

private:
	QStandardItemModel * model;
	QTreeView *m_tree;
	QObject *m_parent;
	std::map<QStandardItem*, DcConfItem*> m_assocTable;
	QMenu * menuFolder;
	QMenu * menuController;

	int32_t m_controllerToCopy;   // номер контроллера для копирования

	QProcess *processDepParamSystem;
	QProcess *processDView;
	DcFormPtr m_currentForm;

	// Вспомогательные функции
	int32_t getControllerID();			// получаем индекс текущего выбранного контроллера

public:
	DcProjModel(QTreeView *ptree, QObject *parent);
	~DcProjModel();

	void clear();
	void fillTree();
	

public slots:
	void onItemChanged(QStandardItem* pitem);
	void onCustomContextMenu(const QPoint &point);
	void onCustomContextMenuHeader(const QPoint &point);
	void onTreeSelection(const QItemSelection &, const QItemSelection &);

	void slotAddRootSubFolder();
	void slotAddRootSubController();
	void slotExportToDB();

	void slotRenameItem();
	void slotAddFolder();
	void slotAddController();
	void slotRemoveItem();
	void slotCopyController();
	void slotExportController();
	void slotConnectController();
	void slotAuthorizeController();
	void slotImportController();
	void slotPasteController();

	void slotOpenDView();
	void slotOpenDepRaramSystem();

private:
	void fillProjTreeItem(DcConfItem *pitem, QStandardItem* parentItem);
	void removeFromAssocMap(QStandardItem *index);
	void GetAllChildren(QStandardItem *index, std::list<QStandardItem *> &indicies);
	void removeFromDataModel(DcConfItem* pmodelitem);
	void addControllerSubmenu(QStandardItem* parent, DcConfItem* pitemparent);
	void addSubmenuNode(DcMenu *menu, QStandardItem *parent, DcController *controller);
	void addSubmenuNode(int32_t controller, QStandardItem * parent, DcConfItem * pitemparent, QJsonObject &obj);
	QString getItemPath(QStandardItem* item);

	void updateController(int32_t controllerId);
	void updateSignals(DcController *contr, DcController *temp);
	void updateBoards(DcController *contr, DcController *temp);
	void updateAlgs(DcController *contr, DcController *temp);
	void updateParams(DcController *contr, DcController *temp);
	void updateSettings(DcController *contr, DcController *temp);
	void setCurrentForm(const DcFormPtr &form);

    bool loadConfigToProject(int32_t id, const QString &filePath, const QString &name);

private:
	QAction * renameFolderItem;
	QAction * renameControllerItem;
	QAction * removeFolderItem;
	QAction * removeControllerItem;
	QAction * addFolder;
	QAction * addController;
	QAction * addControllerOnline;
	QAction *copyController;
	QAction *exportController;
	QAction *connectController;
	QAction *authorizeController;
	QAction *importController;
	QAction *pasteController;
	
	QAction *openDView;
	QAction *openDepParamSystem;
};

//Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
