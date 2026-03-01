#pragma once

#include <qstring.h>
#include <qvector.h>
#include <qsharedpointer.h>
#include <qobject.h>

#include <data_model/storage_for_controller_params/dc_param_cfg.h>	// Старая реализация
#include <data_model/storage_for_controller_params/DcParamCfg_v2_Headline.h>	// Общая информация для группы параметров

class __Helper : public QObject
{
	Q_OBJECT
public:
signals:
	void valueChanged(const QString &value);
};

class DcParamCfg_v2 : public DcParamCfg
{
public:
    DcParamCfg_v2(int32_t controllerid, const QString &name, uint32_t addr, uint32_t ind, const QString &value, QSharedPointer<DcParamCfg_v2_Headline> headline = nullptr);
	~DcParamCfg_v2();

	//// Переопределяем функции из DcParam
	virtual int32_t addr() override;
	virtual int32_t param() override;
	virtual QString name() override;
	virtual DefParamDataType type() override;
	virtual attribType_t flags() override;	// В старом варианте использовались только Чтение/Запись
	virtual QString value() override;

	virtual bool was_changed() override;						// Если параметр был обновлен, ставим true. После процесса чтения все должны становитсья false
	virtual void reset_param_changes() override;				// Сбрасывает флаг, что параметр был изменен

	virtual bool updateValue(QString newval) override;
	virtual bool updateName(QString newname) override;

	virtual QString property(QString name) override;
	virtual bool updateProperty(QString &name, QString &value) override;

	virtual void updatedb(void) override;

	virtual void update() override;
	virtual void insert() override;
	virtual void remove() override;
	//// Конец переопределения функций из DcParam

	uint16_t getProfileCount();
	uint16_t getSubProfileCount();
	uint16_t getDataLenInBytes();

	__Helper* helper() { return &m_helper; }

private:

	QString m_name;	// Имя параметра
	uint16_t m_address;	// Пока храним дополнительно, вдруг понадобится найти заголовок
	uint16_t m_index;	// Индекс относительно адреса
	QString m_value;	// Значения параметра

	QSharedPointer<DcParamCfg_v2_Headline> m_headline;	// Указатель на общие данные

	bool m_wasChanged;	

	__Helper m_helper;
};

