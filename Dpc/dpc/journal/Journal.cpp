#include "Journal.h"

#include <qdebug.h>
#include <qtimer.h>

namespace Dpc::Journal
{
	SignletonJournal::SignletonJournal()
	{
		qRegisterMetaType<Dpc::Journal::ISourcePtr>();
		qRegisterMetaType<Dpc::Journal::RecordList>();

		auto timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, [=]() {
			if (m_recordList.isEmpty())
			return;

		emit newRecords(m_recordList);
		m_recordList.clear();
			});

		timer->start(75);
	}

	SignletonJournal& SignletonJournal::instance()
	{
		static SignletonJournal j;
		return j;
	}

	void SignletonJournal::addInfoMessage(const QString& msg, const ISourcePtr& source)
	{
		m_recordList << Record{ source, Record::Information, QDateTime::currentDateTime(), msg };
	}

	void SignletonJournal::addDebugMessage(const QString& msg, uint8_t level, const ISourcePtr& source)
	{
		m_recordList << Record{ source, Record::Debug, QDateTime::currentDateTime(), QString("%1%2").arg(QString(level, '\t')).arg(msg) };
	}

	void SignletonJournal::addWarningMessage(const QString& msg, const ISourcePtr& source)
	{
		m_recordList << Record{ source, Record::Warning, QDateTime::currentDateTime(), msg };
	}

	void SignletonJournal::addErrorMessage(const QString& msg, const ISourcePtr& source)
	{
		m_recordList << Record{ source, Record::Error, QDateTime::currentDateTime(), msg };
	}

	void SignletonJournal::addSource(const ISourcePtr& source)
	{
		emit sourceAdded(source);
	}

	void SignletonJournal::removeSource(const ISourcePtr& source)
	{
		emit sourceRemoved(source);
	}

	void SignletonJournal::select(const ISourcePtr& source)
	{
		emit sourceSelected(source);
	}
} // namespace