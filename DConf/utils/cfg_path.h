#pragma once

#include <qstring.h>
#include <qfileinfo.h>

namespace cfg
{
    struct path
	{
		//Project paths
        static QString projects();
		static QString curProject();
		static int getNewProjNumber();
		static QString getTopology();
		static QString getTopologyFile();
		static QString getProjectInfo();
		static QString getProjectInfoFile();

		static bool setCurProject(const QString& projath);

		static QString createProj();
		static QString createProj(int projnum);
		static bool isProjectDir(const QFileInfo& info);
		//Controller paths
		static QString templates();
		static QString getTemplateConfigurations();
		static QString getTemplateConfigurationsFile();
		static QString getCompatibleDevices();
		static QString getCompatibleDevicesFile();

		static QString controllerDir(qint32 controllerid);
		static QString controllerFile(qint32 controllerid);
		static QString controllerFile();

		static QString templateControllerDir(qint32 controllerid);
		static QString templateControllerFile(qint32 controllerid);
		static QString templateControllerFile();

		static bool removeController(qint32 controllerid);

		//File systrm operations
		static bool remove(const QString & dirName);
		static bool copy(const QString& src, const QString& dst);
	private:
		static QString  _templatesPath;
		static QString  _projectsPath;
		static QString  _curProjectPath;
	};
}
