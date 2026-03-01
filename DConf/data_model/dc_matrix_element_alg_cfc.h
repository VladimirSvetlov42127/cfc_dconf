#pragma once
#include "dc_matrix_element.h"
#include <db/dc_db_wrapper.h>

class DcMatrixElementAlgCfc : public DcMatrixElement, public DcDbWrapper {
public:

    DcMatrixElementAlgCfc(int32_t controllerid, int32_t src, int32_t dst, const QString &properties);

	void updatedb();

	DefMatrixElementType type(void) { return DEF_MATRIX_TYPE_ALG_CFC; }

	void insert();
	void update();
	void remove();
};
