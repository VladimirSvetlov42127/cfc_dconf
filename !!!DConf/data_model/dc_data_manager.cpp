#include "dc_data_manager.h"

#include <QFile>

#include "utils/cfg_path.h"

//namespace {
//    const QString COMPATIBLE_SOFT_CODES_FILE = cfg::path::templates() + "compatible_devices.txt";
//}

DcDataManager::DcDataManager()
{
	m_projectDesc = new DcProjectDescription();
	m_topology = new DcConfTopology();
	m_controllers = new DcPoolSingleKey<DcController*>();

	loadCompatibleSoftCodes();
}

void DcDataManager::loadCompatibleSoftCodes()
{
	QFile file(cfg::path::getCompatibleDevices());
	if (!file.open(QIODevice::ReadOnly))
		return;

	for (auto &line : QString(file.readAll()).split('\n')) {
		auto softCodesList = line.split("//");
		if (softCodesList.isEmpty())
			continue;

		SoftCodeSet set;
		for (auto &code : softCodesList.first().split(',')) {
			bool ok = false;
			int val = code.trimmed().toInt(&ok);
			if (!ok)
				continue;

			set.insert(val);
		}

		if (!set.empty())
			m_compatibleSoftCodes.push_back(set);
	}
}

DcConfTopology * DcDataManager::topology()
{
	return m_topology;
}

void DcDataManager::clear(void) {
	m_projectDesc->save(cfg::path::getProjectInfo());
	m_projectDesc->clear();
}

void DcDataManager::clearCurrentProgect(void) {
	m_projectDesc->clear();
	m_controllers->clear();
	m_controllers->removeAll();
	m_topology->clear();

}

void DcDataManager::clearController(int32_t controllerid) {
	DcController *pcontroller = m_controllers->getById(controllerid);
	if (pcontroller == nullptr)
		return;

	m_controllers->clear(controllerid);
}

bool DcDataManager::isCompatibleDevices(int softCode1, int softCode2)
{
	if (softCode1 == softCode2)
		return true;

	for (auto &codeList : m_compatibleSoftCodes) {
		auto it = codeList.find(softCode1);
		if (it != codeList.end())
			return codeList.find(softCode2) != codeList.end();		
	}

	return false;
}

DcProjectDescription * DcDataManager::projectDesc() {
	return 	m_projectDesc;
}

DcPoolSingleKey<DcController*>* DcDataManager::controllers()
{
	return m_controllers;
}
