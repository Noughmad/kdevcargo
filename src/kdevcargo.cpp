#include "kdevcargo.h"

#include <QDebug>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(kdevcargoFactory, "kdevcargo.json", registerPlugin<kdevcargo>(); )

kdevcargo::kdevcargo(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevcargo"), parent)
{
    Q_UNUSED(args);

    qDebug() << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "kdevcargo.moc"
