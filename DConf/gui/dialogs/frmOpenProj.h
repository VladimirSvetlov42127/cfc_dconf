#pragma once

#include <QDialog>
#include "ui_frmOpenProj.h"
#include "DConf.h"

class frmOpenProj : public QDialog
{
	Q_OBJECT

public:
	frmOpenProj(DConf *parent);

private:
	Ui::frmOpenProj ui;
	int nextrawnumber;
	DConf *pparent;

public:
	QStandardItemModel * projmodel;

private:
	void initTableView(void);
	void collectData(void);
	void raw(const QString &path, const QString &name, const QString &object, const QString &auth, const QString &desc, const QString &timeupdate);
	void getInfoFromFolder(const QString &foldername);

public slots:
	void projOpen();
	void projRemove();
	void projOpen(QModelIndex index);
};
