#include "DcVirtualFunctionsForm.h"

#include <functional>
#include <qscrollarea.h>
#include <QHeaderView>
#include <QStringList>
#include <QWidget>
#include <QHBoxLayout>



#include <gui/editors/EditorsManager.h>

namespace {
	const ListEditorContainer g_OnOff = { "Откл", "Вкл" };
	const ListEditorContainer g_Groups = { "1", "2", "3", "4"}; // группы уставок

	typedef std::function<bool(DcController *, DcSignal*, int)> ValidatorFunc;
	const QString header_style = "QHeaderView::section { background-color: #547586;	color: #EDF1F2;	padding-left: 4px; border: 1px solid #999999; }";
    const QStringList column_names = { "№", "Выход", "Функция", "Параметр" };
    int column_width[] = { 25, 180, 250, 250 };
	enum Column {
		NUMBER_COLUMN = 0,
		NAME_COLUMN,
		FUNCTION_COLUMN,
		PARAMETER_COLUMN,
		TOTAL_COLUMNS };

	// список не занятых виртуальных дискретных входов, дополнительно можно задать валидатор. 	
	// возвращаемое значение: первый список имена, второй список индексы
	ListEditorContainer virtualInputDiscretList(DcController *controller, ValidatorFunc validator = ValidatorFunc()) {
        QList <uint16_t> member_list;
        int index = 0;
        while (auto func_param = controller->getParam(SP_DOUT_TYPE, index * PROFILE_SIZE)) {
            auto func_value = controller->getParam(SP_DOUT_TYPE, index * PROFILE_SIZE + 1);
            uint16_t func = func_param->value().toUInt();
            auto func_type = controller->virtualFunctionType(func);
            uint16_t value = func_value->value().toUInt();
            if (func_type == DcController::VDIN_CONTROL || func_type == DcController::FIX_VDIN || func_type == DcController::VDIN_EVENT)
                if (!member_list.contains(value)) member_list.append(value);
            index++; }

        ListEditorContainer result;
        int idx = -1;
        for (auto s: controller->getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_VIRTUAL)) {
            idx++;
            bool exist = member_list.contains(idx);
            //if (member_list.contains(idx)) result.append({ QString("Виртуальный вход %1").arg(idx + 1), QString::number(idx) });
            if (!exist && controller->matrix_cfc()->existDst(s->index()))	continue;
            if (!exist && controller->matrix_signals()->existDst(s->index()))	continue;
            if (!exist && controller->matrix_alg()->existDst(s->index()))	continue;
            if (!exist && controller->isConnectionDiscret(s))	continue;
            if (validator && !validator(controller, s, idx)) continue;
            result.append({ QString("(%1) Виртуальный вход %2").arg(s->internalId()).arg(idx + 1), QString::number(idx)}); }

        return result;
	};

	ListEditorContainer fixedVirtualInputDiscretList(DcController *controller)
	{
		auto isFixBitSet = [=](DcController *controller, DcSignal* s, int idx) -> bool {
			DcParamCfg* p = controller->params_cfg()->get(SP_DIN_VDINFIXED, idx / 8);
			if (!p)
				return false;

			return p->value().toUInt() & (1 << (idx % 8));
		};

		return virtualInputDiscretList(controller, isFixBitSet);
	}

	ListEditorContainer xcbrList(DcController *controller)
	{
		QStringList xcbrList;
		DcParamCfg_v2 *xcbr = dynamic_cast<DcParamCfg_v2*>(controller->params_cfg()->get(SP_XCBRCFG, 0));
		if (xcbr)
			for (size_t i = 0; i < xcbr->getProfileCount(); i++)
				xcbrList << QString("Выключатель %1").arg(i + 1);

		return xcbrList;
	}
}

DcVirtualFunctionsForm::DcVirtualFunctionsForm(DcController *controller, const QString &path) :	DcForm(controller, path, "Настройки функций виртуальных выходов")
{
	DcParamCfg_v2 *functionsParam = dynamic_cast<DcParamCfg_v2*>(controller->params_cfg()->get(SP_DOUT_TYPE, 0));
	if (!functionsParam) return;
	QGridLayout* functionsWidgetLayout = new QGridLayout(centralWidget());

	//	Формирование таблицы
	int count = functionsParam->getProfileCount();
	_table = new QTableWidget(this);
	Table()->setRowCount(count);
	Table()->setColumnCount(Column::TOTAL_COLUMNS);
	Table()->verticalHeader()->setVisible(false);
	Table()->horizontalHeader()->setStyleSheet(header_style);
	Table()->setHorizontalHeaderLabels(column_names);
	Table()->horizontalHeader()->setFixedHeight(45);
	for (int i = 0; i < Column::TOTAL_COLUMNS; i++) Table()->horizontalHeader()->resizeSection(i, column_width[i]);

	//	Добавление таблицы на форму
	functionsWidgetLayout->addWidget(Table(), 1, 0);
	functionsWidgetLayout->setRowStretch(1, 1);
	functionsWidgetLayout->setColumnStretch(0, 1);

    //	Получение списка сигналов
    QList<DcSignal*> signals_list = controller->getSignalList(DEF_SIG_TYPE_DISCRETE, DEF_SIG_SUBTYPE_VIRTUAL, DEF_SIG_DIRECTION_OUTPUT);

	//	Заполнение текстовых полей таблицы
	for (int i = 0; i < count; i++) {
		QTableWidgetItem* item = new QTableWidgetItem(QString::number(i));
		item->setTextAlignment(Qt::AlignCenter);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);
		Table()->setItem(i, Column::NUMBER_COLUMN, item);
        item = new QTableWidgetItem(QString("Виртуальный выход ") + QString::number(i + 1));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);
		Table()->setItem(i, Column::NAME_COLUMN, item);	}

	//	Добавление в таблицу виджетов для выбора функций
	auto newFuncList = controller->virtualFunctionList();
	ListEditorContainer functionsList;
	for (auto& it : newFuncList)
		functionsList.append(it.second, it.first);

	for (int widget_row = 0; widget_row < count; widget_row++) {
		ListParamEditor* functionEditor = new ListParamEditor(controller->params_cfg()->get(SP_DOUT_TYPE, widget_row * PROFILE_SIZE), functionsList);
		Table()->setCellWidget(widget_row, Column::FUNCTION_COLUMN, functionEditor);
        DcSignal* out_signal = signals_list.at(widget_row);

		//	Лямбда функция
		auto onFunctionEditorChange = [=](const QString& value) {
			DcParam* p = controller->params_cfg()->get(SP_DOUT_TYPE, widget_row * PROFILE_SIZE + 1);
			Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, nullptr);
            QString template_name = QString("%1 <%2>");
            DcController::VirtualFunctionType functionType = controller->virtualFunctionType(value.toUInt());
            QString func_name = functionsList.text(QVariant(value));

            //	Встроенная лямбда функция
            auto onSignalChange = [=](const QString& data) {
                data == QString() ? out_signal->updateName(template_name.arg(func_name, "")) :
                                    out_signal->updateName(template_name.arg(func_name, QString::number(data.toInt() + 1))); };

            if (functionType == DcController::NOTUSE) {
                out_signal->updateName(QString("Виртуальный дискретный выход ") + QString::number(widget_row + 1));
                return;	}


			if (functionType == DcController::VDIN_CONTROL) {
                auto list = virtualInputDiscretList(controller);
                list.text(QVariant(p->value().toInt())) == QString() ? onSignalChange("") : onSignalChange(p->value());
				QWidget* widget = new QWidget;
				QHBoxLayout* layout = new QHBoxLayout(widget);
				layout->addWidget(new QLabel("Вход:"));
                ListParamEditor* virtualDiscretEditor = new ListParamEditor(p, list);
                connect(virtualDiscretEditor, &ListParamEditor::valueChanged, this, onSignalChange);
				virtualDiscretEditor->setFixedHeight(20);
				layout->addWidget(virtualDiscretEditor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
                return;	}

			if (functionType == DcController::FIX_VDIN) {
                auto list = fixedVirtualInputDiscretList(controller);
                list.text(QVariant(p->value().toInt())) == QString() ? onSignalChange("") : onSignalChange(p->value());
                QWidget* widget = new QWidget;
                QHBoxLayout* layout = new QHBoxLayout(widget);
                layout->addWidget(new QLabel("Вход:"));
                ListParamEditor* virtualDiscretEditor = new ListParamEditor(p, list);
                connect(virtualDiscretEditor, &ListParamEditor::valueChanged, this, onSignalChange);
				virtualDiscretEditor->setToolTip("Список свободных виртуальных дискретных входов, у которых установленна фиксация в 1");
				virtualDiscretEditor->setFixedHeight(20);
				layout->addWidget(virtualDiscretEditor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
                return;	}

			if (functionType == DcController::VDIN_EVENT) {
                auto list = virtualInputDiscretList(controller);
                list.text(QVariant(p->value().toInt())) == QString() ? onSignalChange("") : onSignalChange(p->value());
				QWidget* widget = new QWidget;
				QHBoxLayout* layout = new QHBoxLayout(widget);
				layout->addWidget(new QLabel("Вход:"));
                ListParamEditor* virtualDiscretEditor = new ListParamEditor(p, list);
                connect(virtualDiscretEditor, &ListParamEditor::valueChanged, this, onSignalChange);
				virtualDiscretEditor->setFixedHeight(20);
				layout->addWidget(virtualDiscretEditor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
				return;	}

            if (functionType == DcController::XCBR_CNTRL) {
                auto list = xcbrList(controller);
                list.text(QVariant(p->value().toInt())) == QString() ? onSignalChange("") : onSignalChange(p->value());
                QWidget* widget = new QWidget;
                QHBoxLayout* layout = new QHBoxLayout(widget);
                layout->addWidget(new QLabel("Выключатель:"));
                ListParamEditor* editor = new ListParamEditor(p, list);
                connect(editor, &ListParamEditor::valueChanged, this, onSignalChange);
                editor->setFixedHeight(20);
                layout->addWidget(editor);
                layout->setContentsMargins(5, 0, 5, 0);
                Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
                return;	}

			if (functionType == DcController::CONTROL_SV) {
                out_signal->updateName(template_name.arg(func_name, p->value().toInt() == 0 ? "Откл" : "Вкл"));
				QWidget* widget = new QWidget;
				QHBoxLayout* layout = new QHBoxLayout(widget);
				layout->addWidget(new QLabel("Параметр:"));
				ListParamEditor* editor = new ListParamEditor(p, g_OnOff);
                connect(editor, &ListParamEditor::valueChanged, this, [=](const QString& data) {
                    out_signal->updateName(template_name.arg(func_name, data.toInt() == 0 ? "Откл" : "Вкл")); });
				editor->setFixedHeight(20);
				layout->addWidget(editor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
				return;	}

			if (functionType == DcController::ACTIVE_GROUP) {
                g_Groups.text(QVariant(p->value().toInt())) == QString() ? onSignalChange("") : onSignalChange(p->value());
				QWidget* widget = new QWidget;
				QHBoxLayout* layout = new QHBoxLayout(widget);
				layout->addWidget(new QLabel("Группа уставок:"));
				ListParamEditor* editor = new ListParamEditor(p, g_Groups);
                connect(editor, &ListParamEditor::valueChanged, this, onSignalChange);
				editor->setFixedHeight(20);
				layout->addWidget(editor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
				return;	}

			if (functionType == DcController::XCBR_RZA_CNTRL) {
                out_signal->updateName(template_name.arg(func_name, p->value()));
				QWidget* widget = new QWidget;
				QHBoxLayout* layout = new QHBoxLayout(widget);
				layout->addWidget(new QLabel("Параметр:"));
				LineParamEditor* editor = new LineParamEditor(p);
                connect(editor, &ListParamEditor::valueChanged, this, [=](const QString& data) {
                    out_signal->updateName(template_name.arg(func_name, data)); });
				editor->setFixedHeight(20);
				layout->addWidget(editor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
				return;	}

			if (functionType == DcController::VDOUT_CONFIRM) {
                out_signal->updateName(template_name.arg(func_name, p->value()));
				QWidget* widget = new QWidget;
				QHBoxLayout* layout = new QHBoxLayout(widget);
				layout->addWidget(new QLabel("Параметр:"));
				LineParamEditor* editor = new LineParamEditor(p);
                connect(editor, &ListParamEditor::valueChanged, this, [=](const QString& data) {
                    out_signal->updateName(template_name.arg(func_name, data)); });
				editor->setFixedHeight(20);
				layout->addWidget(editor);
				layout->setContentsMargins(5, 0, 5, 0);
				Table()->setCellWidget(widget_row, Column::PARAMETER_COLUMN, widget);
				return;	}

            out_signal->updateName(func_name);
            return;
		};
	
		onFunctionEditorChange(functionEditor->param()->value());
		connect(functionEditor, &ListParamEditor::valueChanged, this, onFunctionEditorChange);	}

}

bool DcVirtualFunctionsForm::isAvailableFor(DcController * controller)
{
	static QList<Param> params = {
		{SP_DOUT_TYPE}
	};

	return hasAny(controller, params);
}

void DcVirtualFunctionsForm::fillReport(DcIConfigReport * report)
{
	auto device = report->device();

	auto newFuncList = device->virtualFunctionList();
	ListEditorContainer functionsList;
	for (auto& it : newFuncList) functionsList.append(it.second, it.first);
	auto virtualDiscretList = virtualInputDiscretList(device);
	auto fixedVirtualDiscretList = fixedVirtualInputDiscretList(device);
	auto xcbrList = ::xcbrList(device);

	report->insertSection();

	QStringList headers = {Text::ReportTable::Name, "Тип", "Аргумент" };
	DcReportTable table(report->device(), headers, {40});

	DcParamCfg_v2 *functionsParam = dynamic_cast<DcParamCfg_v2*>(device->getParam(SP_DOUT_TYPE, 0));
	for (size_t i = 0; i < functionsParam->getProfileCount(); i++) {
		auto name = QString("Функция %1").arg(i + 1);
		
		auto fParam = device->getParam(SP_DOUT_TYPE, i * PROFILE_SIZE);
		auto type = functionsList.text(fParam->value());

		QString arg = device->params_cfg()->get(SP_DOUT_TYPE, i * PROFILE_SIZE + 1)->value();
		switch (device->virtualFunctionType(fParam->value().toUInt()))
		{
		case DcController::VDIN_CONTROL:
		case DcController::VDIN_EVENT:
			arg = virtualDiscretList.text(arg);
			break;
		case DcController::XCBR_CNTRL:
			arg = xcbrList.text(arg);
			break;
		case DcController::FIX_VDIN:
			arg = fixedVirtualDiscretList.text(arg);
			break;
		default:
			break;
		}

		table.addRow({name, type, arg });
	}

	report->insertTable(table);
}

