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

#include "cargoexecutionconfig.h"
#include "cargobuildjob.h"
#include "cargoplugin.h"

#include <KLocalizedString>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <serialization/indexedstring.h>
#include <util/kdevstringhandler.h>
#include <util/executecompositejob.h>
#include <util/path.h>

#include <KMessageBox>
#include <KParts/MainWindow>
#include <KConfigGroup>
#include <QMenu>
#include <QLineEdit>
#include <QDebug>
class la;

Q_DECLARE_METATYPE(KDevelop::IProject*);

QIcon CargoExecutionConfig::icon() const
{
    return QIcon::fromTheme("system-run");
}

QStringList readProcess(QProcess* p)
{
    QStringList ret;
    while(!p->atEnd()) {
        QByteArray line = p->readLine();
        int nameEnd=line.indexOf(' ');
        if(nameEnd>0) {
            ret += line.left(nameEnd);
        }
    }
    return ret;
}

CargoExecutionConfig::CargoExecutionConfig( QWidget* parent )
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    connect( identifier->lineEdit(), &QLineEdit::textEdited, this, &CargoExecutionConfig::changed );
}

void CargoExecutionConfig::saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project ) const
{
    Q_UNUSED( project );
    cfg.writeEntry("CargoIdentifier", identifier->lineEdit()->text());
    cfg.writeEntry("CargoArguments", arguments->text());
}

void CargoExecutionConfig::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* )
{
    bool b = blockSignals( true );
    identifier->lineEdit()->setText(cfg.readEntry("CargoIdentifier", ""));
    arguments->setText(cfg.readEntry("CargoArguments", ""));
    blockSignals( b );
}

QString CargoExecutionConfig::title() const
{
    return i18n("Configure Cargo Execution");
}

QList< KDevelop::LaunchConfigurationPageFactory* > CargoLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString CargoLauncher::description() const
{
    return i18n("Runs a Rust executable with Cargo");
}

QString CargoLauncher::id()
{
    return "CargoLauncher";
}

QString CargoLauncher::name() const
{
    return i18n("Cargo Launcher");
}

CargoLauncher::CargoLauncher(CargoPlugin* plugin)
    : m_plugin(plugin)
{
}

KJob* CargoLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return nullptr;
    }

    if( launchMode == "execute" )
    {
        CargoBuildJob* job = new CargoBuildJob(m_plugin, cfg->project()->projectItem(), QStringLiteral("run"));

        QString err;
        QStringList runArguments = m_plugin->arguments(cfg, err);
        runArguments.takeFirst();
        job->setRunArguments(runArguments);

        qWarning() << "Running build job with arguments" << runArguments;

        return job;
    }
    qWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return nullptr;
}

KJob* CargoLauncher::calculateDependencies(KDevelop::ILaunchConfiguration* cfg)
{
    Q_UNUSED(cfg);
    return nullptr;
}

KJob* CargoLauncher::dependencies(KDevelop::ILaunchConfiguration* cfg)
{
    return calculateDependencies(cfg);
}


QStringList CargoLauncher::supportedModes() const
{
    return QStringList() << "execute";
}

KDevelop::LaunchConfigurationPage* CargoPageFactory::createWidget(QWidget* parent)
{
    return new CargoExecutionConfig( parent );
}

CargoPageFactory::CargoPageFactory()
{}

CargoExecutionConfigType::CargoExecutionConfigType()
{
    factoryList.append( new CargoPageFactory );
}

CargoExecutionConfigType::~CargoExecutionConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

QString CargoExecutionConfigType::name() const
{
    return i18n("Cargo Launcher");
}

QList<KDevelop::LaunchConfigurationPageFactory*> CargoExecutionConfigType::configPages() const
{
    return factoryList;
}

QString CargoExecutionConfigType::typeId()
{
    return "CargoLauncherType";
}

QIcon CargoExecutionConfigType::icon() const
{
    return QIcon::fromTheme("cargo");
}

static bool canLaunchMetadataFile(const KDevelop::Path &path)
{
    KConfig cfg(path.toLocalFile(), KConfig::SimpleConfig);
    KConfigGroup group(&cfg, "Desktop Entry");
    QStringList services = group.readEntry("ServiceTypes", group.readEntry("X-KDE-ServiceTypes", QStringList()));
    return services.contains("Plasma/Applet");
}

//don't bother, nobody uses this interface
bool CargoExecutionConfigType::canLaunch(const QUrl& ) const
{
    return false;
}

bool CargoExecutionConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    KDevelop::ProjectFolderItem* folder = item->folder();
    if(folder && folder->hasFileOrFolder("metadata.desktop")) {
        return canLaunchMetadataFile(KDevelop::Path(folder->path(), "metadata.desktop"));
    }
    return false;
}

void CargoExecutionConfigType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    config.writeEntry("CargoIdentifier", item->path().toUrl().toLocalFile());
}

void CargoExecutionConfigType::configureLaunchFromCmdLineArguments(KConfigGroup /*config*/, const QStringList &/*args*/) const
{}

QMenu* CargoExecutionConfigType::launcherSuggestions()
{
    QList<QAction*> found;
    QList<KDevelop::IProject*> projects = KDevelop::ICore::self()->projectController()->projects();
    foreach(KDevelop::IProject* p, projects) {
        QSet<KDevelop::IndexedString> files = p->fileSet();
        foreach(const KDevelop::IndexedString& file, files) {
            KDevelop::Path path(file.str());
            if (path.lastPathSegment() == "metadata.desktop" && canLaunchMetadataFile(path)) {
                path = path.parent();
                QString relUrl = p->path().relativePath(path);
                QAction* action = new QAction(relUrl, this);
                action->setProperty("url", relUrl);
                action->setProperty("project", qVariantFromValue<KDevelop::IProject*>(p));
                connect(action, &QAction::triggered, this, &CargoExecutionConfigType::suggestionTriggered);
                found.append(action);
            }
        }
    }

    QMenu *m = nullptr;
    if(!found.isEmpty()) {
        m = new QMenu(i18n("Cargos"));
        m->addActions(found);
    }
    return m;
}

void CargoExecutionConfigType::suggestionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    KDevelop::IProject* p = action->property("project").value<KDevelop::IProject*>();
    QString relUrl = action->property("url").toString();

    KDevelop::ILauncher* launcherInstance = launchers().at( 0 );
    QPair<QString,QString> launcher = qMakePair( launcherInstance->supportedModes().at(0), launcherInstance->id() );

    QString name = relUrl.mid(relUrl.lastIndexOf('/')+1);
    KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher, p, name);
    KConfigGroup cfg = config->config();
    cfg.writeEntry("CargoIdentifier", relUrl);
    emit signalAddLaunchConfiguration(config);
}
