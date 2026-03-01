#pragma once

#include <data_model/dc_signal.h>

bool hasTag(const QString &initName);

void removeTagFromName(DcSignal * signal, const QString & nameToRemove);

QString addInputTag(const QString &tag, const QString &initName);
QString addOutputTag(const QString &tag, const QString &initName);

QString removeAlgInput(const QString &algInputName, const QString &initName);	// Удаляем привязку к выходу алгоритма