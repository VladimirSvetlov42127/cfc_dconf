#include "dc_matrix_element_signal.h"
#include <db/dc_db_manager.h>

DcMatrixElementSignal::DcMatrixElementSignal(int32_t controllerid, int32_t src, int32_t dst, const QString &properties)
    : DcMatrixElement(src, dst, properties)
    , DcDbWrapper(controllerid)
{
}

void DcMatrixElementSignal::updatedb()
{
	update();
}

void DcMatrixElementSignal::insert()
{
	QString insertStrTmp = "INSERT INTO matrix_signals(signal_id_source, signal_id_destination) VALUES(%1, %2);";

	QString insertStr = insertStrTmp.arg(QString::number(src()), QString::number(dst()));
	gDbManager.execute(uid(), insertStr);
}

void DcMatrixElementSignal::update()
{

				/*
				QString updateStrTmp = "UPDATE matrix_signals SET properties = '%1' WHERE signal_id_source = %2 and signal_id_destination = %3;";

				QString updateStr = updateStrTmp.arg("", QString::number(src()), QString::number(dst()));
				gDbManager.execute(uid(), updateStr);
				*/ //TODO properties not used yet in matrix
}

void DcMatrixElementSignal::remove()
{
	QString removeStrTmp = "DELETE FROM matrix_signals WHERE signal_id_source = %1 and signal_id_destination=%2;";
	QString removeStr = removeStrTmp.arg(QString::number(src()), QString::number(dst()));
	gDbManager.execute(uid(), removeStr);
}
