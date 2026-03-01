#pragma once

#include <qdialog.h>

#include <device_operations/DcPassportOperation.h>

class QTreeWidget;

class DcPassportDialog : public QDialog
{
	Q_OBJECT

public:
	DcPassportDialog(DcController *controller, QWidget *parent = nullptr);

	DcPassportOperation::PassportStructure structure() const { return m_ps; }

public slots:
	void onOkButton();

private:
	QTreeWidget *m_treeWidget;
	DcPassportOperation::PassportStructure m_ps;
};