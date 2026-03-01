#pragma once

#include <qdatetime.h>

#include <dpc/dpc_global.h>
#include <dpc/journal/Source.h>

namespace Dpc::Journal
{
	struct  Record
	{
		enum Type {
			Unknown = 0,
			Information = 1,
			Warning = 2,
			Error = 4,
			Debug = 8
		};

		ISourcePtr source;
		Type type;
		QDateTime dateTime;
		QString msg;

		ISource::IndexType sourceId() const { return source ? source->id() : ISource::IndexType(); }
	};
	using RecordList = QList<Record>;
}
Q_DECLARE_METATYPE(Dpc::Journal::RecordList)