#ifndef KDEVCARGO_H
#define KDEVCARGO_H

#include <interfaces/iplugin.h>

class kdevcargo : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    kdevcargo(QObject* parent, const QVariantList& args);
};

#endif // KDEVCARGO_H
