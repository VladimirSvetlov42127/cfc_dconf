#pragma once

#include <functional>
#include <map>
#include <qicon.h>

#include <report/DcIConfigReport.h>

#define ADD_MENU(MENU_NAME, ICON_PATH, TITLE, PARENT) \
static DcMenu& MENU_NAME() { static DcMenu m(ICON_PATH, TITLE, &PARENT(), __LINE__); return m; }

class DcController;

class DcMenu
{
	DcMenu(const QString &iconPath, const QString &title = QString(), DcMenu *parent = nullptr, int pos = 0);

public:
	using CheckFuncType = std::function<bool(DcController*)>;
	using FillReportFuncType = std::function<void(DcIConfigReport*)>;

public:
	DcMenu(const DcMenu &) = delete;
	DcMenu(DcMenu &&) = delete;
	DcMenu& operator=(const DcMenu&) = delete;
	DcMenu& operator=(DcMenu &&) = delete;	

	bool isAvailableFor(DcController *controller) const;
	void fillReport(DcIConfigReport *report) const;
	std::map<int, DcMenu*> childs() const { return m_childs; }
	QString title() const { return m_title; }
	QIcon icon() const { return QIcon(m_iconPath); }
	QString formIdx() const { return m_formIdx; }

	void setCheckFunc(CheckFuncType func) { m_checkFunc = func; }
	void setFormIdx(const QString &formIdx) { m_formIdx = formIdx; }
	void setFillReportFunc(FillReportFuncType func) { m_fillReportFunc = func; }

	static DcMenu& root() { static DcMenu m(":/icons/24/unit.png"); return m; }
	
	ADD_MENU(general_settings, ":/icons/24/setting_3.png", "Общие настройки", root);

	ADD_MENU(input_output, ":/icons/24/exchange.png", "Входы/выходы", root);
		ADD_MENU(discret_inputs, ":/icons/24/din.png", "Дискретные входы", input_output);
			ADD_MENU(discret_input_channels, ":/icons/24/chanel_settings.png", "Поканальные настройки", discret_inputs);
				ADD_MENU(dins_physical, ":/icons/24/signal_in.png", "Физические входы", discret_input_channels);
				ADD_MENU(dins_logical, ":/icons/24/signal_in.png", "Логические входы", discret_input_channels);
				ADD_MENU(dins_virtual, ":/icons/24/signal_in.png", "Виртуальные входы", discret_input_channels);
				ADD_MENU(dins_external, ":/icons/24/signal_in.png", "Внешние входы", discret_input_channels);
			ADD_MENU(discret_2pos_channels, ":/icons/24/double_point.png", "Двухпозиционные сигналы", discret_inputs);
			ADD_MENU(discret_input_adc, ":/icons/24/acp.png", "Входы АЦП", discret_inputs);
		ADD_MENU(discret_outputs, ":/icons/24/dout.png", "Дискретные выходы", input_output);
			ADD_MENU(discret_output_channels, ":/icons/24/chanel_settings.png", "Поканальные настройки", discret_outputs);			
			ADD_MENU(block_control, ":/icons/24/block.png", "Блокировка управления", discret_outputs);
			ADD_MENU(output_clones, ":/icons/24/double_point.png", "Дублирование физических выходов", discret_outputs);
		ADD_MENU(analog_inputs, ":/icons/24/ain.png", "Аналоговые входы", input_output);
			ADD_MENU(analogs_aperture, ":/icons/24/edit_0.png", "Апертуры и пороги", analog_inputs);

	ADD_MENU(thresholds_settings, ":/icons/24/limits.png", "Уставки/пороги", root);
		ADD_MENU(thresholds, ":/icons/24/pke_limit.png", "Пороги", thresholds_settings);
		ADD_MENU(pqi_settings, ":/icons/24/pke.png", "Уставки ПКЭ", thresholds_settings);
		ADD_MENU(pqi_event_settings, ":/icons/24/pke_event.png", "Уставки событий ПКЭ", thresholds_settings);

	ADD_MENU(interface_protocols, ":/icons/24/stack.png", "Интерфейсы/протоколы", root);
		ADD_MENU(sim, ":/icons/24/sim.png", "SIM", interface_protocols);
		ADD_MENU(time_sync, ":/icons/24/rclock.png", "Синхронизация времени", interface_protocols);
		ADD_MENU(rs485, ":/icons/24/rs485.png", "RS-485", interface_protocols);
		ADD_MENU(tcp, ":/icons/24/ethernet_fr.png", "TCP", interface_protocols);
		ADD_MENU(iec101_104, ":/icons/24/iec_104_fr.png", "МЭК 60870-5-101/104", interface_protocols);
			ADD_MENU(iec101_104_settings, ":/icons/24/setting_2.png", "Параметры общие", iec101_104);
			ADD_MENU(iec101_104_channels, ":/icons/24/chanel_settings.png", "Поканальные", iec101_104);
		ADD_MENU(iec103, ":/icons/24/iec_103_fr.png", "МЭК 60870-5-103", interface_protocols);
			ADD_MENU(iec103_settings, ":/icons/24/setting_2.png", "Параметры общие", iec103);
			ADD_MENU(iec103_channels, ":/icons/24/chanel_settings.png", "Поканальные", iec103);
		ADD_MENU(iec61850, ":/icons/24/iec61850_fr.png", "МЭК 61850", interface_protocols);
		ADD_MENU(spodes, ":/icons/24/spodes.png", "CПОДЭС", interface_protocols);
			ADD_MENU(spodes_settings, ":/icons/24/setting_2.png", "Параметры общие", spodes);
			ADD_MENU(spodes_channels, ":/icons/24/chanel_settings.png", "Поканальные", spodes);		

	ADD_MENU(functions, ":/icons/24/function.png", "Функции", root);
		ADD_MENU(analogs_functions, ":/icons/24/transformation.png", "Аналоги", functions);
		ADD_MENU(counters_functions, ":/icons/24/transformation.png", "Счётчики", functions);
		ADD_MENU(virtual_functions, ":/icons/24/virtual_function.png", "Функции виртуальных выходов", functions);
		ADD_MENU(switches, ":/icons/24/switches.png", "Выключатели", functions);
		ADD_MENU(oscillography, ":/icons/24/oscillogram.png", "Осциллографирование", functions);
		ADD_MENU(auv_urov, ":/icons/24/document.png", "АУВ/УРОВ", functions);

	ADD_MENU(algorithms, ":/icons/24/algorithm.png", "Алгоритмы", root);
	ADD_MENU(algorithms_embedded, ":/icons/24/algorithm_built_in.png", "Встроенные", algorithms);
	ADD_MENU(algorithms_custom, ":/icons/24/algorithm_custom.png", "Пользовательские", algorithms);

	ADD_MENU(converter, ":/icons/24/convert.png", "Конвертер", root);

	ADD_MENU(matrix, ":/icons/24/bind_table.png", "Таблица привязки", root);

private:
	std::map<int, DcMenu*> m_childs;

	QString m_iconPath;
	QString m_title;	
	
	CheckFuncType m_checkFunc;
	QString m_formIdx;
	FillReportFuncType m_fillReportFunc;
};