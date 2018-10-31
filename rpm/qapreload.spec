%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

Name:       qapreload

Summary:    Preload library for QA
Version:    1.0.0
Release:    1
Group:      Qt/Qt
License:    LGPL3
URL:        https://github.com/coderus/qapreload
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  pkgconfig(libshadowutils)
BuildRequires:  pkgconfig(packagekitqt5)
BuildRequires:  pkgconfig(contentaction5)
BuildRequires:  pkgconfig(connman-qt5)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  qt5-tools
BuildRequires:  qt5-qtdeclarative-devel-tools
BuildRequires:  qt5-plugin-platform-minimal
BuildRequires:  mer-qdoc-template
Requires:   qtpreloadengine

%description
Library for performing automatic testing QML applications.

%package devel
Summary:    Preload library for QA. plugin.qmltypes
Group:      Qt/Qt
BuildArch:  noarch
Requires:   %{name}

%description devel
%{summary}.

%package doc
Summary:    SailfishTest plugin documentation
Group:      System/Libraries

%description doc
%{summary}.

%package indicator
Summary:    Touch indicator enabler
Group:      System/Libraries

%description indicator
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install
ln -s ../../../../../qtpreloadplugins/libqaengine.so %{buildroot}/usr/lib/qt5/qml/ru/omprussia/sailfishtest/libqaengine.so

/usr/lib/qt5/bin/qmlplugindump -v -noinstantiate -nonrelocatable ru.omprussia.sailfishtest 1.0 %{buildroot}%{_libdir}/qt5/qml > %{buildroot}%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/plugin.qmltypes |:
sed -i 's#%{buildroot}##g' %{buildroot}%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/plugin.qmltypes

mkdir -p %{buildroot}%{_sysconfdir}
touch %{buildroot}%{_sysconfdir}/qapreload-touch-indicator

%post
/bin/systemctl daemon-reload
/bin/systemctl stop qabridge.service
/bin/systemctl restart qabridge.socket
/bin/systemctl enable qabridge.socket
/bin/systemctl-user daemon-reload
/bin/systemctl-user restart qaservice.service

/bin/systemctl-user restart booster-qt5.service
/bin/systemctl-user restart booster-silica-qt5.service

%files
%defattr(-,root,root,-)
# library files
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/qmldir
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/libqaengine.so
%{_libdir}/qtpreloadplugins/libqaengine.so
# bridge files
%{_bindir}/qabridge
%{_bindir}/qabridge-user
/lib/systemd/system/qabridge.service
/lib/systemd/system/qabridge.socket
%{_datadir}/dbus-1/interfaces/ru.omprussia.qabridge.xml
%{_sysconfdir}/dbus-1/system.d/ru.omprussia.qabridge.conf
%{_libdir}/systemd/user/qaservice.service
%{_datadir}/dbus-1/services/ru.omprussia.qaservice.service
%{_datadir}/qapreload/qml/TouchIndicator.qml

%files devel
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/qmldir
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/plugin.qmltypes

%files doc
%defattr(-,root,root,-)
%dir %{_datadir}/doc/qapreload
%{_datadir}/doc/qapreload/qapreload.qch

%files indicator
%{_sysconfdir}/qapreload-touch-indicator
