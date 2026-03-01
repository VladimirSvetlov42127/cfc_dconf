#pragma once

#include <dpc/journal/Record.h>

#define gJournal Dpc::Journal::SignletonJournal::instance()

namespace Dpc::Journal 
{	
	class DPC_EXPORT SignletonJournal : public QObject
	{
		Q_OBJECT
	public:
		static SignletonJournal& instance();

	signals:
		void newRecords(const RecordList&);
		void sourceAdded(const ISourcePtr&);
		void sourceRemoved(const ISourcePtr&);
		void sourceSelected(const ISourcePtr&);

	public slots:
		void addInfoMessage(const QString& msg, const ISourcePtr& source = ISourcePtr());
		void addDebugMessage(const QString& msg, uint8_t level = 0, const ISourcePtr& source = ISourcePtr());
		void addWarningMessage(const QString& msg, const ISourcePtr& source = ISourcePtr());
		void addErrorMessage(const QString& msg, const ISourcePtr& source = ISourcePtr());

		void addSource(const ISourcePtr& source = ISourcePtr());
		void removeSource(const ISourcePtr& source = ISourcePtr());
		void select(const ISourcePtr& source = ISourcePtr());

	protected:
		SignletonJournal();

	private:
		SignletonJournal(const SignletonJournal&) = delete;
		SignletonJournal(SignletonJournal&&) = delete;
		SignletonJournal& operator=(const SignletonJournal&) = delete;
		SignletonJournal& operator=(SignletonJournal&&) = delete;

		RecordList m_recordList;
	};
} // namespace