#pragma once

#include <file_managers/IFileManager.h>

class DcFlexLogicFileManager :public IFileManager
{
public:
	DcFlexLogicFileManager(DcController *device = nullptr);

	QString localGraphFileName(DcAlgCfc *alg) const;
	QString localGraphFileName(int algNum) const;
	QString localImageFileName(DcAlgCfc *alg) const;
	QString localBcaFileName(DcAlgCfc *alg) const;
	QString deviceBcaFileName(DcAlgCfc *alg) const;

	virtual QStringList configFiles() const override;
};

FILE_MANAGER_REGISTER(DcFlexLogicFileManager)