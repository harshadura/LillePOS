#
# spec file for package LillePOS
#
# Copyright 2010-2013, Martin Koller, kollix@aon.at

# norootforbuild

Name:           LillePOS
Version:        1.2
Release:        1
Summary:        A 'Point Of Sale' application for small cafes/bars/restaurants

Group:          Productivity/Office/Other
License:        GNU General Public License version 3.0 (GPLv3)
URL:            http://qt-apps.org/content/show.php/LillePOS?content=136127
Source:         %{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  libqt4-devel

%description
LillePOS is a 'Point Of Sale' application for small cafes/bars/restaurants
designed to be used on touch based devices, originally for the
WeTab (http://wetab.mobi/) but can of course be used on
a netbook or desktop as well.
It uses either SQLite or MySQL to store all data.


%prep
%setup -q


%build
%if 0%{?mandriva_version} || 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
export PATH=/usr/lib/qt4/bin:$PATH  
export QTDIR=%{_prefix}/lib/qt4/  
%endif

qmake
make %{?_smp_mflags}
lrelease LillePOS.pro


%install
make install INSTALL_ROOT=%{buildroot}
%if 0%{?suse_version}  
%suse_update_desktop_file %name
%else  
%endif

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/usr/bin/LillePOS
/usr/share/LillePOS
/usr/share/applications/LillePOS.desktop
/usr/share/pixmaps/LillePOS.png
%doc README INSTALL COPYING
