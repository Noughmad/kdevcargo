/*
 * This file is part of the Cargo plugin for KDevelop.
 *
 * Copyright 2017 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cargoplugin.h"

#include <KPluginFactory>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KShell>
#include <QDebug>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include "cargobuildjob.h"
#include "cargoexecutionconfig.h"

using KDevelop::ProjectTargetItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBuildFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFileItem;
using KDevelop::IPlugin;
using KDevelop::ICore;
using KDevelop::IOutputView;
using KDevelop::IProjectFileManager;
using KDevelop::IProjectBuilder;
using KDevelop::IProject;
using KDevelop::Path;

K_PLUGIN_FACTORY_WITH_JSON(CargoFactory, "kdevcargo.json", registerPlugin<CargoPlugin>(); )

CargoPlugin::CargoPlugin( QObject *parent, const QVariantList & )
    : AbstractFileManagerPlugin( QStringLiteral("kdevcargo"), parent )
{
    m_configType = new CargoExecutionConfigType();
    m_configType->addLauncher( new CargoLauncher( this ) );
    core()->runController()->addConfigurationType( m_configType );
}

CargoPlugin::~CargoPlugin()
{
}

void CargoPlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = nullptr;
}

bool CargoPlugin::addFilesToTarget( const QList<ProjectFileItem*>&, ProjectTargetItem* )
{
    return false;
}

bool CargoPlugin::hasBuildInfo( ProjectBaseItem* ) const
{
    return false;
}

KJob* CargoPlugin::build( ProjectBaseItem* dom )
{
    return new CargoBuildJob( this, dom, QStringLiteral("build") );
}

Path CargoPlugin::buildDirectory( ProjectBaseItem*  item ) const
{
    return item->project()->path();
}

IProjectBuilder* CargoPlugin::builder() const
{
    return const_cast<IProjectBuilder*>(dynamic_cast<const IProjectBuilder*>(this));
}

KJob* CargoPlugin::clean( ProjectBaseItem* dom )
{
    return new CargoBuildJob( this, dom, QStringLiteral("clean") );
}

KJob* CargoPlugin::configure( IProject* project )
{
    return new CargoBuildJob( this, project->projectItem(), QString() );
}

ProjectTargetItem* CargoPlugin::createTarget( const QString&, ProjectFolderItem* )
{
    return nullptr;
}

QHash<QString, QString> CargoPlugin::defines( ProjectBaseItem* ) const
{
    return {};
}

IProjectFileManager::Features CargoPlugin::features() const
{
    return IProjectFileManager::Files | IProjectFileManager::Folders;
}

ProjectFolderItem* CargoPlugin::createFolderItem( IProject* project,
                    const Path& path, ProjectBaseItem* parent )
{
    return new ProjectBuildFolderItem( project, path, parent );
}

Path::List CargoPlugin::includeDirectories( ProjectBaseItem* ) const
{
    return {};
}

Path::List CargoPlugin::frameworkDirectories( ProjectBaseItem* ) const
{
    return {};
}

KJob* CargoPlugin::install( KDevelop::ProjectBaseItem* item, const QUrl &installPrefix )
{
    auto job = new CargoBuildJob( this, item, QStringLiteral("install") );
    job->setInstallPrefix(installPrefix);
    return job;
}

KJob* CargoPlugin::prune( IProject* project )
{
    return clean(project->projectItem());
}

bool CargoPlugin::removeFilesFromTargets( const QList<ProjectFileItem*>& )
{
    return false;
}

bool CargoPlugin::removeTarget( ProjectTargetItem* )
{
    return false;
}

QList<ProjectTargetItem*> CargoPlugin::targets( ProjectFolderItem* ) const
{
    return QList<ProjectTargetItem*>();
}

int CargoPlugin::perProjectConfigPages() const
{
    return 0;
}

KDevelop::ConfigPage* CargoPlugin::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    Q_UNUSED(number);
    Q_UNUSED(options);
    Q_UNUSED(parent);
    return nullptr;
}

QUrl CargoPlugin::executable(KDevelop::ILaunchConfiguration* config, QString& /*error*/) const
{
    Q_UNUSED(config);
    return QUrl::fromLocalFile(QStandardPaths::findExecutable("cargo"));
}

QStringList CargoPlugin::arguments(KDevelop::ILaunchConfiguration* config, QString& /*error*/) const
{
    QStringList ret = {QStringLiteral("run")};
    qWarning() << "Config:" << config->config().entryMap();
    QString id = config->config().readEntry( "CargoIdentifier" );
    if (!id.isEmpty())
    {
        ret << QStringLiteral("--bin") << id;
    }
    QString arguments = config->config().readEntry( "CargoArguments" );
    if (!arguments.isEmpty())
    {
        ret << QStringLiteral("--") << KShell::splitArgs(arguments);
    }
    return ret;
}

KJob* CargoPlugin::dependencyJob(KDevelop::ILaunchConfiguration* config) const
{
    Q_UNUSED(config);
    return nullptr;
}

QUrl CargoPlugin::workingDirectory(KDevelop::ILaunchConfiguration* config) const
{
    return config->project()->path().toUrl();
}

#if KDEVPLATFORM_VERSION >= ((5<<16)|(2<<8)|(0))
QString CargoPlugin::environmentProfileName(KDevelop::ILaunchConfiguration* config) const
#else
QString CargoPlugin::environmentGroup(KDevelop::ILaunchConfiguration* config) const
#endif
{
    Q_UNUSED(config);
    return QString();
}

QString CargoPlugin::nativeAppConfigTypeId() const
{
    return CargoExecutionConfigType::typeId();
}

bool CargoPlugin::useTerminal(KDevelop::ILaunchConfiguration* config) const
{
    Q_UNUSED(config);
    return false;
}

QString CargoPlugin::terminal(KDevelop::ILaunchConfiguration* config) const
{
    Q_UNUSED(config);
    return QString();
}

#if KDEVPLATFORM_VERSION >= VERSION_5_2
QString CargoPlugin::extraArguments(KDevelop::ProjectBaseItem* item) const
{
    Q_UNUSED(item);
    return QString();
}
#endif

#include "cargoplugin.moc"
