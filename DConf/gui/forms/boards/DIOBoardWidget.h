#pragma once

#include <qwidget.h>

class DcBoard;
class DcIConfigReport;

class DIOBoardWidget : public QWidget
{

public:
	DIOBoardWidget(DcBoard *board, QWidget *parent = nullptr);

	static void fillReport(DcBoard *board, DcIConfigReport *report);
	
private:	
	QWidget *createCommonTab() const;
	QWidget *createDrebezgTab() const;
	QWidget *createInversionTab() const;
	QWidget *createTimerTab() const;
	QWidget *createInterferenceTab() const;

	QWidget* makeTab(int32_t addr, int editorsType) const;

private:
	DcBoard *m_board;
};