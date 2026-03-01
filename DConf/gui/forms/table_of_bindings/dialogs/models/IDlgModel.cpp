#include "IDlgModel.h"

IDlgModel::IDlgModel(QObject *parent)
	: QObject(parent), m_tree(nullptr), m_parent(nullptr), m_controller(nullptr), m_signalManager(nullptr)
{
	m_model = new QStandardItemModel();
	m_selectionModel = new QItemSelectionModel(m_model);
}

IDlgModel::IDlgModel(DcController *controller, QTreeView * ptree, QObject * parent)
	: m_tree(ptree),
	m_parent(parent),
	m_controller(controller)
{
	m_model = new QStandardItemModel;
	m_tree->setModel(m_model);
	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

	m_selectionModel = new QItemSelectionModel(m_model);
	m_tree->setSelectionModel(m_selectionModel);

	m_signalManager = new DcSignalManager(m_controller, nullptr, this);

	bool ok = connect(m_selectionModel, &QItemSelectionModel::currentChanged, this, &IDlgModel::slot_itemSelectedChanged);
	Q_ASSERT(ok);
}

IDlgModel::~IDlgModel()
{
}

void IDlgModel::clear()
{
	m_model->clear();
}



DcSignal * IDlgModel::getItemData(QModelIndex &index)
{
	QStandardItem *pitem = m_model->itemFromIndex(index);
	if (pitem == nullptr)
		return nullptr;
	auto search = m_assocTable.find(pitem);
	if (search == m_assocTable.end())
		return nullptr;

	return search->second;
}


void IDlgModel::slot_itemSelectedChanged(const QModelIndex &current, const QModelIndex &previous) {

	if (!current.isValid())
		return;

	auto search = m_assocTable.find(m_model->itemFromIndex(current));	// Ищем в таблице сигналов
	if (search == m_assocTable.end())
		return;

	DcSignal* signal = search->second;
	emit signal_newSignalChoosed(signal);

}

