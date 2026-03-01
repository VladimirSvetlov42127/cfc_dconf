#include "frmCreateProj.h"

#include <QFile>
#include <QtCore/QTextStream>
#include "utils/cfg_path.h"
#include "data_model/dc_data_manager.h"
#include <dpc/sybus/channel/Channel.h>
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

using namespace Dpc::Gui;

frmCreateProj::frmCreateProj(DConf *parent) : QDialog(parent)
{
	pparent = nullptr;
	if (parent == nullptr) return;
	pparent = parent;
	ui.setupUi(this);

	//=====================================================================================================================================
	//	Изменено 10.05.2023	//	Ввод русских букв в имени проекта
	//=====================================================================================================================================
	QRegularExpression regex("^[-a-zA-Z0-9а-яА-Я_\\s]*$");
	_validator = new QRegularExpressionValidator(regex, this);
	ui.lineName->setValidator(_validator);
	return;
}

frmCreateProj::~frmCreateProj()
{
	delete _validator;
	return;
}

bool frmCreateProj::createProject(QString &namestr, QString &objectstr, QString &authorstr, QString &descstr) 
{
	cfg::path::setCurProject(cfg::path::createProj());
	if (cfg::path::curProject().isEmpty())
		false;
	if (!createProjectFile(namestr, objectstr, authorstr, descstr) || 
		!createTopologyFile(namestr, descstr)) {
		cfg::path::setCurProject("");
		return false;
	}
	gDataManager.projectDesc()->setVersion(1);
	gDataManager.projectDesc()->save(cfg::path::getProjectInfo());
	return true;
}

void frmCreateProj::projCreate() {
	QString namestr = ui.lineName->text();
	QString objectstr = ui.lineObject->text();
	QString authorstr = ui.lineAuthor->text();
	QString descstr = ui.textDescription->toPlainText();

	if (namestr.isEmpty()) {
		return;
	}

	if (!createProject(namestr, objectstr, authorstr, descstr)) {
		MsgBox::warning("cancel Project create action");
	}
	else
		pparent->initProject();
	this->close();
}

bool frmCreateProj::createProjectFile(QString &namestr, QString &objectstr, QString &authorstr, QString &descstr) {
	QFile infofile(cfg::path::getProjectInfo());
	if (infofile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream outStream(&infofile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		outStream.setCodec("UTF-8");
#else
		outStream.setEncoding(QStringConverter::Utf8);
#endif
		QString restmp = "{\"name\":\"%1\",\"object\":\"%2\",\"author\":\"%3\",\"desc\":\"%4\",\"create_time\":\"%5\",\"update_time\":\"%6\"}";
		auto currentDateTime = QDateTime::currentDateTime().toString("(yyyy.MM.dd) hh:mm:ss");
		QString res = restmp.arg(namestr, objectstr, authorstr, descstr, currentDateTime, currentDateTime);
		outStream << res;
		infofile.close();
		// add description param to model
		gDataManager.projectDesc()->clear();
		gDataManager.projectDesc()->init(cfg::path::getProjectInfo());
		return true;
	}
	else
		return false;
}

bool frmCreateProj::createTopologyFile(QString &namestr, QString &descstr)
{
	QFile topologyfile(cfg::path::getTopology());
	if (topologyfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream outStream(&topologyfile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		outStream.setCodec("UTF-8");
#else
		outStream.setEncoding(QStringConverter::Utf8);
#endif
		QString restmp = "{\"tree\":[{\"id\":1, \"pid\":0, \"type\":\"%1\", \"name\":\"%2\", \"desc\":\"%3\"}]}";
		QString res = restmp.arg(QString("project"), namestr, descstr);
		outStream << res;
		topologyfile.close();
		gDataManager.topology()->clear();
		return gDataManager.topology()->fromJson();
	}
	return false;
}

void frmCreateProj::projCancel() {
	this->close();
}
