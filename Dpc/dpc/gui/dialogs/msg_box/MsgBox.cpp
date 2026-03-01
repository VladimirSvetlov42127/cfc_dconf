#include "MsgBox.h"

#include <qmessagebox.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qdebug.h>
#include <QPushButton>

#include <dpc/gui/gui.h>

namespace {
	int execMsgBox(QMessageBox *box) {
		box->setWindowIcon(QApplication::windowIcon());
		return box->exec();
	}
} // namespace

namespace Dpc::Gui
{
	void MsgBox::info(const QString& msg)
	{
		QMessageBox msgBox(QMessageBox::Information, "Информация", msg, QMessageBox::Ok);
		msgBox.setIconPixmap(QPixmap(":/icons/32/info.png"));
		execMsgBox(&msgBox);
	}

	void MsgBox::warning(const QString& msg)
	{
		QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", msg, QMessageBox::Ok);
		msgBox.setIconPixmap(QPixmap(":/icons/32/warning.png"));
		execMsgBox(&msgBox);
	}

	void MsgBox::error(const QString& msg)
	{
		QMessageBox msgBox(QMessageBox::Critical, "Ошибка", msg, QMessageBox::Ok);
		msgBox.setIconPixmap(QPixmap(":/icons/32/error.png"));
		execMsgBox(&msgBox);
	}

	bool MsgBox::question(const QString& msg)
	{
		QMessageBox msgBox(QMessageBox::Question, "Подтверждение", msg);
		msgBox.setIconPixmap(QPixmap(":/icons/32/question.png"));
        auto yesButton = msgBox.addButton("Да", QMessageBox::AcceptRole);
		auto noButton = msgBox.addButton("Нет", QMessageBox::RejectRole);
        msgBox.setDefaultButton(noButton);
        execMsgBox(&msgBox);

        return msgBox.clickedButton() == yesButton;
	}
} // namespace
