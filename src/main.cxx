/* Copyright 2010, Martin Koller, kollix@aon.at

  This file is part of LillePOS.

  LillePOS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LillePOS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LillePOS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <stdio.h>

#include <MainWidget.hxx>

void printHelp()
{
  printf("LillePOS understands:\n"
         "-db ... open database defintion dialog\n"
         "-f | --fullscreen ... work in fullscreen mode\n"
         "-n | --noPrinter ... for tests, do not print to printer but to PDF\n"
         "--help ... this text\n");
}

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QString locale = QLocale::system().name();

  QTranslator trans;
  QString translationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
  if ( trans.load(QLatin1String("qt_") + locale, translationPath) )
    app.installTranslator(&trans);

  // LillePOS translation file is searched in the application install dir (e.g. /opt/LillePOS)
  // and also in /usr/share/LillePOS
  QTranslator trans2;  // if current language is not de and translation not found, use english
  if ( !locale.startsWith(QLatin1String("de")) &&
       (trans2.load(QLatin1String("LillePOS_") + locale, QCoreApplication::applicationDirPath()) ||
        trans2.load(QLatin1String("LillePOS_") + locale, QLatin1String("/usr/share/LillePOS")) ||
        trans2.load(QLatin1String("LillePOS_en"), QCoreApplication::applicationDirPath()) ||
        trans2.load(QLatin1String("LillePOS_en"), QLatin1String("/usr/share/LillePOS"))) )
    app.installTranslator(&trans2);


  MainWidget *mainWidget = new MainWidget;

  bool dbSelect = false;

  foreach(const QString &arg, QApplication::arguments())
  {
    if ( arg[0] != '-' )
      continue;

    if ( arg == "-db" )
      dbSelect = true;
    else if ( (arg == "--fullscreen") || (arg == "-f") )
    {
      mainWidget->setWindowState(mainWidget->windowState() ^ Qt::WindowFullScreen);
    }
    else if ( (arg == "--noPrinter") || (arg == "-n") )
    {
      mainWidget->setNoPrinter();
    }
    else // if ( arg == "--help" )  // --help or everything else we do not understand
    {
      printHelp();
      return 0;
    }
  }

  mainWidget->show();

  if ( !mainWidget->openDB(dbSelect) )
    return 0;

  return app.exec();
}
