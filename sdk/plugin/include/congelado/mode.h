#pragma once

#if defined(CONGELADO_GUEST) && defined(CONGELADO_HOST)
#  error "Define exactly one of CONGELADO_GUEST or CONGELADO_HOST — not both"
#endif
#if !defined(CONGELADO_GUEST) && !defined(CONGELADO_HOST)
#  define CONGELADO_GUEST
#endif
