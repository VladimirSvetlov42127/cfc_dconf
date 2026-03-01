#include "DcPassportOperation.h"

#include <qtemporarydir.h>
#include <qdebug.h>
#include <qdesktopservices.h>
#include <qurl.h>

#include <data_model/storage_for_controller_params/DcController_v2.h>
#include <dpc/sybus/channel/Channel.h>
#include <dpc/sybus/utils.h>
#include <device_operations/DcReadConfigOperation.h>
#include <report/DcTextDocumentConfigReport.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		CreatePassportStep = 1,
		SaveFileStep,

		TotalStepsCount
	};

	int sectionsCount(const DcPassportOperation::PassportStructure &ps) {
		if (!ps.section)
			return 0;

		int count = 1;
		for (auto &it : ps.childs)
			count += sectionsCount(it);

		return count;
	}
}

DcPassportOperation::DcPassportOperation(DcController * device, const PassportStructure &structure, const QString &fileName, QObject * parent) :
	DcTemporaryConfigOperation("Создание паспорта конфигурации", device, TotalStepsCount, parent),
	m_ps(structure),
	m_fileName(fileName),
	m_currentSection(0)
{
}

bool DcPassportOperation::after()
{
	addInfo("Создание паспорта конфигурации...");
	setCurrentStep(CreatePassportStep, sectionsCount(m_ps));
	
	auto report = new DcTextDocumentConfigReport(tempDevice());
	m_currentSection = 0;
	fillReport(m_ps, report);

	setCurrentStep(SaveFileStep);
	if (!report->save(m_fileName)) {
		addError(QString("Не удалось сохранить паспорт конфигурации"));
		return false;
	}

	QDesktopServices::openUrl(QUrl(m_fileName));
	return true;
}

void DcPassportOperation::fillReport(const PassportStructure &ps, DcIConfigReport *report)
{
	auto title = ps.section->title();
	if (title.isEmpty())
		title = "Информация об устройстве";

	addInfo(QString("Обработка раздела %1").arg(title));
	emitProgress(++m_currentSection);

	if (!ps.section->isAvailableFor(report->device()))
		return;

	auto hasPrefix = report->addSectionPrefix(ps.section->title());

	ps.section->fillReport(report);
	for (auto &it : ps.childs)
		fillReport(it, report);

	if (hasPrefix)
		report->takeSecionPrefix();
}