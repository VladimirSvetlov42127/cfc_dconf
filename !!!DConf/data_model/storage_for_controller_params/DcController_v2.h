#pragma once
#include <data_model/dc_controller.h>

#include <qsharedpointer.h>


class DcController_v2 :
	public DcController
{
public:
    DcController_v2(int32_t controllerid, const QString &filepath, const QString &name);
	virtual ~DcController_v2();

	virtual DcParamCfgPool<DcParamCfg*> *params_cfg() const override;	// Основная функция, которая должна быть переписана

	QVector<QSharedPointer<DcParamCfg_v2_Headline>> *headlines();
	QSharedPointer<DcParamCfg_v2_Headline> getHeadlineForAddress(uint16_t);

    DcController_v2* clone(int32_t index, const QString &filepath, const QString &name) const;

private:
	DcParamCfgPool<DcParamCfg*> *m_params_cfg_v2;	// Массив объектов, унаследованных от DcParamCfg
	QVector<QSharedPointer<DcParamCfg_v2_Headline>> *mv_headlines;	// Здесь общая инфо для групп параметров с одним адресом
};

