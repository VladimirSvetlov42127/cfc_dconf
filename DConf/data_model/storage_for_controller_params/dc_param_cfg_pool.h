#pragma once
#include <vector>
#include "dc_param_cfg.h"

template< class T >
class DcParamCfgPool
{

private:
	std::vector<T> datavect;

public:
	DcParamCfgPool() {};
	virtual ~DcParamCfgPool() {};

	bool add(T elem, bool from_db) {
		datavect.push_back(elem);
		if (!from_db)
			elem->insert();
		return true;
	}

	T get(uint32_t position) {
		if (position >= datavect.size())
			return nullptr;
		return datavect[position];
	}

	uint32_t size(void) {
		return datavect.size();
	}

	T get(int32_t addr, int32_t param)
	{
		if ((addr < 1) || (param < 0))
			return nullptr;
		for (T pelement : datavect) {
			if ((pelement->addr() == addr)&&(pelement->param() == param))
				return pelement;
		}
		return nullptr;
	}

	bool exist(int32_t addr, int32_t param) {
		for (T pelement : datavect) {
			if ((pelement->addr() == addr) && (pelement->param() == param))
				return true;
		}
		return false;
	}

	bool remove(int32_t addr, int32_t index) {
		for(auto it = datavect.begin(); it != datavect.end(); it++)
			if ((*it)->addr() == addr && (*it)->param() == index) {
				(*it)->remove();
				delete (*it);
				datavect.erase(it);
				return true;
			}
		return false;
	}

	void clear() {
		for (T elem : datavect) {
			if (elem != nullptr) {
				delete elem;
				elem = nullptr;
			}
		}
		datavect.clear();
	}
};

