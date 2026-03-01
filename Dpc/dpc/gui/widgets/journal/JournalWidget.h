#pragma once

#include <qwidget.h>

#include <dpc/journal/Journal.h>

namespace Dpc::Gui
{
	class JournalTableModel;
	class JournalFilterModel;

	class TableView;
	class CheckableComboBox;

	class DPC_EXPORT JournalWidget : public QWidget
	{
		Q_OBJECT
	public:
		JournalWidget(QWidget* parent = nullptr);

	public slots:
		void start();
		void stop();

	private slots:
		void onClearButton();
		void onSaveButton();

		void onSourceAdded(const Journal::ISourcePtr& source);
		void onSourceRemoved(const Journal::ISourcePtr& source);
		void onSourceSelected(const Journal::ISourcePtr& source);
		void onDeviceBoxChanged();
		void onRowsInserted();

	private:
		void clearSources(const QList<Journal::ISource::IndexType>& list);
		void moveToSelected();

	private:
		JournalTableModel* m_model;
		JournalFilterModel* m_filterModel;

		TableView* m_view;
		CheckableComboBox* m_sourceBox;

		QHash<Journal::ISource::IndexType, Journal::ISourcePtr> m_sources;
	};
} // namespace