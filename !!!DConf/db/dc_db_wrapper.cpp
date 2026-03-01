#include <db/dc_db_wrapper.h>

DcDbWrapper::DcDbWrapper(int32_t controllerid)
    : m_uid(controllerid)
{
}

int32_t DcDbWrapper::uid() const
{
	return m_uid;
}
