#ifndef GLOBAL_H
#define GLOBAL_H
#include <QtCore/qglobal.h>

#if defined(QHEATMAP_DLL_LIBRARY)
#  define QHEATMAP_DLL_EXPORT Q_DECL_EXPORT
#else
#  define QHEATMAP_DLL_EXPORT
#endif
#endif // GLOBAL_H
