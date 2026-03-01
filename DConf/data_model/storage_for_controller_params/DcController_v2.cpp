#include "DcController_v2.h"



#include <data_model/storage_for_controller_params/DcParamCfgPool_v2.h>
#include <data_model/storage_for_controller_params/dc_param_cfg_pool.h>


DcController_v2::DcController_v2(int32_t controllerid, const QString &filepath, const QString &name)
    : DcController(controllerid, filepath, name)
{
	m_params_cfg_v2 = new DcParamCfgPool_v2<DcParamCfg*> ();

	mv_headlines = new QVector<QSharedPointer<DcParamCfg_v2_Headline>>;
}


DcController_v2::~DcController_v2()
{
}

DcParamCfgPool<DcParamCfg*>* DcController_v2::params_cfg() const
{
	return m_params_cfg_v2;
}

QVector<QSharedPointer<DcParamCfg_v2_Headline>> *DcController_v2::headlines()
{
	return mv_headlines;
}

QSharedPointer<DcParamCfg_v2_Headline> DcController_v2::getHeadlineForAddress(uint16_t addr)
{
	for (QSharedPointer<DcParamCfg_v2_Headline> item : *mv_headlines) {
		if (item->getAddress() == addr) {
			return item;
		}
	}
	return nullptr;
}

DcController_v2 * DcController_v2::clone(int32_t controllerid, const QString &filepath, const QString &name) const
{
    auto tempDevice = new DcController_v2(controllerid, filepath, name);

	for (size_t i = 0; i < settings()->size(); i++) {
		auto st =settings()->get(i);
        auto newSt = new DcSetting(controllerid, st->name(), st->value());
		tempDevice->settings()->add(newSt, true);
	}
	
	for (auto sg: getSignalList()) {
        auto newSg = new DcSignal(controllerid, sg->index(), sg->internalId(), sg->direction(), sg->type(), sg->subtype(), sg->name(), sg->properties());
		tempDevice->addSignal(newSg, true);
	}

	for (size_t i = 0; i < algs_internal()->size(); i++) {
		auto alg = algs_internal()->get(i);
        auto newAlg = new DcAlgInternal(controllerid, alg->index(), alg->position(), alg->name(), alg->properties());

		for (size_t j = 0; j < alg->ios()->size(); j++) {
			auto ios = alg->ios()->get(j);
            auto newIos = new DcAlgIOInternal(controllerid, ios->index(), ios->alg(), ios->pin(), ios->direction(), ios->name());
			newAlg->ios()->add(newIos, true);
		}

		tempDevice->algs_internal()->add(newAlg, true);
	}

	for (size_t i = 0; i < boards()->size(); i++) {
		auto board = boards()->get(i);
        auto newBoard = new DcBoard(controllerid, board->index(), board->slot(), board->type(), board->inst(), board->properties());
		
		for (size_t j = 0; j < board->params()->size(); j++) {
			auto param = board->params()->get(j);
            auto newParam = new DcParamCfgBoard(controllerid, param->board(), param->addr(), param->param(), param->name(), param->type(), param->flags(), param->value(), param->properties());
			newBoard->params()->add(newParam, true);
		}
	}

	QHash<uint16_t, QSharedPointer<DcParamCfg_v2_Headline>> headLinesHash;
	for (auto it : *mv_headlines) {
		headLinesHash[it->getAddress()] = it;
		tempDevice->headlines()->append(it);
	}

	for (size_t i = 0; i < params_cfg()->size(); i++) {
		auto param = params_cfg()->get(i);
		auto hl = headLinesHash[param->addr()];
		if (!hl)
			continue;

        auto newParam = new DcParamCfg_v2(controllerid, param->name(), param->addr(), param->param(), param->value(), hl);
		tempDevice->params_cfg()->add(newParam, true);
	}

	tempDevice->setChannel(channel());
	return tempDevice;
}
