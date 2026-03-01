#include "DcCustomAlgorithmsForm.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qdir.h>

#include <journal/JSource.h>
#include <db/dc_db_manager.h>

#include "dpc/gui/widgets/TableView.h"
#include "dpc/gui/dialogs/msg_box/MsgBox.h"
#include "gui/forms/algorithms/custom/flex_editor_form.h"
#include "gui/forms/algorithms/custom/cfc_tools/cfc_compiler.h"


using namespace Dpc::Gui;

namespace {
	const QString PROPERTY_DESCRIPTION = "DESC";
	const QString PROPERTY_COMPILED = "COMPILED";

	enum Columns {
		NumberColumn = 0,
		CompileColumn,
		EnableColumn,
		NameColumn,
		DescColumn,

		ColumnsCount
	};

	QString columnName(Columns col) {
		switch (col)
		{
		case Columns::NumberColumn: return "№";
		case Columns::CompileColumn: return "Скомпилирован";
		case Columns::EnableColumn: return "Включен";
		case Columns::NameColumn: return "Название";
		case Columns::DescColumn: return "Описание";
		}

		return QString();
	}

	struct ModelItem {
		DcAlgCfc *alg;
		bool isCompiled;
		bool isEnabled;
	};
	using ModelItemList = std::vector<ModelItem>;

	class AlgsModel : public QAbstractTableModel
	{
	public:	
		AlgsModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}

		using EnabledFuncType = std::function<void(DcAlgCfc*, bool)>;
		void setEnabledFunc(EnabledFuncType func) { m_enabledFunc = func; }

		int rowCount(const QModelIndex &parent = QModelIndex()) const {
			if (parent.isValid())
				return 0;

			return m_items.size();
		}

		int columnCount(const QModelIndex &parent = QModelIndex()) const {
			if (parent.isValid())
				return 0;

			return ColumnsCount;
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
			if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
				return ::columnName((Columns)section);

			return QVariant();
		}

		Qt::ItemFlags flags(const QModelIndex &index) const {
			Qt::ItemFlags fl = QAbstractTableModel::flags(index);
			if (index.column() < EnableColumn)
				return fl;

			if (index.column() == EnableColumn)
				return m_items[index.row()].isCompiled ? fl |= Qt::ItemIsUserCheckable : fl;

			return fl | Qt::ItemIsEditable;
		}

		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
			if (!index.isValid())
				return QVariant();		

			auto &item = m_items[index.row()];
			if (Qt::DisplayRole == role) {
				switch (index.column())
				{
				case NumberColumn: return item.alg->index();
				case NameColumn: return item.alg->name();
				case DescColumn: return item.alg->property(PROPERTY_DESCRIPTION);
				}
			}

			if (Qt::EditRole == role) {
				switch (index.column())
				{
				case NameColumn: return item.alg->name();
				case DescColumn: return item.alg->property(PROPERTY_DESCRIPTION);
				}
			}

			if (Qt::TextAlignmentRole == role)
				return Qt::AlignCenter;

			if (Qt::DecorationRole == role) {
				switch (index.column())
				{
				case CompileColumn: return item.isCompiled ? QIcon(":/icons/32/compil_ok.png") : QIcon(":/icons/32/compil_fail.png");
				}
			}

			if (Qt::CheckStateRole == role) {
				switch (index.column())
				{
				case EnableColumn: return item.isEnabled ? Qt::Checked : Qt::Unchecked;
				}
			}

			return QVariant();
		}

		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) {
			auto &item = m_items[index.row()];
			int col = index.column();
			if (col == Columns::EnableColumn) {				
				if (m_enabledFunc) {
					m_enabledFunc(item.alg, value.toBool());
					return true;
				}
			}

			if (col == Columns::NameColumn) {
				item.alg->updateName(value.toString());
				emit dataChanged(index, index);
				return true;
			}

			if (col == Columns::DescColumn) {
				item.alg->updateProperty(PROPERTY_DESCRIPTION, value.toString());
				emit dataChanged(index, index);
				return true;
			}			

			return false;
		}

		void setItems(const ModelItemList &list) {
			beginResetModel();
			m_items = list;
			endResetModel();
		}

	private:
		ModelItemList m_items;
		EnabledFuncType m_enabledFunc;
	};

	DcAlgCfc *getAlgCfcByIO(DcController *pcontr, int32_t srcid) 
	{
		for (uint32_t i = 0; i < pcontr->algs_cfc()->size(); i++) {
			DcAlgCfc *palg = pcontr->algs_cfc()->get(i);
			for (uint32_t j = 0; j < palg->ios()->size(); j++) {
				DcAlgIOCfc *pcfcio = palg->ios()->get(j);
				if (pcfcio->index() == srcid)
					return palg;
			}
		}

		return nullptr;
	}
}

DcCustomAlgorithmsForm::DcCustomAlgorithmsForm(DcController *controller, const QString &path) :
	DcForm(controller, path, "Пользовательские алгоритмы", false),
	m_addButton(new QPushButton(QIcon(":/icons/32/compil_add.png"), "Добавить")),
	m_removeButton(new QPushButton(QIcon(":/icons/32/compil_del.png"), "Удалить")),
	m_compileButton(new QPushButton(QIcon(":/icons/32/compil.png"), "Компилировать")),
	m_fm(controller)
{
	auto model = new AlgsModel(this);
	model->setEnabledFunc([=](DcAlgCfc *alg, bool enabled) { setAlgorithmEnabled(alg, enabled); update(); });


	m_algsTableView = new TableView(model, this);
	m_algsTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_algsTableView->setEditTriggers(QAbstractItemView::SelectedClicked);
	m_algsTableView->horizontalHeader()->setStretchLastSection(true);
	m_algsTableView->horizontalHeader()->setHighlightSections(false);
	m_algsTableView->horizontalHeader()->resizeSection(NumberColumn, 20);
	m_algsTableView->horizontalHeader()->resizeSection(CompileColumn, 120);
	m_algsTableView->horizontalHeader()->resizeSection(EnableColumn, 100);
	m_algsTableView->horizontalHeader()->resizeSection(NameColumn, 150);

	update();

	connect(m_algsTableView, &QTableView::doubleClicked, this, &DcCustomAlgorithmsForm::onViewDoubleClicked);
	connect(m_addButton, &QPushButton::clicked, this, &DcCustomAlgorithmsForm::onAddButton);
	connect(m_removeButton, &QPushButton::clicked, this, &DcCustomAlgorithmsForm::onRemoveButton);
	connect(m_compileButton, &QPushButton::clicked, this, &DcCustomAlgorithmsForm::onCompileButton);

	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(m_addButton);
	buttonsLayout->addWidget(m_removeButton);
	buttonsLayout->addWidget(m_compileButton);
	buttonsLayout->addStretch();

	QGridLayout *layout = new QGridLayout(centralWidget());
	layout->addLayout(buttonsLayout, 0, 0);
	layout->addWidget(m_algsTableView, 1, 0);
}

DcCustomAlgorithmsForm::~DcCustomAlgorithmsForm()
{
}

bool DcCustomAlgorithmsForm::isAvailableFor(DcController * controller)
{
	static QList<Param> params = {
		{SP_FILE_FLEXLGFILES}
	};

	return hasAny(controller, params);
}

void DcCustomAlgorithmsForm::fillReport(DcIConfigReport * report)
{
	auto device = report->device();

	QStringList headers;
	for (size_t i = 0; i < ColumnsCount; i++)
		headers << columnName((Columns)i);

	QList<DcAlgCfc *> existingAlgs;
	DcReportTable table(device, headers, { 10, 10, 10, 30 });
	auto mainflexLgcFilesParam = dynamic_cast<DcParamCfg_v2*>(device->getParam(SP_FILE_FLEXLGFILES, 0));
	for (size_t i = 0; i < mainflexLgcFilesParam->getSubProfileCount(); i++) {
		DcAlgCfc *alg = device->algs_cfc()->getById(i + 1);
		if (!alg)
			continue;

		existingAlgs << alg;
		auto num = QString::number(alg->index());
		bool isCompiled = alg->property(PROPERTY_COMPILED).toInt() && QFileInfo(DcFlexLogicFileManager(device).localBcaFileName(alg)).exists();
		bool isEnabled = !device->getParam(SP_FILE_FLEXLGFILES, i)->value().isEmpty();
		auto name = alg->name();
		auto desc = alg->property(PROPERTY_DESCRIPTION);
		table.addRow({ num, DcReportTable::checkedSign(isCompiled), DcReportTable::checkedSign(isEnabled), name, desc });
	}

	if (!table.values().size())
		return;

	report->insertSection();
	report->insertTable(table);

	for (auto alg : existingAlgs) {

		//auto algImage = FbdEditWnd::algImage(device, alg);
		//if (algImage.isNull()) {
		//	MsgBox::error(QString("Не удалось создать изображение алгоритма %1").arg(alg->name()));
		//	continue; }
		//report->insertImage(algImage, alg->name());
	}
}

void DcCustomAlgorithmsForm::update()
{	
	ModelItemList itemList;
	auto mainflexLgcFilesParam = dynamic_cast<DcParamCfg_v2*>(controller()->params_cfg()->get(SP_FILE_FLEXLGFILES, 0));
	for (size_t i = 0; i < mainflexLgcFilesParam->getSubProfileCount(); i++) {
		DcAlgCfc *alg = this->alg(i + 1);

		if (!alg)
			continue;

		bool isCompiled = alg->property(PROPERTY_COMPILED).toInt() && QFileInfo(m_fm.localBcaFileName(alg)).exists();
		bool isEnabled = !controller()->params_cfg()->get(SP_FILE_FLEXLGFILES, i)->value().isEmpty();
		itemList.push_back({alg, isCompiled, isEnabled}); }

	static_cast<AlgsModel*>(m_algsTableView->model())->setItems(itemList);

	bool hasAlgs = itemList.size();
	m_removeButton->setEnabled(hasAlgs);
	m_compileButton->setEnabled(hasAlgs);
	m_addButton->setEnabled(itemList.size() < mainflexLgcFilesParam->getSubProfileCount());
}

void DcCustomAlgorithmsForm::onAddButton()
{
	auto algsList = controller()->algs_cfc();
	auto mainflexLgcFilesParam = dynamic_cast<DcParamCfg_v2*>(controller()->params_cfg()->get(SP_FILE_FLEXLGFILES, 0));
	int algNum = -1;
	for (size_t i = 0; i < mainflexLgcFilesParam->getSubProfileCount(); i++) {
		int num = i + 1;
		auto alg = algsList->getById(num);
		if (!alg) {
			algNum = num;
			break; }
	}

	if (algNum < 0)	return;
	if (!createFile(algNum)) return;

	auto alg = new DcAlgCfc(controller()->index(), algNum, algNum - 1, QString("Гибкая логика %1").arg(algNum), QString());
	algsList->add(alg, false);
	update();
	editAlgorithm(alg);
}

void DcCustomAlgorithmsForm::onRemoveButton()
{
	auto selectedIndexList = m_algsTableView->selectionModel()->selectedRows(NumberColumn);
	if (selectedIndexList.isEmpty())
		return;

	if (!MsgBox::question(tr("Вы уверены, что хотите удалить выбранные алгоритмы?"))) 
		return;

	for (auto &index : selectedIndexList) {
		removeAlgorithm(alg(index.data().toInt()));
	}

	update();
}

void DcCustomAlgorithmsForm::onCompileButton()
{
	for (auto &index : m_algsTableView->selectionModel()->selectedRows(NumberColumn))
		compileAlgorithm(alg(index.data().toInt()));

	update();
}

void DcCustomAlgorithmsForm::onViewDoubleClicked(const QModelIndex &index)
{
	auto numberIndex = m_algsTableView->selectionModel()->selectedRows(NumberColumn).value(0);
	editAlgorithm(alg(numberIndex.data().toInt()));
}

void DcCustomAlgorithmsForm::onEditorAboutToClose()
{
	auto editor = dynamic_cast<flexEditorForm*>(sender());
	if (!editor) return;

	auto alg = editor->alg();
	if (editor->hasError()) setCompiled(alg, false);
	else if (compileAlgorithm(alg))	setAlgorithmEnabled(alg, true);

	editor->deleteLater();
	update();
}

DcAlgCfc * DcCustomAlgorithmsForm::alg(int num) const
{
	return controller()->algs_cfc()->getById(num);
}

void DcCustomAlgorithmsForm::editAlgorithm(DcAlgCfc* alg)
{
	auto editor = new flexEditorForm(controller(), alg, this);
	if (!editor->init()) {
		MsgBox::error(editor->errorString());
		delete editor;
		return;	}
		
	connect(editor, &flexEditorForm::aboutToClose, this, &DcCustomAlgorithmsForm::onEditorAboutToClose, Qt::QueuedConnection);
	editor->setWindowModality(Qt::ApplicationModal);
	editor->show();	
}

bool DcCustomAlgorithmsForm::createFile(uint32_t algNum)
{
	auto localPath = m_fm.localPath();
	QDir dir(localPath);
	if (!dir.mkpath(".")) {
		MsgBox::error(QString("Не удалось создать папку для файлов гибкой логики: %1").arg(localPath));
		return false;
	}

	auto fileName = m_fm.localGraphFileName(algNum);
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		MsgBox::error(QString("Не удалось создать файл схемы гибкой логики: %1").arg(fileName));
		return false;
	}

	file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<MODEL/>");
	return true;
}

void DcCustomAlgorithmsForm::removeAlgorithm(DcAlgCfc *alg)
{
	DcPoolSingleKey<DcAlgIOCfc*>* ios = alg->ios();	// Получаем входы/выходы, используемые в алгоритме?
	for (uint32_t i = 0; i < ios->size(); i++) {
		DcAlgIOCfc *pio = ios->get(i);
		if (pio == nullptr)
			continue;
		if (pio->direction() == IO_DIRECTION_INPUT) {
			DcMatrixElementAlgCfc* pmatrix = controller()->matrix_cfc()->get(pio->index());
			if (pmatrix != nullptr)
				controller()->matrix_cfc()->remove(pmatrix->src(), pmatrix->dst());
		}
		else if (pio->direction() == IO_DIRECTION_OUTPUT) {
			DcMatrixElementAlgCfc* pmatrix = controller()->matrix_cfc()->get(pio->index());
			if (pmatrix == nullptr)
				continue;

			int32_t virIndex = pmatrix->dst();
			DcMatrixElementSignal* pmatrixsignal = controller()->matrix_signals()->get(virIndex);	// получаем привзку виртуального сигнала к физическому
			if (pmatrixsignal != nullptr)
				controller()->matrix_signals()->remove(pmatrixsignal->src(), pmatrixsignal->dst());

			DcSignal *old = controller()->getSignal(virIndex);	// Освобождаемые сигналы становятся с именами по умолчанию
			if (old) {
				old->updateName(VIRTUAL_DEFAULT_NAME);
			}

			// Проверяем связи с внутренними алгоритмами
			DcMatrixElementAlg* matrixelem = controller()->matrix_alg()->getDst(virIndex);	// Проверяем, если такой сигнал в таблице привязки внутренних алгоритмов
			while (matrixelem) {	// Удаляем все сигналы, которые были подписаны на удаляемых выход
				controller()->matrix_alg()->removeByDest(virIndex);	// Удаляем старый
				matrixelem = controller()->matrix_alg()->getDst(virIndex);	// Ищем, есть ли еще?
			}

			controller()->matrix_cfc()->remove(pmatrix->src(), pmatrix->dst());
		}
	}
	
	setAlgorithmEnabled(alg, false);
	alg->ios()->removeAll();	//Удаляем входы-выходы из алгоритма
	controller()->algs_cfc()->removeById(alg->index());	// Удаляем сам алгоритм из списка
	
	QFile::remove(m_fm.localGraphFileName(alg));
	QFile::remove(m_fm.localBcaFileName(alg));
	QFile::remove(m_fm.localImageFileName(alg));
}

bool DcCustomAlgorithmsForm::compileAlgorithm(DcAlgCfc* alg)
{
	bool ok = true;
	CfcCompiler* compiler = new CfcCompiler();
	connect(compiler, &CfcCompiler::errorToLog, this, [=](const QString& mess) { gJournal.addErrorMessage(mess, JSource::make(controller())); });
	connect(compiler, &CfcCompiler::infoToLog, this, [=](const QString& mess) { gJournal.addInfoMessage(mess, JSource::make(controller())); });
	ok = compiler->compile(m_fm.localGraphFileName(alg));
	delete compiler;

	setCompiled(alg, ok);
	if (!ok)
		MsgBox::error(QString("При компиляции алгоритма номер %1, возникли критические ошибки").arg(alg->index()));


	return ok;
}

void DcCustomAlgorithmsForm::setAlgorithmEnabled(DcAlgCfc* alg, bool enabled)
{
	gDbManager.beginTransaction(controller()->index());

	QString value = enabled ? m_fm.deviceBcaFileName(alg) : "";
	controller()->params_cfg()->get(SP_FILE_FLEXLGFILES, alg->position())->updateValue(value);

	// сбрасываем все значения таблицы привязки в невалидное значение(0xffff)
	auto param = dynamic_cast<DcParamCfg_v2*>(controller()->params_cfg()->get(SP_FLEXLGCROSSTABLE, alg->position()));
	for (size_t i = 0; i < param->getProfileCount(); i++) {
		if (i != alg->position())
			continue;

		for (size_t j = 0; j < param->getSubProfileCount(); j++)
			controller()->params_cfg()->get(SP_FLEXLGCROSSTABLE, i * PROFILE_SIZE + j)->updateValue(QString::number(USHRT_MAX));
	}

	// настравиваем в таблице актуальные привязки алгоритма
	if (enabled) {
		for (uint32_t i = 0; i < controller()->matrix_cfc()->size(); i++) {
			DcMatrixElementAlgCfc *bind = controller()->matrix_cfc()->get(i);
			DcAlgCfc *tempAlg = getAlgCfcByIO(controller(), bind->src());
			if (tempAlg != alg)
				continue;

			DcAlgIOCfc *pios = tempAlg->ios()->getById(bind->src());
			int32_t pos = tempAlg->position() * PROFILE_SIZE + pios->pin();
			auto signal = controller()->getSignal(bind->dst());
			controller()->params_cfg()->get(SP_FLEXLGCROSSTABLE, pos)->updateValue(QString::number(signal->internalId()));
		}
	}

	gDbManager.endTransaction(controller()->index());
}

void DcCustomAlgorithmsForm::setCompiled(DcAlgCfc * alg, bool compiled)
{
	alg->updateProperty(PROPERTY_COMPILED, QString::number(compiled));
	if (compiled)
		return;

	QFile::remove(m_fm.localBcaFileName(alg));	
	setAlgorithmEnabled(alg, false);
}
