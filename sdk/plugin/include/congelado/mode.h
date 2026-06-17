#pragma once

#if !defined(CONGELADO_GUEST) && !defined(CONGELADO_HOST)
#  error "Define either CONGELADO_GUEST or CONGELADO_HOST before including congelado headers"
#endif
#if defined(CONGELADO_GUEST) && defined(CONGELADO_HOST)
#  error "Define exactly one of CONGELADO_GUEST or CONGELADO_HOST — not both"
#endif
