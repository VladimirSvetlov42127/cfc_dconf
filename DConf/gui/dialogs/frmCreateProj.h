#pragma once

#include <QDialog>
#include "ui_frmCreateProj.h"
#include "DConf.h"

class frmCreateProj : public QDialog
{
	Q_OBJECT

public:
	frmCreateProj(DConf *parent);
	~frmCreateProj();

private:
	Ui::frmCreateProj ui;
	DConf* pparent;
	QValidator* _validator;

public slots:
	void projCreate();
	void projCancel();

private:
	bool createProject(QString &namestr, QString &objectstr, QString &authorstr, QString &descstr);
	bool createProjectFile(QString &namestr, QString &objectstr, QString &authorstr, QString &descstr);
	bool createTopologyFile(QString &namestr, QString &descstr);
	//bool loadProjectToModel(QString &proj_path);
};
