#include "dc_conf_topology.h"

#include "utils/cfg_path.h"

#include "dpc/iconvlite.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QDir>

using namespace std;

namespace {
	const QString element_id = "id";
	const QString element_pid = "pid";
	const QString element_type = "type";
	const QString element_name = "name";
	const QString element_desc = "desc";

	const QString root_tree = "tree";
}


DcConfTopology::DcConfTopology(){

}

DcConfTopology::~DcConfTopology() {

}

void DcConfTopology::clear(void) {
	if (m_topology.empty())
		return;
	for (auto it = m_topology.begin(); it != m_topology.end(); ++it) {
		delete *(it);
		*(it) = nullptr;
	}
	m_topology.clear();
}

bool DcConfTopology::add(DcConfItem *pitem) {
	if (pitem == nullptr)
		return false;
	m_topology.push_back(pitem);
	update();
	toJson();

	return true;
}

void DcConfTopology::remove(uint32_t position) {
	if (position >= m_topology.size())
		return;
	DcConfItem *pitem = m_topology[position];
	if (pitem == nullptr)
		return;
	auto it = std::find(m_topology.begin(), m_topology.end(), pitem);
	if (it != m_topology.end()) {
		m_topology.erase(it);
		update();
		toJson();
	}
}

void DcConfTopology::removeById(int32_t index) {
	if (index < 1)
		return;
	DcConfItem *pitem = getById(index);
	if (pitem == nullptr)
		return;
	auto it = std::find(m_topology.begin(), m_topology.end(), pitem);
	if (it != m_topology.end()) {
		m_topology.erase(it);
		update();
		toJson();
	}
}

DcConfItem *DcConfTopology::get(uint32_t position) {
	if (position >= m_topology.size())
		return nullptr;
	return m_topology[position];
}

DcConfItem *DcConfTopology::getById(int32_t index) {
	if (index < 1)
		return nullptr;
	for (DcConfItem *pelement : m_topology) {
		if (pelement->id() == index)
			return pelement;
	}
	return nullptr;
}

DcConfItem *DcConfTopology::root(void) {
	if (size() < 1)
		return nullptr;
	for (DcConfItem *pelement : m_topology) {
		if (pelement->parent_id() == 0)
			return pelement;
	}
	return nullptr;
}

list<DcConfItem*> DcConfTopology::childs(int32_t index) {
	list<DcConfItem*> res;
	for (DcConfItem *pelement : m_topology) {
		if (pelement->parent_id() == index)
			res.push_back(pelement);
	}
	return res;
}

uint32_t DcConfTopology::size(void) {
	return m_topology.size();
}

bool DcConfTopology::exist(int32_t index) {
	for (DcConfItem *pelement : m_topology) {
		if (pelement->id() == index)
			return pelement;
	}
	return false;
}

int32_t DcConfTopology::maxIndex(void) {
	int curindex = 0;
	for (DcConfItem *pelement : m_topology) {
		if (pelement->id() > curindex)
			curindex = pelement->id();
	}
	return curindex;
}

bool DcConfTopology::fromJson(void) {
	QString path = cfg::path::getTopology();
	QFile File(path);
	if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QByteArray data = File.readAll();
	data = conv().toUtf8(data).toUtf8();

	QJsonParseError JsonParseError;
	QJsonDocument json_doc = QJsonDocument::fromJson(data, &JsonParseError);

	File.close();

	if (JsonParseError.error != QJsonParseError::NoError)
		return false;

	QJsonObject obj_root = json_doc.object();
	QJsonArray jsonArray = obj_root[root_tree].toArray();
	foreach(const QJsonValue & value, jsonArray) {
		QJsonObject obj = value.toObject();
		int32_t id = obj[element_id].toInt();
		if (id > 0) {
			int32_t pid = obj[element_pid].toInt();
			QString tp = obj[element_type].toString();
			QString nm = obj[element_name].toString();
			QString desc = obj[element_desc].toString();

			DcConfItem *item = new DcConfItem(id, pid, tp, nm, desc);
			if (item != nullptr)
				m_topology.push_back(item);
		}
	}

	return true;
}
/*
bool DcConfTopology::insert(QString path_file, DcConfItem *pitem){
	
	QString path = QDir(path_file).filePath("topology.cfg");
	QFile File(path);
	File.open(QIODevice::ReadOnly | QIODevice::Text);

	QJsonParseError JsonParseError;
	QJsonDocument JsonDocument = QJsonDocument::fromJson(File.readAll(), &JsonParseError);

	File.close();

	QJsonObject RootObject = JsonDocument.object();
	QJsonArray Array = RootObject.value("tree").toArray();
	int size = Array.size();
	QJsonObject ElementObject;
	ElementObject.insert("id", QJsonValue(pitem->id()));
	ElementObject.insert("pid", QJsonValue(pitem->parent_id()));
	ElementObject.insert("type", QJsonValue(pitem->type()));
	ElementObject.insert("name", QJsonValue(pitem->name()));
	ElementObject.insert("desc", QJsonValue(pitem->properties()));

	Array.insert(size, QJsonValue(ElementObject));
	RootObject.insert("tree", Array);
	JsonDocument.setObject(RootObject);

	File.open(QFile::WriteOnly);
	File.write(JsonDocument.toJson());
	File.close();

	return true;
}
*/
bool DcConfTopology::update()
{
	QString path = cfg::path::getTopology();	
	QFile jsonFile(path);
	if (!jsonFile.open(QIODevice::WriteOnly))
		return false;

	QJsonObject rootobj;
	QJsonArray topologyArray;

	for (const auto& item : m_topology)
	{
		QJsonObject ElementObject;
		ElementObject.insert(element_id, QJsonValue(item->id()));
		ElementObject.insert(element_pid, QJsonValue(item->parent_id()));
		ElementObject.insert(element_type, QJsonValue(item->type()));
		ElementObject.insert(element_name, QJsonValue(item->name()));
		//ElementObject.insert(element_desc, QJsonValue(item->properties()));
		topologyArray.append(ElementObject);
	}

	rootobj[root_tree] = topologyArray;
	QJsonDocument doc(rootobj);


	jsonFile.write(doc.toJson());
	jsonFile.close();

	return true;
}

bool DcConfTopology::toJson(void)
{
	QString path = cfg::path::getTopology();
	QFile File(path);

	if (!File.open(QIODevice::WriteOnly))
		return false;

	QJsonDocument JsonDocument;

	QJsonObject RootObject = JsonDocument.object();
	QJsonArray Array = RootObject.value(root_tree).toArray();

	for (DcConfItem *pitem : m_topology) {
		QJsonObject ElementObject;
		ElementObject.insert(element_id, QJsonValue(pitem->id()));
		ElementObject.insert(element_pid, QJsonValue(pitem->parent_id()));
		ElementObject.insert(element_type, QJsonValue(pitem->type()));
		ElementObject.insert(element_name, QJsonValue(pitem->name()));
		//ElementObject.insert(element_desc, QJsonValue(pitem->properties()));
		Array.append(ElementObject);
	}
	RootObject.insert(root_tree, Array);
	JsonDocument.setObject(RootObject);
	File.write(JsonDocument.toJson());
	File.close();
	return true;
}
