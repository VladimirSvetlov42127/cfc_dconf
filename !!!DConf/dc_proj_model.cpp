#include "dc_proj_model.h"
#include <data_model/dc_data_manager.h>
#include <db/dc_db_manager.h>
#include <qitemselectionmodel.h>
#include <utils/qzipwriter_p.h>
#include <utils/qzipreader_p.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "qdebug.h"
#include <qdiriterator.h>
#include <qstyleditemdelegate.h>
#include <unordered_map>
#include <math.h>

#include <qformlayout.h>
#include <qlineedit.h>
#include <qtimer.h>

#include <report/DcTextDocumentConfigReport.h>
#include <qdesktopservices.h>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>					// Всплывающий окна с ошибками и предупреждениями
#include "DConf.h"
#include <utils/cfg_path.h>

#include <journal/JSource.h>
#include <data_model/storage_for_controller_params/DcController_v2.h>
#include <gui/forms/default/DcDefaultForm.h>
#include <gui/forms/main/DcMainForm.h>
#include <dpc/sybus/ParamAttribute.h>

#include <gui/forms/DcFormFactory.h>
#include <gui/forms/DcMenu.h>

#include <gui/dialogs/add_controller.h>
#include <QPushButton>

//
#include <utils/bindings_update.h>


using namespace std;
using namespace Dpc::Gui;

namespace 
{
#ifdef QT_DEBUG
	const QString DEP_PARAMSYSTEM_PATH = "../3rdparty/DepParamsSystem/DEP_ParmsSystem.exe";
	const QString DVIEW_PATH = "dview.exe";
#else
	const QString DEP_PARAMSYSTEM_PATH	= "DEP_ParmsSystem.exe";
	const QString DVIEW_PATH =	"dview.exe";
#endif

	const int TEMP_CONTROLLER_ID = 9999;
	const QString TEXT_ALREADY_RUNING = "Программа уже запущена!";

    const char* SETTINGS_BOARDS_VERSION = "boards_version";
    const unsigned int BOARDS_UPDATE_STORAGE_VERSION = 2;

	const QString item_project = "project";
	const QString item_folder = "folder";
	const QString item_controller = "controller";
	const QString item_item = "item";

	const QString form_index = "formIdx";

    const QString default_filder_name = "Новая папка";

	class RenameDialog : public QDialog
	{
		QLineEdit *m_edit;
	public:
		RenameDialog(const QString &name, QWidget *parent = nullptr) : QDialog(parent)
		{
			setWindowTitle("Введите имя проекта");
			m_edit = new QLineEdit(name, this);
			m_edit->setSelection(0, name.size());
			QPushButton *okButton = new QPushButton("Ok", this);
			connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

			QHBoxLayout *bLayout = new QHBoxLayout;
			bLayout->addStretch();
			bLayout->addWidget(okButton);

			QVBoxLayout *layout = new QVBoxLayout(this);
			layout->addWidget(m_edit);
			layout->addLayout(bLayout);
		}

		QString name() const { return m_edit->text(); }
	};

	enum {
		ProgressRole = Qt::UserRole + 2,
		ProgressHasValueRole,
		ProgressValueRole
	};


	class ProgressDelegate : public QStyledItemDelegate
	{
		//Q_OBJECT

	public:
		ProgressDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) { }

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
		{
			if (!index.data(ProgressRole).toBool()) {
				QStyledItemDelegate::paint(painter, option, index);				
				return;
			}			

			QStyleOptionProgressBar progressBarOption;
			progressBarOption.state = QStyle::State_Enabled | QStyle::State_Children |
				QStyle::State_Horizontal | QStyle::State_Item;
			progressBarOption.direction = QApplication::layoutDirection();
			progressBarOption.rect = option.rect;
			progressBarOption.fontMetrics = QApplication::fontMetrics();
			progressBarOption.minimum = 0;
			progressBarOption.maximum = index.data(ProgressHasValueRole).toBool() ? 100 : 0;
			progressBarOption.progress = index.data(ProgressValueRole).toInt();
			progressBarOption.text = index.data(Qt::DisplayRole).toString();
			progressBarOption.textAlignment = Qt::AlignCenter;
			progressBarOption.textVisible = true;
			QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
		}
	};

	

	void setC1Face(int32_t controllerIdx)
	{
		DcController *pcontr = gDataManager.controllers()->getById(controllerIdx);
		if (!pcontr)
			return;

        DcSetting *c1Settings = new DcSetting(controllerIdx, "C1", QString::number(1));
		pcontr->settings()->add(c1Settings);
		
		DcParamCfg *param = pcontr->params_cfg()->get(1791, 2);
		if (param)
			param->updateValue(QString::number(1));
		
		param = pcontr->params_cfg()->get(1791, 3);
		if (param)
			param->updateValue(QString::number(2));
	}	

	void setEnabled(QStandardItem *item, bool enabled)
	{
		item->setEnabled(enabled);
		for (size_t i = 0; i < item->rowCount(); i++) {
			auto child = item->child(i);
			if (child)
				setEnabled(child, enabled);
		}
	}
}

DcProjModel::DcProjModel(QTreeView *ptree, QObject *pmain) : m_tree(ptree), m_parent(pmain), m_controllerToCopy(0) {
	
	processDepParamSystem = new QProcess(this);			// Выделение памяти под процесс
	processDView = new QProcess(this);					// Выделение памяти под процесс

	if (m_tree->model()) {
		delete m_tree->model();
	}
	model = new QStandardItemModel(m_tree);
	m_tree->setModel(model);
	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

	menuFolder = new QMenu();	// Элементы меню при нажатии на правую кнопку мыши
	menuController = new QMenu();

	// Определения действий
	renameFolderItem = new QAction("Переименовать", this);					// Действия при нажатии правой кнопки мыши на контроллер
	connect(renameFolderItem, SIGNAL(triggered()), this, SLOT(slotRenameItem()));
	renameControllerItem = new QAction("Переименовать", this);
	connect(renameControllerItem, SIGNAL(triggered()), this, SLOT(slotRenameItem()));
	removeFolderItem = new QAction("Удалить", this);
	connect(removeFolderItem, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
	removeControllerItem = new QAction("Удалить", this);
	connect(removeControllerItem, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
	addFolder = new QAction("Добавить папку", this);
	connect(addFolder, SIGNAL(triggered()), this, SLOT(slotAddFolder()));
	addController = new QAction("Добавить устройство", this);
	connect(addController, SIGNAL(triggered()), this, SLOT(slotAddController()));
	copyController = new QAction("Копировать", this);
	connect(copyController, SIGNAL(triggered()), this, SLOT(slotCopyController()));
	exportController = new QAction("Экспорт", this);
	connect(exportController, SIGNAL(triggered()), this, SLOT(slotExportController()));
	connectController = new QAction("Соединение", this);
	connect(connectController, SIGNAL(triggered()), this, SLOT(slotConnectController()));
	//authorizeController = new QAction("Авторизация", this);
	//connect(authorizeController, SIGNAL(triggered()), this, SLOT(slotAuthorizeController()));
	importController = new QAction("Импорт устройства", this);
	connect(importController, SIGNAL(triggered()), this, SLOT(slotImportController()));
	pasteController = new QAction("Вставить устройство", this);
	connect(pasteController, SIGNAL(triggered()), this, SLOT(slotPasteController()));
	pasteController->setEnabled(false);

	openDView = new QAction("Открыть DView", this);
	connect(openDView, SIGNAL(triggered()), this, SLOT(slotOpenDView()));

	openDepParamSystem = new QAction("Открыть DepParamSystem", this);
	connect(openDepParamSystem, SIGNAL(triggered()), this, SLOT(slotOpenDepRaramSystem()));

	
	
	// Заполняем меню действиями
	menuFolder->addAction(renameFolderItem);
	menuFolder->addSeparator();
	menuFolder->addAction(addFolder);
	menuFolder->addAction(addController);
	menuFolder->addSeparator();
	menuFolder->addAction(importController);
	menuFolder->addAction(pasteController);
	menuFolder->addSeparator();
	menuFolder->addAction(removeFolderItem);
	
	menuController->addAction(renameControllerItem);
	menuController->addSeparator();
	menuController->addAction(copyController);
	menuController->addAction(exportController);
	menuController->addSeparator();
	menuController->addAction(connectController);
	//menuController->addAction(authorizeController);
	menuController->addAction(openDView);					// Открыть DView
	menuController->addAction(openDepParamSystem);			// Открыть Dep_ParamSystem
	menuController->addSeparator();
	menuController->addAction(removeControllerItem);

	if (gDataManager.projectDesc() == nullptr)
		return;

	QString projname = gDataManager.projectDesc()->name();
	if (projname.isEmpty())
		return;

	QStandardItem* itemProject = new QStandardItem(projname);
	model->setHorizontalHeaderItem(0, itemProject);
}

DcProjModel::~DcProjModel()
{
	delete processDepParamSystem;
	delete processDView;

	gDbManager.closeAll();	// Закрываем все контроллеры
}

void DcProjModel::clear()
{
	model->clear();
}

void DcProjModel::fillTree()
{
	m_tree->setItemDelegateForColumn(0, new ProgressDelegate(this));
	m_tree->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
	connect(m_tree->header(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenuHeader(const QPoint &)));
	connect(m_tree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(onTreeSelection(const QItemSelection&, const QItemSelection&)));
	QObject::connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)));
	int rootIndex = 0;
	DcConfItem *pitem = gDataManager.topology()->root();
	if (pitem == nullptr)
		return;
	QStandardItem* itemProject = new QStandardItem(pitem->name());
	itemProject->setIcon(QIcon(":/icons/32/folder_project.png"));
	model->setHorizontalHeaderItem(0, itemProject);
	m_assocTable.insert(std::make_pair(itemProject, pitem));
	list<DcConfItem*> childs = gDataManager.topology()->childs(pitem->id());
	if (childs.empty())
		return;
	for (DcConfItem *pitem : childs) {
		fillProjTreeItem(pitem, itemProject);
	}
}

void DcProjModel::onCustomContextMenuHeader(const QPoint &point) 
{
	QAction *renameAction = new QAction("Переименовать", this);
	connect(renameAction, &QAction::triggered, this, [this]() {
		QStandardItem *projectItem = model->horizontalHeaderItem(0);
		RenameDialog dlg(projectItem->text(), qobject_cast<DConf*>(m_parent)->centralWidget());
		if (dlg.exec() != QDialog::Accepted || dlg.name().isEmpty())
			return;

		projectItem->setText(dlg.name());
		onItemChanged(projectItem);

		gDataManager.projectDesc()->setName(dlg.name());
		gDataManager.projectDesc()->save(cfg::path::getProjectInfo());
	});

	QMenu * menu = new QMenu();
	QAction * addFolder = new QAction("Добавить папку", this);
	connect(addFolder, SIGNAL(triggered()), this, SLOT(slotAddRootSubFolder()));
	QAction * addController = new QAction("Добавить устройство", this);
	connect(addController, SIGNAL(triggered()), this, SLOT(slotAddRootSubController()));
	/*QAction * exportToDb = new QAction("Выгрузить конфигурацию", this);
	connect(exportToDb, SIGNAL(triggered()), this, SLOT(slotExportToDB()));*/
	menu->addAction(renameAction);
	menu->addSeparator();
	menu->addAction(addFolder);
	menu->addAction(addController);
	
	/*menu->addSeparator();
	menu->addAction(exportToDb);*/
	menu->popup(m_tree->viewport()->mapToGlobal(point));
}

void DcProjModel::onCustomContextMenu(const QPoint &point)
{
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;

	QStandardItem* pitem = model->itemFromIndex(index);
	
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	
	QString type = search->second->type();
	if (type.compare(item_folder) == 0) {
		menuFolder->popup(m_tree->viewport()->mapToGlobal(point));
		return;
	}
	else if (type.compare(item_controller) == 0) {
		menuController->popup(m_tree->viewport()->mapToGlobal(point));
		return;
	}
}

void DcProjModel::fillProjTreeItem(DcConfItem *pitem, QStandardItem* parentItem) {
	if (pitem == nullptr)
		return;

	QStandardItem* itemProject = new QStandardItem(pitem->name());
	if (pitem->type().contains(item_folder))
		itemProject->setIcon(QIcon(":/icons/24/folder.png"));
	else if (pitem->type().contains(item_controller)) {
		//itemProject->setIcon(QIcon(":/images/16/unit.png"));
		//
		//
		//auto backColor = QColor("#d6d6d6");
		//backColor.setAlpha(80);
		//itemProject->setBackground(backColor);		
		//
		//
        QString dbpath = cfg::path::controllerFile(pitem->id());
        if (!loadConfigToProject(pitem->id(), dbpath, pitem->name()))
            return;

		updateController(pitem->id());
		addControllerSubmenu(itemProject, pitem);
	}
	/*else
		itemProject->setIcon(QIcon(":/images/info.png"));*/
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	
	if (pitem->parent_id() == 1)
		model->appendRow(itemProject);
	else
		parentItem->appendRow(itemProject);
	m_assocTable.insert(std::make_pair(itemProject, pitem));
	QObject::connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged(QStandardItem*)));
	list<DcConfItem*> childs = gDataManager.topology()->childs(pitem->id());
	if (childs.empty())
		return;
	for (DcConfItem *pitem : childs)
		fillProjTreeItem(pitem, itemProject);
}

// Вызывается при нажатие на любой элемент меню дерева слева
void DcProjModel::onTreeSelection(const QItemSelection &sel, const QItemSelection &dsel)
{
	QModelIndexList vd = sel.indexes();
	if (vd.empty())
		return;

	QModelIndex current = vd.first();
	if (!current.isValid())
		return;

	QStandardItem* pitem = model->itemFromIndex(current);
	auto var = pitem->data();
	if (var.canConvert<DcFormPtr>()) {
		auto form = var.value<DcFormPtr>();
		setCurrentForm(form);
		return;
	}

	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	if (search->second->type().compare(item_folder) == 0)
		return;

	QString item_path = getItemPath(pitem);
	int32_t contrid = search->second->id();
	QString formIdx = search->second->property(form_index);

	DcController *controller = gDataManager.controllers()->getById(contrid);
	if (!controller) {
		MsgBox::error(QString("Не коректный индекс контролера: %1").arg(contrid));
		return;
	}

	DcFormPtr newForm = DcFormFactory::instance().build(formIdx, controller, item_path);
	if (!newForm)
		newForm = std::make_shared<DcDefaultForm>(controller, item_path);

	newForm->setObjectName(formIdx);
	int i = 0;
	auto mainForm = dynamic_cast<DcMainForm*>(newForm.get());
	if (mainForm) {
		pitem->setData(QVariant::fromValue(newForm));
		QTimer *timer = new QTimer(mainForm);
		connect(timer, &QTimer::timeout, this, [=]() mutable { pitem->setData(i++, ProgressValueRole); });
		connect(mainForm, &DcMainForm::stateChanged, this, [=](bool busy, bool withProgress, bool deviceTemplateLocked) {
			for (size_t i = 0; i < pitem->rowCount(); i++) {
				auto child = pitem->child(i);
				if (child)
					setEnabled(child, !busy || !deviceTemplateLocked);
			}

			connect(mainForm, &DcMainForm::progress, this, [=](int value) {pitem->setData(value, ProgressValueRole); });
			pitem->setData(withProgress, ProgressHasValueRole);
			pitem->setData(busy, ProgressRole);	
			if (busy && !withProgress)
				timer->start(50);
			else
				timer->stop();
		});
	}

	setCurrentForm(newForm);
}

QString DcProjModel::getItemPath(QStandardItem* item)
{
	QString item_path;
	QStringList str_list;
	QStandardItem* pos = item;
	if (item->parent() == 0)
		return item->text();
	else {
		str_list.append(item->text());
		while (pos->parent() != 0)
		{
			str_list.append(pos->parent()->text());
			pos = pos->parent();
		}
	}
	//delete pos;

	int count = str_list.count();
	for (int i = str_list.count() - 1; i >= 0; i--)
	{
		if (i == 0)
			item_path = item_path + str_list[i];
		else {
			item_path = item_path + str_list[i] + " | ";
		}
	}
	return item_path;
}

void DcProjModel::updateController(int32_t controllerId)
{	
	DcController *controller = gDataManager.controllers()->getById(controllerId);
	if (!controller)
		return;

    QString templatePath = cfg::path::templateControllerFile(controllerId);

	if (!QFile::exists(templatePath))
		return;

    DcController *templateController = gDbManager.load(TEMP_CONTROLLER_ID, templatePath, "");
    if (!templateController) {
		MsgBox::error(QString("Ошибка чтения шаблона устройства %1").arg(templatePath));
		return;
	}
    gDbManager.close(TEMP_CONTROLLER_ID);

	gDbManager.beginTransaction(controllerId);
	updateSignals(controller, templateController);
	updateBoards(controller, templateController);
	updateAlgs(controller, templateController);	
	updateParams(controller, templateController);
    updateSettings(controller, templateController);
    gDbManager.endTransaction(controllerId);

    //  Загрузка Service Manager
    controller->loadServiceManager();
    delete templateController;
}

void DcProjModel::updateSignals(DcController *contr, DcController* temp) 
{
	// генератор ключа сигнала(внутрений номер, тип, направление, подтип)
	auto makeKey = [](DcSignal* s) { return QString("%1-%2-%3-%4").arg(s->type()).arg(s->direction()).arg(s->subtype()).arg(s->internalId()); };

	// создаем контейнер сигналов из конфигурации шаблона
	std::unordered_map<QString, DcSignal*> tempConfigSignlas;
	for (auto&& t : temp->getSignalList()) {
		tempConfigSignlas.insert({ makeKey(t), t});
	}

	// проходимся по списку сигналов из конфигурации устройства, и ищем соответсвующий сигнал в контейнере из конфигурации шаблона
	// если не находим, то удаляем сигнал из конфигурации устройства.
	// если нашли, то коректируем свойства и имя в сигнале из конфигурации устройства, на соответствующие из сигнала кофнигурации шаблона и 
	// удаляем соответсвующий сигнал из контейнера конфигурации шаблона.
	
	int32_t maxIndex = 0;
	for(auto &&s: contr->getSignalList()) {		
		auto findIt = tempConfigSignlas.find(makeKey(s));
		if (findIt == tempConfigSignlas.end()) {
			contr->removeSignal(s->internalId(), s->type(), s->direction());
			continue;
		}

		auto t = findIt->second;
		if (s->properties() != t->properties()) {
			for (auto pt = t->props().cbegin(); pt != t->props().cend(); pt++)
				s->updateProperty(pt.key(), pt.value());
		}

		if (s->subtype() != DEF_SIG_SUBTYPE_VIRTUAL)
			s->updateName(t->name());

		tempConfigSignlas.erase(findIt);		
		maxIndex = max(maxIndex, s->index());
	}
	maxIndex++;

	// проходимся по всем оставшемся сигналам из контейнера конфигурации шаблона и добавляем их в конфигурацию устройства.
	for (auto &it: tempConfigSignlas) {
		auto t = it.second;
        auto signal = new DcSignal(contr->index(), maxIndex++, t->internalId(), t->direction(), t->type(), t->subtype(), t->name(), t->properties());
		contr->addSignal(signal, false);
	}
}

void DcProjModel::updateBoards(DcController *contr, DcController* temp)
{
    // !!! При чтении параметров плат из файла базы данных конфигурации не обрабатывается ситуация с битыми записями.
    // !!! Потенциально возможна ситуация при которой не получится вставить новую запись из-за наличия записи с таким же 'primary key'

    // Cверка версий плат шаблона и конфигурации
    uint32_t tempBoardsVersion = 0;
    if (auto verSetting = temp->settings()->get(SETTINGS_BOARDS_VERSION); verSetting)
        tempBoardsVersion = verSetting->value().toUInt();

    uint32_t contrBoardsVersion = 0;
    if (auto verSetting = contr->settings()->get(SETTINGS_BOARDS_VERSION); verSetting)
        contrBoardsVersion = verSetting->value().toUInt();

    if (contrBoardsVersion >= tempBoardsVersion)
        return;

    if (contrBoardsVersion < BOARDS_UPDATE_STORAGE_VERSION)
        DcBoard::updateStorage(contr->index());

    // У устройства из конфигурации
    // Запоминаем занчения параметров у интелектуальных(slot > -1) плат(ориентируемся на слоты) и затем удаляем все платы и параметры на них.
    QHash<QString, QString> boardsParamValues;
    QString keyTemplate = "%1-%2-%3";
    for(size_t i = 0; i < contr->boards()->size(); ++i) {
        auto board = contr->boards()->get(i);
        if (board->slot() > -1) {
            for (size_t j = 0; j < board->params()->size(); ++j) {
                auto param = board->params()->get(j);
                auto key = keyTemplate.arg(board->slot()).arg(param->addr()).arg(param->param());
                boardsParamValues[key] = param->value();

                param->remove(); // удалить параметр из базы данных конфигурации
            }

            board->params()->clear(); // удалить все параметры из памяти
        }
    }

    contr->boards()->removeAll();

    // Заполняем платы и их параметры из шаблона.
    // Если встречается параметр, который ранее был на какой то плате(ориентируемся на слоты), востанавливаем его значение
    for(size_t i = 0; i < temp->boards()->size(); ++i) {
        auto board = temp->boards()->get(i);
        auto newBoard = new DcBoard(contr->index(), board->index(), board->slot(), board->type(), board->inst(), board->properties());
        contr->boards()->add(newBoard, false);

        for(size_t j = 0; j < board->params()->size(); ++j) {
            auto param = board->params()->get(j);
            auto key = keyTemplate.arg(board->slot()).arg(param->addr()).arg(param->param());
            auto paramValue = boardsParamValues.value(key, param->value());

            auto newParam = new DcParamCfgBoard(newBoard->uid(), newBoard->index(), param->addr(), param->param(), param->name(), param->type(), param->flags(), paramValue, param->properties());
            newBoard->params()->add(newParam, false);
        }
    }

    // Запись о версии плат из шаблона в конфигурацию
    if (auto verSetting = contr->settings()->get(SETTINGS_BOARDS_VERSION); verSetting) {
        verSetting->updateValue(QString::number(tempBoardsVersion));
        return;
    }

    auto verSettings = new DcSetting(contr->index(), SETTINGS_BOARDS_VERSION, QString::number(tempBoardsVersion));
    contr->settings()->add(verSettings);
}

void DcProjModel::updateAlgs(DcController *contr, DcController* temp)
{
	// удаление
	int32_t maxIndex = 0;
	for (int i = contr->algs_internal()->size() - 1; i >= 0; i--) {
		DcAlgInternal *c = contr->algs_internal()->get(i);
		DcAlgInternal *same = nullptr;
		for (size_t l = 0; l < temp->algs_internal()->size(); l++) {
			DcAlgInternal *t = temp->algs_internal()->get(l);
			if (*t == *c) {
				same = t;
				break;
			}
		}
		if (!same)
			contr->algs_internal()->remove(i);
		else {
			maxIndex = std::max(maxIndex, c->index());
			for (int j = c->ios()->size() - 1; j >= 0; j--) {
				DcAlgIOInternal *cc = c->ios()->get(j);
				DcAlgIOInternal *tt = same->ios()->getById(cc->index());
				if (!tt)
					c->ios()->remove(j);
			}
		}
	}

	// добавление
	maxIndex++;
	for (size_t i = 0; i < temp->algs_internal()->size(); i++) {
		DcAlgInternal *t = temp->algs_internal()->get(i);
		DcAlgInternal *same = nullptr;
		for (size_t l = 0; l < contr->algs_internal()->size(); l++) {
			DcAlgInternal *c = contr->algs_internal()->get(l);
			if (*t == *c) {
				same = c;
				break;
			}
		}
		if (!same) {
            same = new DcAlgInternal(contr->index(), maxIndex++, t->position(), t->name(), t->properties());
			contr->algs_internal()->add(same, false);
		}

		for (size_t j = 0; j < t->ios()->size(); j++) {
			DcAlgIOInternal *tt = t->ios()->get(j);
			if (!same->ios()->getById(tt->index())) {
                DcAlgIOInternal *cc = new DcAlgIOInternal(contr->index(), tt->index(), same->index(), tt->pin(), tt->direction(), tt->name());
				same->ios()->add(cc, false);
			}
		}
	}
}

void DcProjModel::updateParams(DcController *contr, DcController* temp)
{
	DcController_v2 *tempV2 = dynamic_cast<DcController_v2*>(temp);
	DcController_v2 *contrV2 = dynamic_cast<DcController_v2*>(contr);
	if (!tempV2 || !contrV2)
		return;

	// Удаляем параметры которых нет в новом шаблоне. Также удаляем те которые отличаются, но сохраняем их значения.
	QHash<QString, QString> existingParamsAndNotExistingValues;
	QString keyTemplate = "%1-%2";
	auto it = contrV2->headlines()->begin();
	while(it != contrV2->headlines()->end()) {
		auto c = *it;
		auto t = tempV2->getHeadlineForAddress(c->getAddress());
		if (t && (*c == *t)) {
			existingParamsAndNotExistingValues[keyTemplate.arg(c->getAddress())] = QString();
			it++;
			continue;
		}

		for (size_t n = 0; n < c->getProfileCount(); n++)
			for (size_t m = 0; m < c->getSubProfileCount(); m++) {
				int32_t index = n * PROFILE_SIZE + m;
				if (t) {
					QString value = contrV2->params_cfg()->get(c->getAddress(), index)->value();
					existingParamsAndNotExistingValues[keyTemplate.arg(c->getAddress()).arg(index)] = value;
				}							
				contrV2->params_cfg()->remove(c->getAddress(), index);				
			}

		c->remove();
		it = contrV2->headlines()->erase(it);
	}

	// Добавляем все новые параметры из шаблона. Также добавляем заново все измененые параметры, но дя тех которые были раньше востанваливаем старые значения
	for (size_t i = 0; i < tempV2->headlines()->size(); i++) {
		auto t = tempV2->headlines()->at(i);
		auto find = existingParamsAndNotExistingValues.find(keyTemplate.arg(t->getAddress()));
		if (find != existingParamsAndNotExistingValues.end())
			continue;

		auto headline = QSharedPointer<DcParamCfg_v2_Headline>
            (new DcParamCfg_v2_Headline(contrV2->index(), t->getName(), t->getDataType(), t->getAddress(), t->getAttributes(), t->getProfileCount(), t->getSubProfileCount(), t->getDataLenInBytes()));
		headline->insert();
		contrV2->headlines()->push_back(headline);

		for(size_t n = 0; n < t->getProfileCount(); n++)
			for (size_t m = 0; m < t->getSubProfileCount(); m++) {
				int32_t index = n * PROFILE_SIZE + m;
				auto tempParam = tempV2->params_cfg()->get(t->getAddress(), index);
				QString value = existingParamsAndNotExistingValues.value(keyTemplate.arg(t->getAddress()).arg(index), tempParam->value());

				DcParamCfg_v2 *contrParam = new DcParamCfg_v2(contrV2->index(), tempParam->name(), tempParam->addr(), tempParam->param(), value, headline);
				contrV2->params_cfg()->add(contrParam, false);
			}
	}

	auto copyParamValues = [tempV2, contrV2](int addr) {
		auto contrParamHeadline = contrV2->getHeadlineForAddress(addr);
		if (!contrParamHeadline)
			return;

		for(size_t i = 0; i < contrParamHeadline->getProfileCount(); i++)
			for (size_t j = 0; j < contrParamHeadline->getSubProfileCount(); j++) {
				int index = i * PROFILE_SIZE + j;
				auto tempParam = tempV2->params_cfg()->get(addr, index);
				auto contrParam = contrV2->params_cfg()->get(addr, index);
				if (!tempParam || !contrParam)
					continue;

				contrParam->updateValue(tempParam->value());
			}
	};	

	// меняем значения специальных параметров
	for (auto it : DcController::specialParams(DcController::UpdatableParam)) {
		copyParamValues(it);
	}

	auto removeParam = [contrV2](uint16_t addr) {
		auto headLine = contrV2->getHeadlineForAddress(addr);
		if (!headLine)
			return;

		for (size_t i = 0; i < headLine->getProfileCount(); i++)
			for (size_t j = 0; j < headLine->getSubProfileCount(); j++) {
				int index = i * PROFILE_SIZE + j;
				contrV2->params_cfg()->remove(addr, index);
			}

		headLine->remove();
	};

	// удаляем ненужные параметры
	removeParam(SP_PASSWORDMAN);
	removeParam(SP_PASSWORDNET);
}

void DcProjModel::updateSettings(DcController *contr, DcController* temp)
{
	const QStringList settingsToUpdateList { "MASK_BYTES_COUNT" };

	for (size_t i = 0; i < temp->settings()->size(); i++) {
		DcSetting *tempSetting = temp->settings()->get(i);

		if (tempSetting->name() == "cfg_version") // временно отключаем обработку версии конфигурации.
			continue;

		DcSetting *contrSetting = contr->settings()->get(tempSetting->name());
		if (!contrSetting) {
            contrSetting = new DcSetting(contr->index(), tempSetting->name(), tempSetting->value());
			contr->settings()->add(contrSetting);
		}

		if ( settingsToUpdateList.contains(contrSetting->name()) )
			contrSetting->updateValue(tempSetting->value());
	}	
}

void DcProjModel::setCurrentForm(const DcFormPtr & form)
{
	auto pmainfrm = qobject_cast<DConf*>(m_parent);
	pmainfrm->takeCentralWidget();

	m_currentForm = form;
	pmainfrm->setCentralWidget(form.get());	// В главное окно устанавливаем полученный виджет
	gJournal.select(JSource::make(form->controller()));
    m_currentForm->activate();
}

bool DcProjModel::loadConfigToProject(int32_t id, const QString &filePath, const QString &name)
{
    auto deviceConfig = gDbManager.load(id, filePath, name);
    if (!deviceConfig) {
        MsgBox::error(QString("Ошибка загрузки конфигурации устройства из файла: %1").arg(filePath));
        return false;
    }

    gDataManager.controllers()->add(deviceConfig, true);
    return true;
}

void DcProjModel::slotAddRootSubFolder(){
	QStandardItem* pitem = model->horizontalHeaderItem(0);

	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	DcConfItem* pnode = search->second;
	QStandardItem* itemProject = new QStandardItem(default_filder_name);
	itemProject->setIcon(QIcon(":/icons/24/folder.png"));
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	model->appendRow(itemProject);

	int32_t newindex = gDataManager.topology()->maxIndex();
	newindex++;
	DcConfItem* newnode = new DcConfItem(newindex, pnode->id(), item_folder, default_filder_name, QString());
	gDataManager.topology()->add(newnode);
	m_assocTable.insert(std::make_pair(itemProject, newnode));
}

void DcProjModel::slotAddRootSubController() {
	QStandardItem* pitem = model->horizontalHeaderItem(0);

	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	DcConfItem* pnode = search->second;

	int32_t newindex = gDataManager.topology()->maxIndex();
    newindex++;
    AddController dlgcontroller(newindex);
	if (dlgcontroller.exec() != QDialog::Accepted) {
		return;
	}

	QString dbpath = cfg::path::controllerFile(newindex);
	QString contrname = dlgcontroller.model();
	contrname.append(QString("(%1)").arg(newindex));
    if (!loadConfigToProject(newindex, dbpath, contrname))
        return;

	// установить значения кнопок вкл и откл для модификаций DEPROTEC C1
	if (dlgcontroller.isC1())
		setC1Face(newindex);

	QStandardItem* itemProject = new QStandardItem(contrname);
	//itemProject->setIcon(QIcon(":/images/16/unit.png"));
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	//pitem->appendRow(itemProject);
	model->appendRow(itemProject);

	DcConfItem* newnode = new DcConfItem(newindex, pnode->id(), QString(item_controller), contrname, QString(""));
	gDataManager.topology()->add(newnode);
	m_assocTable.insert(std::make_pair(itemProject, newnode));
	addControllerSubmenu(itemProject, newnode);
}

void DcProjModel::slotExportToDB() {
	int j = 0;//TODO
}

void DcProjModel::slotRenameItem() {
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;

	QStandardItem* pitem = model->itemFromIndex(index);
	m_tree->edit(pitem->index());
}

void DcProjModel::slotAddFolder() {
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;

	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	DcConfItem* pnode = search->second;

	QStandardItem* itemProject = new QStandardItem(default_filder_name);
	itemProject->setIcon(QIcon(":/icons/24/folder.png"));
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	pitem->appendRow(itemProject);

	int32_t newindex = gDataManager.topology()->maxIndex();
	newindex++;
	DcConfItem* newnode = new DcConfItem(newindex, pnode->id(), QString(item_folder), QString(default_filder_name), QString(""));
	gDataManager.topology()->add(newnode);
	m_assocTable.insert(std::make_pair(itemProject, newnode));
}

void DcProjModel::slotAddController() {
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;

	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	DcConfItem* pnode = search->second;


	int32_t newindex = gDataManager.topology()->maxIndex();
	newindex++;
    AddController dlgcontroller(newindex);
	if (dlgcontroller.exec() != QDialog::Accepted) {
		return;
	}

	QString dbpath = cfg::path::controllerFile(newindex);
    QString contrname = dlgcontroller.model();
	contrname.append(QString("(%1)").arg(newindex));
    if (!loadConfigToProject(newindex, dbpath, contrname))
        return;

	// установить значения кнопок вкл и откл для модификаций DEPROTEC C1
	if (dlgcontroller.isC1())
		setC1Face(newindex);

	QStandardItem* itemProject = new QStandardItem(contrname);
	//itemProject->setIcon(QIcon(":/images/16/unit.png"));
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	pitem->appendRow(itemProject);
	
	DcConfItem* newnode = new DcConfItem(newindex, pnode->id(), QString(item_controller), contrname, QString(""));
	gDataManager.topology()->add(newnode);
	m_assocTable.insert(std::make_pair(itemProject, newnode));
	addControllerSubmenu(itemProject, newnode);
}

void DcProjModel::slotRemoveItem() { // контекстное меню. 

	bool deleteComtroller = MsgBox::question(tr("Вы уверены, что хотите удалить этот элемент?"));
	if (!deleteComtroller) return;

	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;
	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;

	DcConfItem* pmodelitem = search->second;
	QStandardItem* treemodelitem = search->first;

	removeFromAssocMap(treemodelitem); // циклическое удаление из map всех дочерних элементов
	removeFromDataModel(pmodelitem); // циклическое удаление из модели данных всех дочерних элементов
	m_tree->model()->removeRow(index.row(), index.parent()); // удаление из модели дерева
}

void DcProjModel::slotCopyController()
{
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;
	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;

	DcConfItem* pmodelitem = search->second;
	if (!pmodelitem->type().contains(item_controller))
		return;
	
	m_controllerToCopy = pmodelitem->id();
	pasteController->setEnabled(true);
}

void DcProjModel::slotExportController()
{
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;
	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;

	DcConfItem* pmodelitem = search->second;
	if (!pmodelitem->type().contains(item_controller)) {
		MsgBox::error(QString("Возможно экспортировать только контроллеры"));
		return;
	}

	QString filename = QFileDialog::getSaveFileName(nullptr, "Экспорт Устройства",	QDir::currentPath(), "Проекты (*.dunit)");
	if (filename.isNull())
		return;

	QZipWriter zip(filename);
	if (zip.status() != QZipWriter::NoError)
		return;
	zip.setCompressionPolicy(QZipWriter::AutoCompress);

	QString unitpath = cfg::path::controllerDir(pmodelitem->id());
	QDirIterator it(unitpath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString file_path = it.next();

		QString projDirModify = unitpath;	// Проблема в том, что путь может быть либо с "\", либо с "/". И тогда путь не удалится в file_path.remove(projDirModify) 
		if (projDirModify.contains('/') && projDirModify.contains('\\')) {
			while (projDirModify.contains('\\')) {
				projDirModify[projDirModify.indexOf('\\')] = '/';
			}
		}

		if (it.fileInfo().isDir()) {
			zip.setCreationPermissions(QFile::permissions(file_path));
			zip.addDirectory(file_path.remove(projDirModify));
		}
		else if (it.fileInfo().isFile()) {
			QFile file(file_path);
			if (!file.open(QIODevice::ReadOnly))
				continue;

			zip.setCreationPermissions(QFile::permissions(file_path));
			QByteArray ba = file.readAll();
			zip.addFile(file_path.remove(projDirModify), ba);
		}
	}
	zip.close();
}

void DcProjModel::slotConnectController()
{
	return;

	//QModelIndex index = m_tree->currentIndex();
	//if (!index.isValid())
	//	return;
	//QStandardItem* pitem = model->itemFromIndex(index);
	//auto search = m_assocTable.find(pitem);
	//if (search == m_assocTable.end())
	//	return;

	//DcConfItem* pmodelitem = search->second;
	//int32_t controllerid = pmodelitem->id();
	////qsettingsdlg dlg(controllerid);
	//frmConnection dlg(controllerid);
	//dlg.exec();
}

void DcProjModel::slotAuthorizeController()
{
	/*QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;
	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;

	DcConfItem* pmodelitem = search->second;
	int32_t controllerid = pmodelitem->id();
	qlogindlg dlg(controllerid);
	dlg.exec();*/
}

void DcProjModel::slotImportController()
{
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;
	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;

	DcConfItem* pmodelitem = search->second;
	if (!(pmodelitem->type().contains(item_folder) || pmodelitem->type().contains(item_project))) {
		MsgBox::error(QString("Некорректный тип узла для импорта"));
		return;
	}

	QString filename = QFileDialog::getOpenFileName(nullptr, "Импорт проекта", QDir::currentPath(),	"Проекты (*.dunit)");
	if (filename.isNull()) {
		MsgBox::error("Не выбран файл устройства. Импорт невозможен.");
		return;
	}

	int32_t newindex = gDataManager.topology()->maxIndex();
	newindex++;

	QString controllerpath = cfg::path::controllerDir(newindex);
	QDir controllerdstdir(controllerpath);
	if (controllerdstdir.exists())
        cfg::path::remove(controllerpath);
	QDir().mkdir(controllerpath);

	QZipReader unzip(filename, QIODevice::ReadOnly);
	if (!unzip.exists()) {
		MsgBox::error(QString("Некорректный путь к архиву устройства"));
		//TODO rollback create controller folder
		return;
	}
	
	QList<QZipReader::FileInfo> allFiles = unzip.fileInfoList();
	QZipReader::FileInfo fi;
	foreach(QZipReader::FileInfo fi, allFiles){
		const QString absPath = controllerpath + fi.filePath;
		if (fi.isDir){
			if (!QDir().mkdir(controllerpath+ fi.filePath)) {
				MsgBox::error(QString("Ошибка распаковки архива устройства 1"));
				return;
			}
			if (!QFile::setPermissions(absPath, fi.permissions)) {
				MsgBox::error(QString("Ошибка распаковки архива устройства 2"));
				return;
			}
		}
	}

	foreach(QZipReader::FileInfo fi, allFiles){
		const QString absPath = controllerpath + fi.filePath;
		if (fi.isFile){
			QFile file(absPath);
			if (file.open(QFile::WriteOnly)){
				QApplication::setOverrideCursor(Qt::WaitCursor);
				file.write(unzip.fileData(fi.filePath), unzip.fileData(fi.filePath).size());
				file.setPermissions(fi.permissions);
				QApplication::restoreOverrideCursor();
				file.close();
			}
		}
	}

	unzip.close();

	QString dbpath = controllerpath + cfg::path::controllerFile();
	QString contrname = QFileInfo(filename).fileName();
    if (!loadConfigToProject(newindex, dbpath, contrname))
        return;

	QStandardItem* itemProject = new QStandardItem(contrname);
	//itemProject->setIcon(QIcon(":/images/16/unit.png"));
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	pitem->appendRow(itemProject);

	DcConfItem* newnode = new DcConfItem(newindex, pmodelitem->id(), QString(item_controller), contrname, QString(""));
	gDataManager.topology()->add(newnode);
	m_assocTable.insert(std::make_pair(itemProject, newnode));
	addControllerSubmenu(itemProject, newnode);
}

void DcProjModel::slotPasteController()
{
	if (m_controllerToCopy < 1)
		return;

	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return;

	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	DcConfItem* pnode = search->second;

	if ((pnode->type().compare(item_controller) == 0)||(pnode->type().compare(item_item) == 0))
		return;

	int32_t newindex = gDataManager.topology()->maxIndex();
	newindex++;
	
	QString contrdstpath = cfg::path::controllerDir(newindex);
	QString contrsrcpath = cfg::path::controllerDir(m_controllerToCopy);
	QDir controllerdstdir(contrdstpath);
	if (controllerdstdir.exists())
        cfg::path::remove(contrdstpath);
	QDir().mkdir(contrdstpath);
    if (!cfg::path::copy(contrsrcpath, contrdstpath)) {
		MsgBox::error(QString("Ошибка копирования устройства. Не удалось скопировать файлы конфигурации"));
		return;
	}

	QString dbpath = contrdstpath + cfg::path::controllerFile();
	QString contrname = gDataManager.controllers()->getById(m_controllerToCopy)->name() + "(" + QString::number(newindex) + ")";
    if (!loadConfigToProject(newindex, dbpath, contrname))
        return;

	QStandardItem* itemProject = new QStandardItem(contrname);
	//itemProject->setIcon(QIcon(":/images/16/unit.png"));
	itemProject->setFlags(itemProject->flags() | Qt::ItemIsEditable);
	pitem->appendRow(itemProject);

	DcConfItem* newnode = new DcConfItem(newindex, pnode->id(), QString(item_controller), contrname, QString(""));
	gDataManager.topology()->add(newnode);
	m_assocTable.insert(std::make_pair(itemProject, newnode));
	addControllerSubmenu(itemProject, newnode);
	
	m_controllerToCopy = 0;
	pasteController->setEnabled(false);
}


void DcProjModel::slotOpenDView(){

	if (processDView->state() == QProcess::Running) {
		MsgBox::warning(TEXT_ALREADY_RUNING);
		return;
	}
	
	uint32_t controllerid = getControllerID();				// Получаем ID выбранного контроллера
	DcController *pcontroller = gDataManager.controllers()->getById(controllerid);	// Получаем контроллер
	if (pcontroller == nullptr) {
		MsgBox::warning("Ошибка. Нельзя обратиться к текущему устройству!");
		return;
	}

	processDView->start(DVIEW_PATH);

	//if (pcontroller->channel() == nullptr) {			// Если нет соединения, запускаем без параметров
	//	processDView->start(DVIEW_EXE_DIR);				// Путь к exe. Без русских букв и пробелов
	//}
	//else {

	//	ChannelPtr connectionClass = pcontroller->channel();				// Получаем базовый класс для коннекта
	//	QStringList arguments;													// список аргументов

	//	switch (connectionClass->type()) {
	//	case Channel::TCP: {
	//			TcpChannel* tcpClass = dynamic_cast <TcpChannel*>(connectionClass.get());
	//			QString host = "/host=" + tcpClass->ipAddress();		// Преобразование строки
	//			QString port = "/port=" + QString::number(tcpClass->port());
	//			arguments << "/iface = tcp " << host << port;
	//			break;
	//		}

	//		case Channel::VCOM: {
	//			VComChannel* comClass = dynamic_cast <VComChannel*>(connectionClass.get());
	//			QString port = "/port=" + QString::number(comClass->getComInd());
	//			arguments << "/iface=vcom" << port;
	//			break;
	//		}

	//		case Channel::RS485vTCP: {
	//			TcpRs485Channel *c = dynamic_cast <TcpRs485Channel*>(connectionClass.get());
	//			QString host = "/host=" + c->ipAddress();		// Преобразование строки
	//			QString port = "/port=" + QString::number(c->port());
	//			QString rs485Port = "/rs485port="; // +QString::number(c->boardPort());
	//			QString rs485Device = "/rs485device="; // +QString::number(c->rs485Device());
	//			arguments << "/iface = rs485vtcp " << host << port << rs485Port << rs485Device;
	//			break;
	//		}
	//	}
	//	processDView->start(DVIEW_EXE_DIR, arguments);				// Путь к exe. Без русских букв и пробелов
	//}
}

void DcProjModel::slotOpenDepRaramSystem() {

	if (processDepParamSystem->state() == QProcess::NotRunning) {		// Если приложение еще не запущено
		processDepParamSystem->start(DEP_PARAMSYSTEM_PATH);				// Путь к exe. Без русских букв и пробелов
		if (!processDepParamSystem->waitForStarted())
			MsgBox::error(QString("%1").arg(processDepParamSystem->errorString()));
	}
	else {		// Если запускается или уже работает 
		MsgBox::warning(TEXT_ALREADY_RUNING);
	}

}

void DcProjModel::onItemChanged(QStandardItem* pitem) {
	if (pitem == nullptr)
		return;

	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return;
	QString newname = search->first->text();
	if (newname.isEmpty())
		return;
	if (newname.compare(search->second->name()) == 0)// если имя не менялось - ничего не делаем
		return;
	search->second->updateName(newname);
	//update topology in json-file
	gDataManager.topology()->update();
	gDataManager.topology()->toJson();

	if (search->second->type() != item_controller)
		return;
	
	DcController *controller = gDataManager.controllers()->getById(search->second->id());
	if (!controller) {
		MsgBox::error(QString("Не коректный контролера"));
		return;
	}

	controller->updateName(newname);
}

void DcProjModel::GetAllChildren(QStandardItem *index, list<QStandardItem *> &indicies)
{
	auto search = m_assocTable.find(index);
	indicies.push_back(index);
	int rowcount = index->rowCount();
	for (int i = 0; i < rowcount; i++) {
		GetAllChildren(index->takeChild(i), indicies);
	}
}

void DcProjModel::removeFromAssocMap(QStandardItem *index) {
	list<QStandardItem*> list;
	GetAllChildren(index, list);

	if (list.empty())
		return;
	for (QStandardItem* indextmp : list) {
		auto search = m_assocTable.find(indextmp);
		m_assocTable.erase(indextmp);
	}
}

void DcProjModel::removeFromDataModel(DcConfItem* pmodelitem) {
	list<DcConfItem*> childs = gDataManager.topology()->childs(pmodelitem->id());
	if (pmodelitem->type().compare(item_controller) == 0) {
        if (auto controller = gDataManager.controllers()->getById(pmodelitem->id()); controller)
            gJournal.removeSource(JSource::make(controller));

		gDataManager.controllers()->removeById(pmodelitem->id());
		gDbManager.close(pmodelitem->id());
        cfg::path::removeController(pmodelitem->id());
	}

	gDataManager.topology()->removeById(pmodelitem->id());

	if (childs.empty())
		return;
	for (DcConfItem *tmpelem : childs)
		removeFromDataModel(tmpelem);
}

void DcProjModel::addSubmenuNode(int32_t controller, QStandardItem * parent, DcConfItem * pitemparent, QJsonObject &obj) {
	QString title = obj["title"].toString();
	QString desc = obj["desc"].toString();
	QString reftype = obj["reftype"].toString();
	QString ref = obj["ref"].toString();
	QString icon = obj["icon"].toString();

	QStandardItem* itemProject = new QStandardItem(title);
	itemProject->setIcon(QIcon(icon));
	itemProject->setEditable(false);
	parent->appendRow(itemProject);

	DcConfItem *item = new DcConfItem(0, 0, QString(item_item), title, desc);
	m_assocTable.insert(std::make_pair(itemProject, item));
	item->updateProperty("ref", ref);
	item->updateProperty("reftype", reftype);
	item->updateProperty("controller", QString::number(controller));
	QJsonArray jsonproparray = obj["properties"].toArray(); // дописать свойства
	foreach(const QJsonValue & value, jsonproparray) {
		QJsonObject obj2 = value.toObject();
		QJsonObject::iterator it;
		for (it = obj2.begin(); it != obj2.end(); it++) {
			QString key = it.key();
			QString value = it.value().toString();
			item->updateProperty(key, value);
		}
	}

	QJsonArray jsonsubarray = obj["submenu"].toArray();
	foreach(const QJsonValue & value, jsonsubarray) {
		QJsonObject obj = value.toObject();
		addSubmenuNode(controller, itemProject, item, obj);
	}
}

void DcProjModel::addSubmenuNode(DcMenu * menu, QStandardItem * parent, DcController * controller)
{
	if (!menu->isAvailableFor(controller))
		return;

	QStandardItem* projectItem = new QStandardItem(menu->icon(), menu->title());
	projectItem->setEditable(false);
	parent->appendRow(projectItem);

	DcConfItem *item = new DcConfItem(controller->index(), 0, QString(item_item), QString(), QString());
	item->updateProperty(form_index, menu->formIdx());
	m_assocTable.insert(std::make_pair(projectItem, item));

	for (auto &it : menu->childs())
		addSubmenuNode(it.second, projectItem, controller);
}

void DcProjModel::addControllerSubmenu(QStandardItem * parent, DcConfItem * pitemparent)
{
	DcController *controller = gDataManager.controllers()->getById(pitemparent->id());
	if (!controller)
		return;

	switch (controller->type())
	{
	case DcController::Deprotec: parent->setIcon(QIcon(":/icons/24/dep_deprotec.png")); break;
	case DcController::LT: parent->setIcon(QIcon(":/icons/24/dep_lt.png")); break;
	case DcController::GSM: parent->setIcon(QIcon(":/icons/24/dep_p-gsm.png")); break;
	case DcController::P_SC: parent->setIcon(QIcon(":/icons/24/dep_p-sc.png")); break;
	case DcController::P_SCI: parent->setIcon(QIcon(":/icons/24/dep_p-sci.png")); break;
	case DcController::DepRtu: parent->setIcon(QIcon(":/icons/24/dep_rtu.png")); break;
	case DcController::ExRza: parent->setIcon(QIcon(":/icons/24/dep_exrza.png")); break;
	case DcController::ExSW: parent->setIcon(QIcon(":/icons/24/dep_ex-sw.png")); break;
    case DcController::RTU3_M_P: parent->setIcon(QIcon(":/icons/24/dep_rtu3-m.png")); break;
    case DcController::T2: parent->setIcon(QIcon(":/icons/24/dep_t-module.png")); break;
	case DcController::T2_DOUT_8R: parent->setIcon(QIcon(":/icons/24/dep_t-module.png")); break;
	case DcController::T2_EM_3M: parent->setIcon(QIcon(":/icons/24/dep_t-module.png")); break;
	case DcController::RPR_485_T3: parent->setIcon(QIcon(":/icons/24/dep_t-module.png")); break;
	default: break;
	}

	gJournal.addSource(JSource::make(controller));

	//parent->setIcon(DcMenu::root().icon());
	if (DcMenu::root().isAvailableFor(controller))
		pitemparent->updateProperty(form_index, DcMenu::root().formIdx());

	for (auto &it : DcMenu::root().childs())
		addSubmenuNode(it.second, parent, controller);
}

int32_t DcProjModel::getControllerID() {				// получаем индекс текущего выбранного контроллера
	QModelIndex index = m_tree->currentIndex();
	if (!index.isValid())
		return -1;
	QStandardItem* pitem = model->itemFromIndex(index);
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return -1;

	DcConfItem* pmodelitem = search->second;
	return pmodelitem->id();
}
