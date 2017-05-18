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

#ifndef CUSTOMBUILDSYSTEMPLUGIN_H
#define CUSTOMBUILDSYSTEMPLUGIN_H

#include <interfaces/iplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/abstractfilemanagerplugin.h>
#include <execute/iexecuteplugin.h>

class KConfigGroup;
class KDialogBase;
class CargoExecutionConfigType;

namespace KDevelop
{
class ProjectBaseItem;
class IProject;
}

class Cargo : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IProjectBuilder, public KDevelop::IBuildSystemManager, public IExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )
    Q_INTERFACES( KDevelop::IProjectFileManager )
    Q_INTERFACES( KDevelop::IBuildSystemManager )
    Q_INTERFACES( IExecutePlugin )
public:
    explicit Cargo( QObject *parent = nullptr, const QVariantList &args = QVariantList() );
    virtual ~Cargo();

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
    KConfigGroup configuration( KDevelop::IProject* ) const;
    KConfigGroup findMatchingPathGroup( const KConfigGroup& cfg, KDevelop::ProjectBaseItem* item ) const;

// IPlugin API
public:
    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

// IExecutePlugin API
    QUrl executable(KDevelop::ILaunchConfiguration* config, QString& error) const override;
    QStringList arguments(KDevelop::ILaunchConfiguration* config, QString& error) const override;
    KJob* dependencyJob(KDevelop::ILaunchConfiguration* config) const override;
    QString environmentGroup(KDevelop::ILaunchConfiguration* config) const override;
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
