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

#ifndef CUSTOMBUILDJOB_H
#define CUSTOMBUILDJOB_H

#include <outputview/outputjob.h>
#include <QProcess>
#include <QUrl>

class Cargo;
namespace KDevelop
{
class ProjectBaseItem;
class CommandExecutor;
class OutputModel;
class IProject;
}

class CargoBuildJob : public KDevelop::OutputJob
{
Q_OBJECT
public:
    enum ErrorType {
        UndefinedBuildType = UserDefinedError,
        FailedToStart,
        UnknownExecError,
        Crashed,
        WrongArgs,
        ToolDisabled,
        NoCommand
    };

    CargoBuildJob( Cargo*, KDevelop::ProjectBaseItem*, const QString& command );
    void start() override;
    bool doKill() override;

    void setInstallPrefix(const QUrl &installPrefix) { this->installPrefix = installPrefix; }
    void setRunArguments(const QStringList &arguments) { this->runArguments = arguments; }

private slots:
    void procFinished(int);
    void procError( QProcess::ProcessError );
private:
    KDevelop::OutputModel* model();
    QString command;
    QString projectName;
    QString cmd;
    QString environment;
    QString builddir;
    QUrl installPrefix;
    QStringList runArguments;
    KDevelop::CommandExecutor* exec;
    bool killed;
    bool enabled;
};

#endif 
