#pragma once

#include <data_model/storage_for_controller_params/DcController_v2.h>

bool resetUnusedSlaves(DcController *controller, uint16_t portNumber, uint16_t activeSlaveCount);	// Выключаем все неиспользуемые слейвы

bool cleanSlaveConfig(DcController * controller, uint16_t portId, uint16_t slaveId);