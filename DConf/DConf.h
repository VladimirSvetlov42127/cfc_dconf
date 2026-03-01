#pragma once

#include "ui_DConf.h"
#include <QtWidgets/QMainWindow>

#include "dc_proj_model.h"

class DcUpdater;

class DConf : public QMainWindow
{
	Q_OBJECT

public:
	DConf(QWidget *parent = Q_NULLPTR);
	void initProject();	

private:
	Ui::dconfClass ui;
	DcProjModel *pmodel;
	QString m_importDir;
	DcUpdater* m_updater;
	bool m_updaterSilent;

public slots:
	void projCreateSlot();
	void projOpenSlot();
	void appAboutSlot();
	void projImportSlot();
	void projExportSlot();
	void appExitSlot();
	void openHelp(const QString& context = QString());
	void checkForUpdates(bool silent = true);

signals:

private slots:
	void onUpdaterError(const QString& errorMsg);
	void onUpdaterState(bool hasUpdates);
    void onErrorDb(const QString &msg, const QString &filePath, DcController *device);

private:
	void initProjectTree();
	void projectExit();
	void saveSettings() const;
	void loadSettings();
	void setStatusBarMsg(const QString& text, const QIcon& icon);

protected:
	virtual void closeEvent(QCloseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
};
