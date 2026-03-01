#pragma once

#include <memory>
#include <qobject.h>

#include <dpc/dpc_global.h>

namespace Dpc::Journal
{
	class DPC_EXPORT ISource : public QObject
	{
		Q_OBJECT
	public:
		using IndexType = uint32_t;

		virtual IndexType id() const = 0;
		virtual QString name() const = 0;

	signals:
		void nameChanged(const QString &name);
	};

	using ISourcePtr = std::shared_ptr<ISource>;
}
Q_DECLARE_METATYPE(Dpc::Journal::ISourcePtr)
