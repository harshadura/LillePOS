TARGET = LillePOS

QT += sql
QT += network

SOURCES += main.cxx
SOURCES += MainWidget.cxx
SOURCES += SettingsDialog.cxx
SOURCES += TableSelection.cxx
SOURCES += OpenTicketWidget.cxx
SOURCES += OpenTicketsListWidget.cxx
SOURCES += CategoriesWidget.cxx
SOURCES += GroupsWidget.cxx
SOURCES += GroupEdit.cxx
SOURCES += ProductsWidget.cxx
SOURCES += ProductEdit.cxx
SOURCES += ClosedTickets.cxx
SOURCES += DaySelectorDialog.cxx
SOURCES += MonthSelectorDialog.cxx
SOURCES += PictureView.cxx
SOURCES += PictureButtonSettings.cxx
SOURCES += WorkingTimes.cxx
SOURCES += WorkingTimesList.cxx
SOURCES += TouchSpinBox.cxx
SOURCES += DateTimeEdit.cxx
SOURCES += TouchDelegate.cxx
SOURCES += ReportView.cxx
SOURCES += EmployeesWidget.cxx
SOURCES += DatabaseDefinition.cxx
SOURCES += FinishTicketDialog.cxx
SOURCES += Backup.cxx

HEADERS += MainWidget.hxx
HEADERS += SettingsDialog.hxx
HEADERS += TableSelection.hxx
HEADERS += OpenTicketWidget.hxx
HEADERS += OpenTicketsListWidget.hxx
HEADERS += CategoriesWidget.hxx
HEADERS += GroupsWidget.hxx
HEADERS += GroupEdit.hxx
HEADERS += ProductsWidget.hxx
HEADERS += ProductEdit.hxx
HEADERS += ClosedTickets.hxx
HEADERS += DaySelectorDialog.hxx
HEADERS += MonthSelectorDialog.hxx
HEADERS += PictureView.hxx
HEADERS += PictureButtonSettings.hxx
HEADERS += WorkingTimes.hxx
HEADERS += WorkingTimesList.hxx
HEADERS += TouchSpinBox.hxx
HEADERS += DateTimeEdit.hxx
HEADERS += TouchDelegate.hxx
HEADERS += ReportView.hxx
HEADERS += EmployeesWidget.hxx
HEADERS += DatabaseDefinition.hxx
HEADERS += FinishTicketDialog.hxx
HEADERS += Backup.hxx

FORMS += LillePOS.ui
FORMS += SettingsDialog.ui
FORMS += CategoriesWidget.ui
FORMS += GroupsWidget.ui
FORMS += GroupEdit.ui
FORMS += ProductsWidget.ui
FORMS += ProductEdit.ui
FORMS += ClosedTickets.ui
FORMS += DaySelectorDialog.ui
FORMS += MonthSelectorDialog.ui
FORMS += PictureButtonSettings.ui
FORMS += WorkingTimes.ui
FORMS += WorkingTimesList.ui
FORMS += DateTimeEdit.ui
FORMS += TouchSpinBox.ui
FORMS += ReportView.ui
FORMS += EmployeesWidget.ui
FORMS += DatabaseDefinition.ui
FORMS += FinishTicketDialog.ui
FORMS += Backup.ui

RESOURCES += LillePOS.qrc

target.path = /usr/bin
INSTALLS += target

# rules copied from Qt4 translations .pro file
TRANSLATIONS += LillePOS_en.ts

translations.path = /usr/share/LillePOS
translations.files = $$TRANSLATIONS
translations.files ~= s,\\.ts$,.qm,g
translations.CONFIG += no_check_exist
INSTALLS += translations

icon.path = /usr/share/pixmaps
icon.files = LillePOS.png
INSTALLS += icon

desktop.path = /usr/share/applications
desktop.files = LillePOS.desktop
INSTALLS += desktop
