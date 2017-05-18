/************************************************************************
 * KDevelop4 Cargo Buildsystem Support                                 *
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

#include "cargobuildjob.h"

#include <KLocalizedString>
#include <KShell>

#include <interfaces/iproject.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/commandexecutor.h>
#include <project/projectmodel.h>

#include "cargoplugin.h"

using namespace KDevelop;

CargoBuildJob::CargoBuildJob( Cargo* plugin, KDevelop::ProjectBaseItem* item, const QString& command )
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

    QString title = i18nc("Running '<command> <arguments>'", "Running '%1'", cmd, command);
    setTitle(title);
    setObjectName(title);
    setDelegate( new KDevelop::OutputDelegate );
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

        setStandardToolView( KDevelop::IOutputView::BuildView );
        setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
        KDevelop::OutputModel* model = new KDevelop::OutputModel( QUrl::fromLocalFile(builddir) );
        model->setFilteringStrategy( KDevelop::OutputModel::CompilerFilter );
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

