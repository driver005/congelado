#include "include/c/extern/plugin/registration.h"
#include "include/c/intern/array.h"
#include "include/c/intern/attrtype.h"
#include "include/c/intern/bitset.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/complex.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/deque.h"
#include "include/c/intern/duration.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/forward_list/forward_list.h"
#include "include/c/intern/hash.h"
#include "include/c/intern/hive/hive.h"
#include "include/c/intern/list/list.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/set.h"
#include "include/c/intern/shape.h"
#include "include/c/intern/span.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/time_point.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"
#include "include/c/macros.h"

#define ATEN_XPU_DOMAIN aten_xpu::SyclPluginDomain::k_intern
#include "docs/aten_xpu/plugin_entry.h"

import std;
import aten_xpu;

extern "C"
{
    ATEN_XPU_PLUGIN_ENTRY("aten_xpu.intern")

    ATEN_XPU_EXPORT_OPS(tensor, TF_TensorOps, registry.getOps().getTensorOps())
    ATEN_XPU_EXPORT_WEAK_OPS(string, TF_StringOps, &registry.getStringOps())
    ATEN_XPU_EXPORT_WEAK_OPS(status, TF_StatusOps, &registry.getStatusOps())
    ATEN_XPU_EXPORT_WEAK_OPS(buffer, TF_BufferOps, &registry.getBufferOps())
    ATEN_XPU_EXPORT_OPS(registration, TF_RegistrationOps, &registry.getRegistrationOps())
    ATEN_XPU_EXPORT_OPS(complex, TF_ComplexOps, &registry.getComplexOps())
    ATEN_XPU_EXPORT_OPS(duration, TF_DurationOps, &registry.getDurationOps())
    ATEN_XPU_EXPORT_OPS(time_point, TF_TimePointOps, &registry.getTimePointOps())
    ATEN_XPU_EXPORT_OPS(datatype, TF_DataTypeOps, &registry.getDataTypeOps())
    ATEN_XPU_EXPORT_OPS(attrtype, TF_AttrTypeOps, &registry.getAttrTypeOps())
    ATEN_XPU_EXPORT_OPS(shape, TF_ShapeOps, &registry.getShapeOps())
    ATEN_XPU_EXPORT_OPS(file_statistics, TF_FileStatisticsOps, &registry.getFileStatisticsOps())
    ATEN_XPU_EXPORT_OPS(hash, TF_HashOps, &registry.getHashOps())
    ATEN_XPU_EXPORT_OPS(array, TF_ArrayOps, &registry.getArrayOps())
    ATEN_XPU_EXPORT_OPS(vector, TF_VectorOps, &registry.getVectorOps())
    ATEN_XPU_EXPORT_OPS(deque, TF_DequeOps, &registry.getDequeOps())
    ATEN_XPU_EXPORT_OPS(list, TF_ListOps, &registry.getListOps())
    ATEN_XPU_EXPORT_OPS(forward_list, TF_ForwardListOps, &registry.getForwardListOps())
    ATEN_XPU_EXPORT_OPS(hive, TF_HiveOps, &registry.getHiveOps())
    ATEN_XPU_EXPORT_OPS(bitset, TF_BitSetOps, &registry.getBitSetOps())
    ATEN_XPU_EXPORT_OPS(span, TF_SpanOps, &registry.getSpanOps())
    ATEN_XPU_EXPORT_OPS(map, TF_MapOps, &registry.getMapOps())
    ATEN_XPU_EXPORT_OPS(set, TF_SetOps, &registry.getSetOps())
}
