// Minimal backend for intern/tf_tstring.h — previously declarations-only. ice::String
// (cc/abi/intern/string.cppm) wraps TF_TString and is what the TF_Generator_* cc wrapper
// (ice::GeneratorController/GeneratorSourceCode) uses for its string parameters, so this is
// a genuine prerequisite for tools/stable_hlo_generator to link, not an unrelated add-on.
//
// Deliberately skips TF's real small-string-optimization bit-packing (SMALL/OFFSET
// variants) — nothing in this repo depends on TF_TString's raw byte layout being
// upstream-compatible, only on the documented Init/Copy/AssignView/GetDataPointer/GetType/
// GetSize/GetCapacity/Dealloc contract, which this satisfies using only the union's `large`
// member: owned (copied) strings get capacity > 0, non-owning views get capacity == 0.

#include "c/intern/tf_tstring.h"

#include <cstdlib>
#include <cstring>

extern "C" {

void TF_StringInit(TF_TString *t) {
    if (!t) {
        return;
    }
    t->large.size = 0;
    t->large.capacity = 0;
    t->large.data = nullptr;
}

void TF_StringCopy(TF_TString *dst, const char *src, size_t size) {
    if (!dst) {
        return;
    }
    if (dst->large.capacity > 0 && dst->large.data) {
        std::free(dst->large.data);
    }
    dst->large.size = size;
    dst->large.data = static_cast<char *>(std::malloc(size + 1));
    if (!dst->large.data) {
        dst->large.capacity = 0;
        dst->large.size = 0;
        return;
    }
    dst->large.capacity = size + 1;
    if (src && size > 0) {
        std::memcpy(dst->large.data, src, size);
    }
    dst->large.data[size] = '\0';
}

void TF_StringAssignView(TF_TString *dst, const char *src, size_t size) {
    if (!dst) {
        return;
    }
    if (dst->large.capacity > 0 && dst->large.data) {
        std::free(dst->large.data);
    }
    dst->large.size = size;
    dst->large.capacity = 0;
    dst->large.data = const_cast<char *>(src);
}

const char *TF_StringGetDataPointer(const TF_TString *tstr) { return tstr ? tstr->large.data : nullptr; }

TF_TString_Type TF_StringGetType(const TF_TString *str) {
    if (!str) {
        return TF_TSTR_VIEW;
    }
    return str->large.capacity > 0 ? TF_TSTR_LARGE : TF_TSTR_VIEW;
}

size_t TF_StringGetSize(const TF_TString *tstr) { return tstr ? tstr->large.size : 0; }

size_t TF_StringGetCapacity(const TF_TString *str) { return str ? str->large.capacity : 0; }

void TF_StringDealloc(TF_TString *tstr) {
    if (!tstr) {
        return;
    }
    if (tstr->large.capacity > 0 && tstr->large.data) {
        std::free(tstr->large.data);
    }
    tstr->large.size = 0;
    tstr->large.capacity = 0;
    tstr->large.data = nullptr;
}

} // extern "C"
