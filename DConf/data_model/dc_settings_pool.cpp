#include "dc_settings_pool.h"

using namespace std;

DcSettingsPool::DcSettingsPool()
{
}

DcSettingsPool::~DcSettingsPool()
{
	for (DcSetting *pelement : m_settings) {
		if (pelement != nullptr) {
			delete pelement;
			pelement = nullptr;
		}
	}
	m_settings.clear();
}

bool DcSettingsPool::add(DcSetting *psetting, bool from_db)
{
	if (psetting == nullptr)
		return false;

	if ((psetting->name().isEmpty()) || (psetting->value().isEmpty()))
		return false;

	if (exist(psetting->name()))
		return false;

	m_settings.push_back(psetting);

	if (!from_db)
		psetting->insert();
	return true;
}

void DcSettingsPool::remove(uint32_t position)
{
	if (position >= m_settings.size())
		return;
	DcSetting *psetting = m_settings[position];
	if (psetting == nullptr)
		return;
	psetting->remove();
	auto it = std::find(m_settings.begin(), m_settings.end(), psetting);
	if (it != m_settings.end())
		m_settings.erase(it);
}

void DcSettingsPool::remove(const QString &name)
{
	if (name.isEmpty())
		return;
	DcSetting *psetting = get(name);
	if (psetting == nullptr)
		return;
	psetting->remove();
	auto it = std::find(m_settings.begin(), m_settings.end(), psetting);
	if (it != m_settings.end())
		m_settings.erase(it);
}

DcSetting *DcSettingsPool::get(uint32_t position)
{
	if (position >= m_settings.size())
		return nullptr;
	return m_settings[position];
}

DcSetting *DcSettingsPool::get(const QString &name)
{
	if (name.isEmpty())
		return nullptr;
	for (DcSetting *pelement : m_settings) {
		if (pelement->name().compare(name)==0)
			return pelement;
	}
	return nullptr;
}

uint32_t DcSettingsPool::size(void)
{
	return m_settings.size();
}

bool DcSettingsPool::exist(const QString &name)
{
	for (DcSetting *pelement : m_settings) {
		if (pelement->name().compare(name) == 0)
			return true;
	}
	return false;
}

void DcSettingsPool::clear(void) {
	if (m_settings.empty())
		return;
	for (auto it = m_settings.begin(); it != m_settings.end(); ++it) {
		delete *(it);
		*(it) = nullptr;
	}
	m_settings.clear();
}