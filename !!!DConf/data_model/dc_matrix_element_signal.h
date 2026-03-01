#pragma once
#include "dc_matrix_element.h"
#include <db/dc_db_wrapper.h>

class DcMatrixElementSignal : public DcMatrixElement, public DcDbWrapper {
public:

    DcMatrixElementSignal(int32_t controllerid, int32_t src, int32_t dst, const QString &properties);

	void updatedb();

	DefMatrixElementType type(void) { return DEF_MATRIX_TYPE_SIGNAL; }

	void insert();
	void update();
	void remove();
};
