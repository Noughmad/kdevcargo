/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

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

K_PLUGIN_FACTORY_WITH_JSON(CargoFactory, "kdevcargo.json", registerPlugin<Cargo>(); )

Cargo::Cargo( QObject *parent, const QVariantList & )
    : AbstractFileManagerPlugin( QStringLiteral("kdevcargo"), parent )
{
    m_configType = new CargoExecutionConfigType();
    m_configType->addLauncher( new CargoLauncher( this ) );
    core()->runController()->addConfigurationType( m_configType );
}

Cargo::~Cargo()
{
}

void Cargo::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = nullptr;
}

bool Cargo::addFilesToTarget( const QList<ProjectFileItem*>&, ProjectTargetItem* )
{
    return false;
}

bool Cargo::hasBuildInfo( ProjectBaseItem* ) const
{
    return false;
}

KJob* Cargo::build( ProjectBaseItem* dom )
{
    return new CargoBuildJob( this, dom, QStringLiteral("build") );
}

Path Cargo::buildDirectory( ProjectBaseItem*  item ) const
{
    return item->project()->path();
}

IProjectBuilder* Cargo::builder() const
{
    return const_cast<IProjectBuilder*>(dynamic_cast<const IProjectBuilder*>(this));
}

KJob* Cargo::clean( ProjectBaseItem* dom )
{
    return new CargoBuildJob( this, dom, QStringLiteral("clean") );
}

KJob* Cargo::configure( IProject* project )
{
    return new CargoBuildJob( this, project->projectItem(), QString() );
}

ProjectTargetItem* Cargo::createTarget( const QString&, ProjectFolderItem* )
{
    return nullptr;
}

QHash<QString, QString> Cargo::defines( ProjectBaseItem* ) const
{
    return {};
}

IProjectFileManager::Features Cargo::features() const
{
    return IProjectFileManager::Files | IProjectFileManager::Folders;
}

ProjectFolderItem* Cargo::createFolderItem( IProject* project,
                    const Path& path, ProjectBaseItem* parent )
{
    return new ProjectBuildFolderItem( project, path, parent );
}

Path::List Cargo::includeDirectories( ProjectBaseItem* ) const
{
    return {};
}

Path::List Cargo::frameworkDirectories( ProjectBaseItem* ) const
{
    return {};
}

KJob* Cargo::install( KDevelop::ProjectBaseItem* item, const QUrl &installPrefix )
{
    auto job = new CargoBuildJob( this, item, QStringLiteral("install") );
    job->setInstallPrefix(installPrefix);
    return job;
}

KJob* Cargo::prune( IProject* project )
{
    return clean(project->projectItem());
}

bool Cargo::removeFilesFromTargets( const QList<ProjectFileItem*>& )
{
    return false;
}

bool Cargo::removeTarget( ProjectTargetItem* )
{
    return false;
}

QList<ProjectTargetItem*> Cargo::targets( ProjectFolderItem* ) const
{
    return QList<ProjectTargetItem*>();
}

KConfigGroup Cargo::configuration( IProject* project ) const
{
    Q_UNUSED(project);
    return KConfigGroup();
}

int Cargo::perProjectConfigPages() const
{
    return 0;
}

KDevelop::ConfigPage* Cargo::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    Q_UNUSED(number);
    Q_UNUSED(options);
    Q_UNUSED(parent);
    return nullptr;
}

QUrl Cargo::executable(KDevelop::ILaunchConfiguration* config, QString& /*error*/) const
{
    Q_UNUSED(config);
    return QUrl::fromLocalFile(QStandardPaths::findExecutable("cargo"));
}

QStringList Cargo::arguments(KDevelop::ILaunchConfiguration* config, QString& /*error*/) const
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

KJob* Cargo::dependencyJob(KDevelop::ILaunchConfiguration* config) const
{
    Q_UNUSED(config);
    return nullptr;
}

QUrl Cargo::workingDirectory(KDevelop::ILaunchConfiguration* config) const
{
    return config->project()->path().toUrl();
}

QString Cargo::environmentGroup(KDevelop::ILaunchConfiguration* /*config*/) const
{
    return QString();
}

QString Cargo::nativeAppConfigTypeId() const
{
    return CargoExecutionConfigType::typeId();
}

bool Cargo::useTerminal(KDevelop::ILaunchConfiguration* /*config*/) const
{
    return false;
}

QString Cargo::terminal(KDevelop::ILaunchConfiguration* /*config*/) const
{
    return QString();
}

#include "cargoplugin.moc"
