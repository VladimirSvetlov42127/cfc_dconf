#ifndef __FUNC_SERVICE_OUTPUT_H__
#define __FUNC_SERVICE_OUTPUT_H__

#include "service_manager/services/service_output.h"


class FuncServiceOutput : public ServiceOutput
{
public:
    FuncServiceOutput(const QString &name, DcParam * bind_param, Service *service);
    virtual ~FuncServiceOutput() = default;

    QString name() const override { return _name; };

protected:
    void updateBinding(VirtualInputSignal* new_target) override;

private:
    QString _name;
};

#endif // __FUNC_SERVICE_OUTPUT_H__
