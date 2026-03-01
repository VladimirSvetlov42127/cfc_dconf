#pragma once

#include <qdialog.h>
#include "ui_IBindDialog.h"

#include <gui/forms/table_of_bindings/dialogs/models/IDlgModel.h>

class IBindDialog : public QDialog
{
	Q_OBJECT		

public:
	IBindDialog(DcController *controller, QWidget *parent = Q_NULLPTR);
	virtual ~IBindDialog();

	bool updateValue(int32_t storage, int32_t processing);
	void setModel(IDlgModel* model) { m_model = model; };
	
protected:
	Ui::IBindDialog ui;
	IDlgModel* m_model = nullptr;
	DcController* m_controller;

protected:
	virtual void selectSignal() = 0;
	virtual void newSignalChoosed(DcSignal* signal) = 0;

public slots:
	void slot_cancel();
	void slot_select();
	void slot_dbl_clicked(QModelIndex elem);

	void slot_newSignalChoosed(DcSignal* signal);

};
