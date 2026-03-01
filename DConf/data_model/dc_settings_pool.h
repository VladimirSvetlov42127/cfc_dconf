#pragma once
#include <vector>
#include "dc_setting.h"

class DcSettingsPool {

public:
	DcSettingsPool();
	~DcSettingsPool();

private:
	std::vector<DcSetting*> m_settings;
	
public:
	bool add(DcSetting *psetting, bool from_db = 0);
	void remove(uint32_t position);
	void remove(const QString &name);
	DcSetting *get(uint32_t position);
	DcSetting *get(const QString &name);
	uint32_t size(void);
	bool exist(const QString &name);
	void clear(void);
};