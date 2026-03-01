#include "DcParamCfgPool_v2.h"



//template<class T>
//bool DcParamCfgPool_v2<T>::parameterExistWithSameProperties(T elem)
//{
//	for(T param: datavect) {
//		if (param->addr() == elem->addr() && param->param() == elem->param()) {
//			if (elem1->name() != elem2->name()) return false;
//			if (elem1->type() != elem2->type()) return false;
//			if (elem1->flags() != elem2->flags()) return false;
//
//			if (elem1->getProfileCount() != elem2->getProfileCount()) return false;
//			if (elem1->getSubProfileCount() != elem2->getSubProfileCount()) return false;
//			if (elem1->getDataLenInBytes() != elem2->getDataLenInBytes()) return false;
//
//			return true;
//		}
//	}
//
//	return false;
//}