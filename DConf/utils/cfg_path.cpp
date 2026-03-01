#include "cfg_path.h"

#include <qdir.h>
#include <qfiledialog.h>
#include <qcoreapplication.h>
#include <qstandardpaths.h>

#include <dpc/gui/dialogs/msg_box/MsgBox.h>


#include <data_model/dc_data_manager.h>
//#include <dc_db_connection.h>

using namespace Dpc::Gui;

namespace {
    const QString proj_prefix = "proj_";
}

QString cfg::path::_templatesPath = "../templates/";
QString cfg::path::_curProjectPath = QString();
QString cfg::path::_projectsPath = QString("%1/DEP/DConf/proj/").arg(QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)
//До main() <Company> и <AppName> пустые
#ifdef WIN32
	//WIN: C:/ProgramData/<Company>/<AppName>
	//LINUX: /etc/xdg/<Company>/<AppName>
	//MAC: /Users/<UserName>/Library/Application Support/<AppName>
 	.at(1));
#else
	//WIN: C:/Users/User/AppData/Local/<Company>/<AppName>
	//LINUX: /home/user/.config/<Company>/<AppName>
	//MAC: /Users/<UserName>/Library/Preferences/com.<Company>.<AppName>.plist
	.at(0));
#endif 


QString cfg::path::templates()
{
	return _templatesPath;
}

QString cfg::path::getTemplateConfigurations()
{
	return cfg::path::templates() + getTemplateConfigurationsFile();
}

QString cfg::path::getTemplateConfigurationsFile()
{
	return "configurations.lst";
}

QString cfg::path::getCompatibleDevices()
{
	return cfg::path::templates() + getCompatibleDevicesFile();
}

QString cfg::path::getCompatibleDevicesFile()
{
	return "compatible_devices.txt";
}

QString cfg::path::controllerDir(qint32 controllerid)
{
	if (curProject().isEmpty())
		return QString();
	return curProject() + "/u" + QString::number(controllerid);
}

QString cfg::path::controllerFile(qint32 controllerid)
{
	if (curProject().isEmpty())
		return QString();
    return controllerDir(controllerid) + controllerFile();;
}

QString cfg::path::controllerFile()
{
	return "/uconf.db";
}

QString cfg::path::projects()
{
	return _projectsPath;
}

QString cfg::path::curProject()
{
	return _curProjectPath;
}

bool cfg::path::setCurProject(const QString& projath)
{
	if (_curProjectPath == projath)
		return true;
	QDir file(projath);
	if (!file.exists()) {
		return false;
	}
	_curProjectPath = projath;
	return true;
}

int cfg::path::getNewProjNumber() {
	QDir file(projects());
	if (!file.exists()) {
		return 1;
	}
	int num = 0;
	foreach(QFileInfo fileInfo, file.entryInfoList())
	{
		if (!isProjectDir(fileInfo))
			continue;

		int number = fileInfo.fileName().mid(5).toInt();
		if (num < number)
			num = number;
	}
	num++;
	return num;
}

bool cfg::path::isProjectDir(const QFileInfo& fileInfo)
{
	if (fileInfo.fileName().size() < 6)
		return false;
	if (!fileInfo.fileName().contains(proj_prefix))
		return false;
	return true;
}

QString cfg::path::getTopology()
{
	if (curProject().isEmpty())
		return QString();
	return QDir(curProject()).filePath(getTopologyFile());
}

QString cfg::path::getTopologyFile()
{
	return "topology.cfg";
}

QString cfg::path::getProjectInfo()
{
	if (curProject().isEmpty())
		return QString();
	return QDir(curProject()).filePath(getProjectInfoFile());
}

QString cfg::path::getProjectInfoFile()
{
	return "info.cfg";
}

QString cfg::path::createProj() 
{
	return createProj(getNewProjNumber());
}

QString cfg::path::createProj(int projnum) {
	QString folder = QString("%1%2%3")
		.arg(projects())
		.arg(proj_prefix)
		.arg(projnum, 3, 10, QLatin1Char('0'));

	if (QDir().mkpath(folder)) {	// Создаем cо вложенными папками, поэтому не mkdir()
		return folder;
	}
	return QString();
}

bool cfg::path::remove(const QString & dirName)
{
	bool result = true;
	QDir dir(dirName);

	if (dir.exists()) {
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = remove(info.absoluteFilePath());
			}
			else {
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) {
				return result;
			}
		}
		result = QDir().rmdir(dirName);
	}
	return result;
}

bool cfg::path::removeController(qint32 controllerid) {
	
	QString contrpath = controllerDir(controllerid);
	QDir controllerdir(contrpath);
	if (controllerdir.exists())
		return remove(contrpath);
	return false;
}

bool cfg::path::copy(const QString& src, const QString& dst) {
	QDir dir(src);
	if (!dir.exists())
		return false;

	const QString src_dir = QDir::toNativeSeparators(dir.absolutePath()) + QDir::separator();
	const QString dst_dir = QDir::toNativeSeparators(QDir(dst).absolutePath()) + QDir::separator();

	QDir projpath(QDir::currentPath());
    for(auto& d: dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		QString dst_path = dst_dir + d;
		projpath.mkpath(dst_path);
		if (!copy(src_dir + d, dst_path))
			return false;
	}

    foreach(auto &f, dir.entryList(QDir::Files)) {
		QString dst_path = dst_dir + f;
		if (QFile().exists(dst_path))
			QFile().remove(dst_path);

		if (!QFile::copy(src_dir + f, dst_path))
			return false;
	}
	return true;
}

QString cfg::path::templateControllerDir(qint32 controllerid) {
	DcController *pcontroller = gDataManager.controllers()->getById(controllerid);
	if (pcontroller == nullptr) {
		MsgBox::error(QString("Не удалось найти файл меню контроллера. Не найден контроллер c указанным индексом №" + QString::number(controllerid)));
		return QString();
	}
	DcSetting *model = pcontroller->settings()->get("model");
    if (model == nullptr) {
		MsgBox::error(QString("Ошибка конфигурации устройства. Не удалось установить модель устройства №" + QString::number(controllerid)));
		return QString();
	}

	// пробуем получить меню из директории модификации
	QString dir = templates() + model->value();

	DcSetting* modifier = pcontroller->settings()->get("prodkey");
	if (modifier)
		dir += QString("/%1").arg(modifier->value());

	QDir dircom(dir);
	if (dircom.exists()) {
		return dir;
	}

	return QString();
}


QString cfg::path::templateControllerFile(qint32 controllerid)
{
	QString dir = templateControllerDir(controllerid);
	if (dir.isEmpty())
		return QString();
	return dir + templateControllerFile();

}

QString cfg::path::templateControllerFile()
{
	return "/template.db";
}
