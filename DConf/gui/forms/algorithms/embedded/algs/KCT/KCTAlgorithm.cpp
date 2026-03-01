#include "KCTAlgorithm.h"

#include <gui/editors/EditorsManager.h>

namespace {
	ALG_ADDRS_FUNC(SP_KCT_WORDIN_PARAM)
}

namespace Text {
	const QString Byte1 = "Показывать небаланс";
}

KCTAlgorithm::KCTAlgorithm() :
    Board_Algorithm(BoardType::AFE7 | BoardType::AFE11, alg_addrs().bytes)
{
}

QWidget* KCTAlgorithm::createWidget() const
{
	auto widget = new QWidget;
	QGridLayout *layout = new QGridLayout(widget);

	for (size_t i = 0; i < groupCount(); i++) {
		auto group = widgetForProfile(i);
		EditorsManager em(controller(), new QGridLayout(group));

		uint16_t floatsParam = alg_addrs().floats;
		uint16_t bytesParam = alg_addrs().bytes;

		em.addCheckEditor(bytesParam, alg_index(i, 1), Text::Byte1);

		em.addLineEditor(floatsParam, alg_index(i, 0), Algs::Text::ActuationAmperage, 0, std::numeric_limits<float>::max());// 300);
		em.addLineEditor(floatsParam, alg_index(i, 1), Algs::Text::ActuationTime, 0, std::numeric_limits<float>::max());// 300);
		em.addLineEditor(floatsParam, alg_index(i, 2), Algs::Text::ReturnRate, 0, 1);

		layout->addWidget(group, 0, i);
	}

	layout->setColumnStretch(layout->columnCount(), 1);
	layout->setRowStretch(layout->rowCount(), 1);
	return widget;
}

void KCTAlgorithm::fillReport(DcReportTable * table) const
{
	QList<DcReportTable::ParamRecord> byte1;
	QList<DcReportTable::ParamRecord> float0;
	QList<DcReportTable::ParamRecord> float1;
	QList<DcReportTable::ParamRecord> float2;

	for (size_t i = 0; i < groupCount(); i++) {
		uint16_t floatsParam = alg_addrs().floats;
		uint16_t bytesParam = alg_addrs().bytes;

		byte1 << DcReportTable::ParamRecord(bytesParam, alg_index(i, 1), "", true);

		float0 << DcReportTable::ParamRecord(floatsParam, alg_index(i, 0));
		float1 << DcReportTable::ParamRecord(floatsParam, alg_index(i, 1));
		float2 << DcReportTable::ParamRecord(floatsParam, alg_index(i, 2));
	}

	table->addRow(Text::Byte1, byte1);
	table->addRow(Algs::Text::ActuationAmperage, float0);
	table->addRow(Algs::Text::ActuationTime, float1);
	table->addRow(Algs::Text::ReturnRate, float2);
}
