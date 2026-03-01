#pragma once
#include "dc_param_cfg_pool.h"

template< class T >
class DcParamCfgPool_v2 : 	public DcParamCfgPool<T>
{
public:
	DcParamCfgPool_v2() {};
	virtual ~DcParamCfgPool_v2() {};
	
	//bool parameterExistWithSameProperties(T elem);

private:
	//T val;
};