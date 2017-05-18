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

#ifndef CARGOEXECUTIONCONFIG_H
#define CARGOEXECUTIONCONFIG_H

#include <interfaces/launchconfigurationtype.h>
#include <interfaces/launchconfigurationpage.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>

#include "ui_cargoexecutionconfig.h"

class CargoPlugin;

class CargoExecutionConfig : public KDevelop::LaunchConfigurationPage, Ui::CargoExecutionPage
{
Q_OBJECT
public:
    CargoExecutionConfig( QWidget* parent );
    void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = nullptr ) override;
    void saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project = nullptr ) const override;
    QString title() const override;
    QIcon icon() const override;
};

class CargoLauncher : public KDevelop::ILauncher
{
public:
    CargoLauncher( CargoPlugin* plugin );
    QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const override;
    QString description() const override;
    QString id() override;
    QString name() const override;
    KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    virtual KJob* dependencies(KDevelop::ILaunchConfiguration* cfg);
    QStringList supportedModes() const override;
    
    static KJob* calculateDependencies(KDevelop::ILaunchConfiguration* cfg);
private:
    CargoPlugin* m_plugin;
};

class CargoPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    CargoPageFactory();
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configured informations, as well as its type and a name
 */
class CargoExecutionConfigType : public KDevelop::LaunchConfigurationType
{
Q_OBJECT
public:
    CargoExecutionConfigType();
    ~CargoExecutionConfigType() override;

    static QString typeId();
    QString id() const override { return typeId(); }
    QString name() const override;
    QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const override;  
    QIcon icon() const override;
    bool canLaunch( const QUrl &file ) const override;
    bool canLaunch(KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromCmdLineArguments(KConfigGroup config, const QStringList& args) const override;
    
    QMenu* launcherSuggestions() override;

private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;

public slots:
    void suggestionTriggered();
};
#endif

