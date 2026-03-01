#pragma once
#include "dc_project_description.h"
#include "dc_conf_topology.h"
#include "dc_settings_pool.h"
#include <qstring.h>
#include "dc_pool.h"
#include <data_model/dc_controller.h>
#include <unordered_set>

#define CFC_ALG_IO_SIZE 32

#define gDataManager DcDataManager::getInstance()

using SoftCodeSet = std::unordered_set<int>;
using SoftCodeSetContainer = std::vector<SoftCodeSet>;

class DcDataManager
{
public:
	static DcDataManager& getInstance()
	{
		static DcDataManager    instance;
		return instance;
	}
private:
	DcDataManager();

	DcProjectDescription *m_projectDesc;
	DcConfTopology *m_topology;
	DcPoolSingleKey<DcController*> *m_controllers;
	SoftCodeSetContainer m_compatibleSoftCodes;

	DcDataManager(DcDataManager const&);

private:
	void loadCompatibleSoftCodes();

public:
	
	DcConfTopology *topology();
	DcProjectDescription * projectDesc();
	DcPoolSingleKey<DcController*> *controllers();
	void clear();
	void clearCurrentProgect(void);
	void clearController(int32_t controllerid);
	bool isCompatibleDevices(int softCode1, int softCode2);
};
