#pragma once
#include <qstring.h>

class DcDbWrapper {
public:
    DcDbWrapper(int32_t controllerid);
    int32_t uid() const ;

	virtual void insert() = 0;
	virtual void update() = 0;
    virtual void remove() = 0;

private:
	int32_t m_uid;
};


