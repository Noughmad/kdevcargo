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

#ifndef CARGOPLUGIN_H
#define CARGOPLUGIN_H

#include <interfaces/iplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/abstractfilemanagerplugin.h>
#include <execute/iexecuteplugin.h>
#include <kdevplatform_version.h>

#define VERSION_5_2 ((5<<16)|(2<<8)|(0))

class KConfigGroup;
class KDialogBase;
class CargoExecutionConfigType;

namespace KDevelop
{
class ProjectBaseItem;
class IProject;
}

class CargoPlugin : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IProjectBuilder, public KDevelop::IBuildSystemManager, public IExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )
    Q_INTERFACES( KDevelop::IProjectFileManager )
    Q_INTERFACES( KDevelop::IBuildSystemManager )
    Q_INTERFACES( IExecutePlugin )
public:
    explicit CargoPlugin( QObject *parent = nullptr, const QVariantList &args = QVariantList() );
    virtual ~CargoPlugin();

// ProjectBuilder API
    KJob* build( KDevelop::ProjectBaseItem* dom ) override;
    KJob* clean( KDevelop::ProjectBaseItem* dom ) override;
    KJob* prune( KDevelop::IProject* ) override;

    /// @p installPrefix will be passed as DESTDIR environment variable
    KJob* install( KDevelop::ProjectBaseItem* item, const QUrl &installPrefix ) override;
    KJob* configure( KDevelop::IProject* ) override;
signals:
    void built( KDevelop::ProjectBaseItem *dom );
    void installed( KDevelop::ProjectBaseItem* );
    void cleaned( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem *dom );
    void configured( KDevelop::IProject* );
    void pruned( KDevelop::IProject* );

// AbstractFileManagerPlugin API
public:
    Features features() const override;
    virtual KDevelop::ProjectFolderItem* createFolderItem( KDevelop::IProject* project, 
                    const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent = nullptr ) override;

// BuildSystemManager API
public:
    bool addFilesToTarget( const QList<KDevelop::ProjectFileItem*>& file, KDevelop::ProjectTargetItem* parent ) override;
    bool hasBuildInfo( KDevelop::ProjectBaseItem* ) const override;
    KDevelop::Path buildDirectory( KDevelop::ProjectBaseItem* ) const override;
    IProjectBuilder* builder() const override;
    KDevelop::ProjectTargetItem* createTarget( const QString& target, KDevelop::ProjectFolderItem* parent ) override;
    QHash<QString, QString> defines( KDevelop::ProjectBaseItem* ) const override;
    KDevelop::Path::List includeDirectories( KDevelop::ProjectBaseItem* ) const override;
    KDevelop::Path::List frameworkDirectories( KDevelop::ProjectBaseItem* ) const override;
    bool removeFilesFromTargets( const QList<KDevelop::ProjectFileItem*>& ) override;
    bool removeTarget( KDevelop::ProjectTargetItem* target ) override;
    QList<KDevelop::ProjectTargetItem*> targets( KDevelop::ProjectFolderItem* ) const override;

    virtual KDevelop::Path compiler(KDevelop::ProjectTargetItem* p) const final { return {}; }

#if KDEVPLATFORM_VERSION >= VERSION_5_2
    QString extraArguments(KDevelop::ProjectBaseItem* item) const override;
#endif

// IPlugin API
public:
    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

// IExecutePlugin API
    QUrl executable(KDevelop::ILaunchConfiguration* config, QString& error) const override;
    QStringList arguments(KDevelop::ILaunchConfiguration* config, QString& error) const override;
    KJob* dependencyJob(KDevelop::ILaunchConfiguration* config) const override;

#if KDEVPLATFORM_VERSION >= VERSION_5_2
    QString environmentProfileName(KDevelop::ILaunchConfiguration* config) const override;
#else
    QString environmentGroup(KDevelop::ILaunchConfiguration* config) const override;
#endif

    QString nativeAppConfigTypeId() const override;
    QString terminal(KDevelop::ILaunchConfiguration* config) const override;
    bool useTerminal(KDevelop::ILaunchConfiguration* config) const override;
    QUrl workingDirectory(KDevelop::ILaunchConfiguration* config) const override;

// IPlugin API
    void unload() override;

private:
    CargoExecutionConfigType* m_configType;
};

#endif
