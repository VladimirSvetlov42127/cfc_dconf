#pragma once

#include <gui/forms/DcFormFactory.h>

class QToolBar;
class QLineEdit;
class QAction;

class DcMainForm : public DcForm
{
	Q_OBJECT

public:
	DcMainForm(DcController *controller, const QString &path);

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);

signals:
	void progress(int value);
	void stateChanged(bool busy, bool withProgress, bool deviceTemplateLocked);

public slots:
	virtual void activate() override;

private slots:
	void onConnectButton();
	void onDisconnectButton();
	void onProgress(int value);
	void onBeforeOperation(bool withProgress, bool deviceTemplateLocked);
	void onAfterOperation();
	void loadPurposeFile();
	void savePurposeFile();

private:
	void updateDeviceWidget();

private:
	QToolBar *m_toolBar;
	QAction *m_disconnectAction;
	QWidget *m_deviceWidget;

	QLineEdit* m_companyEdit;
	QLineEdit* m_objectEdit;
	QLineEdit* m_placeEdit;
	QLineEdit* m_connectionEdit;
	QLineEdit* m_cabinetEdit;
	QLineEdit* m_implementerEdit;
	QLineEdit* m_dateEdit;
	QLineEdit* m_periodEdit;
	QLineEdit* m_reasonEdit;
};

REGISTER_FORM(DcMainForm, DcMenu::root);