#include "DIOBoardWidget.h"

#include <qtabwidget.h>


#include <report/DcIConfigReport.h>
#include <data_model/dc_controller.h>

#include <gui/editors/EditorsManager.h>

namespace {
	const ListEditorContainer g_AmperageList = { "DC", "AC" };

	// temporary, for counting board params profile!!!!
	int getProfileCount(DcBoard *board, int32_t addr)
	{
		int count = 0;
		while (board->params()->get(addr, count))
			count++;

		return count;
	}
}

namespace Text {
	namespace Common {
		const QString Title = "Общие";
		const QString Amperage = "Оперативный ток";
	}

	const QString Drebezg = "Постоянная обработки дребезга";
	const QString Inversion = "Инверсия";
	const QString Timer = "Cторожевой таймер дребезга";
	const QString Interference = "Параметр отстройки от помех";
}

DIOBoardWidget::DIOBoardWidget(DcBoard *board, QWidget *parent) :
	QWidget(parent),
	m_board(board)
{
	QVBoxLayout *formLayout = new QVBoxLayout(this);
	QTabWidget *tabWidget = new QTabWidget;
	formLayout->addWidget(tabWidget);

	tabWidget->addTab(createCommonTab(), Text::Common::Title);
	tabWidget->addTab(createDrebezgTab(), Text::Drebezg);
	tabWidget->addTab(createInversionTab(), Text::Inversion);
	tabWidget->addTab(createTimerTab(), Text::Timer);
	tabWidget->addTab(createInterferenceTab(), Text::Interference);
}

void DIOBoardWidget::fillReport(DcBoard * board, DcIConfigReport * report)
{
	auto param = board->params()->get(SP_DIN_ALG, 0);
	if (param) {
		DcReportTable table(report->device());
		table.addRow(DcReportTable::ParamRecord(param, g_AmperageList.toHash(), Text::Common::Amperage));
		report->insertTable(table);
	}

	int paramProfileCount = getProfileCount(board, SP_DIN_INT_CNT);
	if (!paramProfileCount)
		return;

	QStringList headers = {
			Text::ReportTable::Name,
			Text::Drebezg,
			Text::Inversion,
			Text::Timer,
			Text::Interference
	};

	DcReportTable table(report->device(), headers, { 40 });
	for (size_t i = 0; i < paramProfileCount; i++) {
		QString name = QString("Di%1").arg(i + 1);
		QList<DcReportTable::ParamRecord> recs;
		recs.append(DcReportTable::ParamRecord(board->params()->get(SP_DIN_INT_CNT, i)));
		recs.append(DcReportTable::ParamRecord(board->params()->get(SP_DIN_INVERS, i), QString(), true));
		recs.append(DcReportTable::ParamRecord(board->params()->get(SP_DIN_INT_WDT, i)));
		recs.append(DcReportTable::ParamRecord(board->params()->get(SP_DIN_NOISE_WDT, i)));
		table.addRow(name, recs);
	}

	report->insertTable(table);
}

QWidget * DIOBoardWidget::createCommonTab() const
{
	auto param = m_board->params()->get(SP_DIN_ALG, 0);
	if (!param)
		return nullptr;

	QWidget *tab = new QWidget;
	QGridLayout *tabLayout = new QGridLayout(tab);
	EditorsManager manager(nullptr, tabLayout);

	manager.addListEditor(param, Text::Common::Amperage, g_AmperageList);

	manager.addStretch();
	return tab;
}

QWidget * DIOBoardWidget::createDrebezgTab() const
{
	return makeTab(SP_DIN_INT_CNT, 0);
}

QWidget * DIOBoardWidget::createInversionTab() const
{
	return makeTab(SP_DIN_INVERS, 1);
}

QWidget * DIOBoardWidget::createTimerTab() const
{
	return makeTab(SP_DIN_INT_WDT, 0);
}

QWidget * DIOBoardWidget::createInterferenceTab() const
{
	return makeTab(SP_DIN_NOISE_WDT, 0);
}

QWidget * DIOBoardWidget::makeTab(int32_t addr, int editorsType) const
{
	int paramProfileCount = getProfileCount(m_board, addr);
	if (!paramProfileCount)
		return nullptr;

	QWidget *tab = new QWidget;
	QGridLayout *tabLayout = new QGridLayout(tab);
	EditorsManager manager(nullptr, tabLayout);

	for (size_t i = 0; i < paramProfileCount; i++) {
		QString label = QString("Di%1").arg(i + 1);
		auto param = m_board->params()->get(addr, i);

		switch (editorsType)
		{
		case 0: manager.addLineEditor(param, label); break;
		case 1: manager.addCheckEditor(param, label); break;
		default: break;
		}		
	}

	manager.addStretch();
	return tab;
}
