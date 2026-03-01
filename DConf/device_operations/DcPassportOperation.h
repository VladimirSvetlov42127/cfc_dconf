#pragma once

#include <device_operations/DcTemporaryConfigOperation.h>
#include <gui/forms/DcMenu.h>

class DcIConfigReport;

class DcPassportOperation : public DcTemporaryConfigOperation
{
	Q_OBJECT

public:
	struct PassportStructure 
	{
		DcMenu *section = nullptr;
		QList<PassportStructure> childs;
	};

	DcPassportOperation(DcController *device, const PassportStructure &structure, const QString &fileName, QObject *parent = nullptr);

protected:
	virtual bool after() override;

private:
	void fillReport(const PassportStructure &ps, DcIConfigReport *report);

private:
	PassportStructure m_ps;
	QString m_fileName;
	int m_currentSection;
};