#pragma once

#include <qwidget.h>

#include <data_model/dc_controller.h>

class QToolBar;
class QPushButton;
class QProgressBar;
class QTabWidget;
class QLineEdit;

class DcIDeviceOperation;

class DeviceWidget : public QWidget
{
	Q_OBJECT

public:
	DeviceWidget(DcController *controller, QWidget *parent = nullptr);
	~DeviceWidget();

signals:
	void progress(int value);
	void beforeOperation(bool withProgress, bool deviceTemplateLocked);
	void afterOperation();

public slots:
	void updateInfo();

private slots:
	void onReloadButton();
	void onReadButton();
	void onWriteButton();
	void onRestartButton();
	void onFormatButton();
	void onResetButton();
	void onProgress(int value);
	void onPassport();
    void onFileSystem();
	void slotUpdateSystem();

	void execOperation(DcIDeviceOperation *operation, bool withProgress, bool lockDeviceTemplate);
	void onOperationFinished(DcIDeviceOperation *operation);

	void onUpdateInfoFinished(DcIDeviceOperation *operation);
	void updateIpAddress(const QString &newIpAddress);

private:
	DcController* device() const;
	ChannelPtr channel();
	void setBusy(bool busy, bool withProgress, bool lockDeviceTemplate);
	bool isBusy() const;

	using ToolBarMenuItemList = QList<QPair<QString, QVariant>>;

	template<class Functor>
	void addToolBarMenu(const QIcon &icon, const QString &text, Functor functor, const ToolBarMenuItemList &menuItems, const QString &toolTip);

private:
	DcController *m_device;
	QToolBar *m_toolBar;
	QAction *m_readAction;
	QAction *m_writeAction;
	QWidget *m_deviceInfoWidget;
	QProgressBar *m_progressBar;
	QTabWidget *m_tabWidget;
	QLineEdit *m_ipAddressEdit;
	bool m_isBusy;
};
