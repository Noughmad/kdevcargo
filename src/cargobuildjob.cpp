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

#include "cargobuildjob.h"

#include <KLocalizedString>
#include <KShell>

#include <interfaces/iproject.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <outputview/filtereditem.h>
#include <outputview/outputfilteringstrategies.h>
#include <util/commandexecutor.h>
#include <project/projectmodel.h>

#include "cargoplugin.h"

using namespace KDevelop;

class CargoFilterStrategy : public KDevelop::IFilterStrategy
{
public:
    explicit CargoFilterStrategy(const QUrl& buildDir);
    virtual ~CargoFilterStrategy();

    FilteredItem errorInLine(const QString& line) override;
    FilteredItem actionInLine(const QString& line) override;

private:
    KDevelop::Path buildDir;
    QString currentFile;
    KDevelop::FilteredItem::FilteredOutputItemType currentItemType;
};

CargoFilterStrategy::CargoFilterStrategy(const QUrl& buildDir)
 : buildDir(buildDir)
{
}

CargoFilterStrategy::~CargoFilterStrategy()
{
}

KDevelop::FilteredItem CargoFilterStrategy::errorInLine(const QString& line)
{
    KDevelop::FilteredItem item(line);
    if (line.startsWith(QStringLiteral("error:")) || line.startsWith(QStringLiteral("error[")))
    {
        item.type = FilteredItem::ErrorItem;
    }
    else if (line.startsWith(QStringLiteral("warning:")) || line.startsWith(QStringLiteral("warning[")))
    {
        item.type = FilteredItem::WarningItem;
    }
    else if (line.startsWith(QStringLiteral("   Compiling"))
            || line.startsWith(QStringLiteral("    Finished")))
    {
        item.type = FilteredItem::ActionItem;
    }
    else
    {
        QStringList elements = line.split(' ', QString::SkipEmptyParts);
        if (elements.size() > 1 && elements[0] == QStringLiteral("-->"))
        {
            item.type = currentItemType;
            QStringList location = elements[1].split(':');
            if (location.size() > 0)
            {
                currentFile = location[0];

                item.isActivatable = true;
                item.url = Path(buildDir, currentFile).toUrl();
                if (location.size() > 1)
                {
                    /*
                     * Cargo counts lines from 1, and so does Kate,
                     * but KDevelop internally counts from 0,
                     * so we have to decrement the line number by 1.
                     * The same is true for column numbers.
                     */
                    item.lineNo = location[1].toInt() - 1;
                }
                if (location.size() > 2)
                {
                    item.columnNo = location[2].toInt() - 1;
                }
            }
        }
        else if (elements.size() >= 1 && (elements[0] == '|' || elements[0] == '='))
        {
            item.type = FilteredItem::InformationItem;
        }
        else if (elements.size() >= 2 && elements[1] == '|')
        {
            item.type = FilteredItem::InformationItem;
            item.isActivatable = true;
            item.url = Path(buildDir, currentFile).toUrl();
            item.lineNo = elements[0].toInt() - 1;

            /*
             * We determine the column number from the line itself,
             * as the first non-space character after the line number and '|'.
             */
            int idx = elements[0].size() + 3;
            int length = line.size();

            item.columnNo = 0;
            for (int i = idx; i < length; ++i)
            {
                if (!line[i].isSpace())
                {
                    item.columnNo = i - idx;
                    break;
                }
            }
        }
        else
        {
            item.type = FilteredItem::StandardItem;
        }
    }
    currentItemType = item.type;
    return item;
}

KDevelop::FilteredItem CargoFilterStrategy::actionInLine(const QString& line)
{
    return KDevelop::FilteredItem(line);
}


CargoBuildJob::CargoBuildJob( CargoPlugin* plugin, KDevelop::ProjectBaseItem* item, const QString& command )
    : OutputJob( plugin )
    , command( command)
    , exec(nullptr)
    , killed( false )
    , enabled( false )
{
    setCapabilities( Killable );
    QString subgrpname;
    projectName = item->project()->name();
    builddir = plugin->buildDirectory( item ).toLocalFile();

    cmd = "cargo";

    QString title = i18nc("<command> <arguments>", "%1 %2", cmd, command);
    setTitle(title);
    setObjectName(title);
    setDelegate( new KDevelop::OutputDelegate );
    standardViewType = KDevelop::IOutputView::BuildView;
}

void CargoBuildJob::start()
{
    if (command.isEmpty())
    {
        setError( NoCommand );
        setErrorText( i18n( "No Cargo command specified" ) );
        emitResult();
    }
    else
    {
        QStringList arguments;
        arguments << command;
        if (!installPrefix.isEmpty())
        {
            arguments << QStringLiteral("--root") << installPrefix.toLocalFile();
        }

        if (!runArguments.isEmpty())
        {
            arguments << runArguments;
        }

        setStandardToolView( standardViewType );
        setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
        QUrl buildUrl = QUrl::fromLocalFile(builddir);
        KDevelop::OutputModel* model = new KDevelop::OutputModel(buildUrl);
        model->setFilteringStrategy(new CargoFilterStrategy(buildUrl));
        setModel( model );

        startOutput();

        exec = new KDevelop::CommandExecutor( cmd, this );

        exec->setArguments( arguments );
        exec->setWorkingDirectory( builddir );
        
        connect( exec, &CommandExecutor::completed, this, &CargoBuildJob::procFinished );
        connect( exec, &CommandExecutor::failed, this, &CargoBuildJob::procError );

        connect( exec, &CommandExecutor::receivedStandardError, model, &OutputModel::appendLines );
        connect( exec, &CommandExecutor::receivedStandardOutput, model, &OutputModel::appendLines );

        model->appendLine( QStringLiteral("%1> %2 %3").arg( builddir ).arg( cmd ).arg( KShell::joinArgs(arguments) ) );
        exec->start();
    }
}

bool CargoBuildJob::doKill()
{
    killed = true;
    exec->kill();
    return true;
}

void CargoBuildJob::procError( QProcess::ProcessError err )
{
    if( !killed ) {
        if( err == QProcess::FailedToStart ) {
            setError( FailedToStart );
            setErrorText( i18n( "Failed to start command." ) );
        } else if( err == QProcess::Crashed ) {
            setError( Crashed );
            setErrorText( i18n( "Command crashed." ) );
        } else {
            setError( UnknownExecError );
            setErrorText( i18n( "Unknown error executing command." ) );
        }
    }
    emitResult();
}

KDevelop::OutputModel* CargoBuildJob::model()
{
    return qobject_cast<KDevelop::OutputModel*>( OutputJob::model() );
}

void CargoBuildJob::procFinished(int code)
{
    //TODO: Make this configurable when the first report comes in from a tool
    //      where non-zero does not indicate error status
    if( code != 0 ) {
        setError( FailedShownError );
        model()->appendLine( i18n( "*** Failed ***" ) );
    } else {
        model()->appendLine( i18n( "*** Finished ***" ) );
    }
    emitResult();
}

