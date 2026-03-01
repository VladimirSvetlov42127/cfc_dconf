#pragma once
#include "dc_conf_item.h"
#include <vector>

class DcConfTopology {
public:
	DcConfTopology();
	~DcConfTopology();

private:
	std::vector<DcConfItem*> m_topology;

public:
	bool add(DcConfItem *pitem);
	void remove(uint32_t position);
	void removeById(int32_t index);
	void clear(void);
	DcConfItem *get(uint32_t position);
	DcConfItem *getById(int32_t index);
	DcConfItem *root(void);
	std::list<DcConfItem*> childs(int32_t index);
	uint32_t size(void);
	bool exist(int32_t index);
	int32_t maxIndex(void);
	//bool insert(QString path_file, DcConfItem *pitem);
	bool update();
	bool fromJson(void);
	bool toJson(void);
};