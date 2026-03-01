#include "DcBoardsForm.h"

#include <gui/editors/EditorsManager.h>
#include <gui/forms/boards/DIOBoardWidget.h>

namespace {
	ListEditorContainer availableBoards(DcController *device)
	{
		ListEditorContainer wItems;
		for (size_t i = 0; i < device->boards()->size(); i++) {
			auto b = device->boards()->get(i);
			if (b->slot() < 1)
				continue;

			wItems.append({ QString("%1 %2").arg(b->type()).arg(b->slot()), b->index() });
		}

		return wItems;
	}
}

DcBoardsForm::DcBoardsForm(DcController *controller, const QString &path) :
	DcForm(controller, path, "Настройки плат расширения")
{
	auto wCreator = [=](const QVariant &data) -> QWidget* {
		auto board = controller->boards()->getById(data.toInt());
		if (!board)
			return nullptr;

		return new DIOBoardWidget(board);
	};

	QGridLayout *layout = new QGridLayout(centralWidget());
	EditorsManager manager(controller, layout);
	manager.addWidgetCreatorLayout(manager.addListEditor("Плата", availableBoards(controller)), wCreator);
	layout->setColumnStretch(2, 1);
}

bool DcBoardsForm::isAvailableFor(DcController * controller)
{
	for (size_t i = 0; i < controller->boards()->size(); i++)
		if (controller->boards()->get(i)->slot() > 0)
			return true;

	return false;
}

void DcBoardsForm::fillReport(DcIConfigReport * report)
{
	for (auto &it : availableBoards(report->device())) {
		auto board = report->device()->boards()->getById(it.value.toInt());
		if (!board)
			continue;

		report->insertSection(it.text);
		DIOBoardWidget::fillReport(board, report);
	}
}
