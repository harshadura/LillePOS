LillePOS
========

LillePOS is a 'Point Of Sale' application originally implemented for a small coffee shop. 
It has written on C++/Qt and can be used with either SQLite/MySQL as the database.

LillePOS is Licensed under: GNU GENERAL PUBLIC LICENSE (See LICENSE file)

Original URL: http://qt-apps.org/content/show.php/LillePOS?content=136127

*Copyright 2010, Martin Koller, kollix@aon.at*

This is the README for LillePOS, a "Point Of Sale" application
originally implemented for a small coffee shop my sister runs,
( http://www.lillehus.at ) and designed to be used on
touch based devices, originally for the WeTab ( http://wetab.mobi/ ).

Although my native language is german, I write this file in english so
that hopefully a greater audience is reached.

LillePOS is used to handle orders for people sitting in the shop or also for products "to go",
printing invoices, doing reports, etc.

To run the system, you first need to configure the number of tables your shop has,
text which shall appear on the tickets, and chose the printers you want to use.
We use the TSC/TTP-245C printer for printing tickets.

The main thing you need to do is to define the products you sell and the price for them.
Products are organized in categories and groups.
Categories have groups, and each group contains products.

An example could be:
beverages
  - cold drinks
    - Cola
    - Beer
  - hot drinks
    - Cappucino
    - Latte Macchiato
Snacks
  - Toasts
    - Ham/Cheese Toast
  - Pizza
    - Margerita
and so on - you get the picture ;-) ...

You can take multiple orders per table (e.g. people pay by themself and not as a group)
and when paying you can join multiple tickets to one invoice by selecting multiple tickets
before clicking the "pay now" button.
In the main screen you see all open tickets. Paying a ticket closes it, assigns an invoice number
and removes it from the view of open tickets (data is never removed from the database).
Optionally, when paying, a ticket can be printed. The ticket shows the shop name (address, etc.)
and a logo.

When entering an order, you can optionally print a "serving preparation ticket", which
is simply a list of not yet served products. E.g. if you have an employee who is just
preparing the food, but is not directly accessing LillePOS on this PC. The still unserved
products are also listed per order on the main screen. Pressing the "served" button
marks the products as served and they will be hidden.

Apropos Employees:
You can also define the employees, and they shall register their "come and go" times
in the "Working Times" dialog. You can view/print a summary of the working times per employee
per month.

There is also the feature of viewing/printing a daily report, which is a summary of
all product groups and the sum of all invoices per group, including a summary on
the different tax groups.

Picture View:
There is also a feature which allows one to view a picture (e.g. a photo) and describing text for it.
The definition of the pictures is done in the settings dialog and a name per picture will be used
to show an additional button in the main view's bottom area left to the menu button.
This feature is meant as a quick access to information helping the waiters to prepare food,
e.g. products consisting of multiple items like a breakfast.
The picture files need to have the special names pictureButton<num>.jpg, num starting from 0,
e.g. pictureButton0.jpg and must be placed in the working directory

LillePOS can do an automatic backup of the database file (SQLite) or a database dump file (MySQL) to
either some filesystem location or also to e.g. an FTP server. If configured,
this will automatically be done whenever you create a daily report (assuming
this is the last step per day you do with LillePOS).
For an FTP server, you need to enter the full URL including a username and
a password if required, e.g. ftp://username:password@ftp.mybackup.server/backup/dir

Even though LillePOS is designed for touch based devices (e.g. large buttons
you can easily touch with your finger), it can of course also be run
and used with a mouse. It has already been used on a very small screen
netbook device (Aspire One) with just 1024x600, with the WeTab (1366x768) of course,
and also with a 17 inch Laptop, all running Linux of course!

Details probably only for developers (see also the INSTALL file):

LillePOS is based on Qt4 ( http://qt-project.org ).
It can run with the SQLite database ( http://www.sqlite.org/ )
(which you would normally do on a standalone PC) but also with a MySQL server
( http://dev.mysql.com/ ). The SQLite backend is the best tested one, as
it's the one we really use day to day.
As it's using Qt's QSql module, it should not be too hard to adapt it
to use any other database supported by Qt.

The source code contains german texts, wrapped in Qt's tr() calls,
so that it can be easily translated.
English translation is already available.

Credits:

Nearly all icons are taken from the Oxygen Icon Theme of the great KDE project
( http://www.kde.org )
