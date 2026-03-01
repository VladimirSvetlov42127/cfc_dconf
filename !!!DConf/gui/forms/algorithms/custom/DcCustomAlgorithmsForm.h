#pragma once

#include <gui/forms/DcFormFactory.h>

#include <file_managers/DcFlexLogicFileManager.h>

class QTableView;
class QPushButton;

class DcAlgCfc;

class DcCustomAlgorithmsForm : public DcForm
{
	Q_OBJECT

public:
	DcCustomAlgorithmsForm(DcController *controller, const QString &path);
	~DcCustomAlgorithmsForm();

	static bool isAvailableFor(DcController *controller);
	static void fillReport(DcIConfigReport *report);

private slots:
	void update();

	void onAddButton();
	void onRemoveButton();
	void onCompileButton();
	void onViewDoubleClicked(const QModelIndex &index);
	void onEditorAboutToClose();

private:
	DcAlgCfc* alg(int num) const;

	void editAlgorithm(DcAlgCfc* alg);
	bool createFile(uint32_t algNum);
	void removeAlgorithm(DcAlgCfc* alg);
	bool compileAlgorithm(DcAlgCfc* alg);
	void setAlgorithmEnabled(DcAlgCfc* alg, bool enabled);
	void setCompiled(DcAlgCfc* alg, bool compiled);

private:
	QTableView *m_algsTableView;
	QPushButton *m_addButton;
	QPushButton *m_removeButton;
	QPushButton *m_compileButton;

	DcFlexLogicFileManager m_fm;
};

REGISTER_FORM(DcCustomAlgorithmsForm, DcMenu::algorithms_custom);

