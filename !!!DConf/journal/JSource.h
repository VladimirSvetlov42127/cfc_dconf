#pragma once

#include <dpc/journal/Journal.h>

#include <data_model/dc_controller.h>

class JSource : public Dpc::Journal::ISource
{
public:
	JSource(DcController* device) :
		m_device(device)
	{
		connect(device, &DcController::nameChanged, this, &Dpc::Journal::ISource::nameChanged);
	}

	virtual IndexType id() const override { return qHash(m_device); }
	virtual QString name() const override { return m_device->name(); }

	static Dpc::Journal::ISourcePtr make(DcController* device) { return std::make_shared<JSource>(device); }

private:
	DcController* m_device;
};
