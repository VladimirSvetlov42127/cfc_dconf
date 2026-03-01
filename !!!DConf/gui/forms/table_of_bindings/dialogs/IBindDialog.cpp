#include "IBindDialog.h"

IBindDialog::IBindDialog(DcController *controller, QWidget *parent)
	: QDialog(parent),
	m_controller(controller)
{
	ui.setupUi(this);
	setWindowIcon(QIcon(":/images/16/bind.png"));

	QObject::connect(ui.projTree, &QTreeView::doubleClicked, this, &IBindDialog::slot_dbl_clicked);
	QObject::connect(ui.btnSelect, &QPushButton::released, this, &IBindDialog::slot_select);
	QObject::connect(ui.btnCancel, &QPushButton::released, this, &IBindDialog::slot_cancel);
}

IBindDialog::~IBindDialog()
{
}

bool IBindDialog::updateValue(int32_t storage, int32_t processing)
{
	if (storage < 1)
		return false;
	if (processing < 1)
		return false;
	return true;
}

void IBindDialog::slot_cancel() {
	this->done(QDialog::Rejected);
}

void IBindDialog::slot_select() {
	selectSignal();
}

void IBindDialog::slot_dbl_clicked(QModelIndex elem) {
	m_model->slot_itemSelectedChanged(elem, QModelIndex());	// Вызываем явно слот, чтобы подставить данные в новое имя 
	slot_select();
}

void IBindDialog::slot_newSignalChoosed(DcSignal* signal) {
	newSignalChoosed(signal);
}