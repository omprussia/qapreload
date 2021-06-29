# Copyright (c) 2019-2020 Open Mobile Platform LLC.
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

Name:       qapreload

Summary:    Preload library for QA
Version:    2.0.0
Release:    1
Group:      Qt/Qt
License:    LGPL3
URL:        https://github.com/omprussia/qapreload
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5XmlPatterns)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  pkgconfig(libshadowutils)
BuildRequires:  pkgconfig(packagekitqt5)
BuildRequires:  pkgconfig(contentaction5)
BuildRequires:  pkgconfig(connman-qt5)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(rpm)
BuildRequires:  qt5-tools
BuildRequires:  qt5-qtdeclarative-devel-tools
BuildRequires:  qt5-plugin-platform-minimal
Obsoletes:      qtpreloadengine
Obsoletes:      qtpreloadengine-ld
Suggests:       qapreload-ld
Suggests:       screenrecorder

%description
Library for performing automatic testing QML applications.

%package ld
Summary:    ld.so.preload enabler
Group:      System/System
BuildArch:  noarch
Requires:   %{name}

%description ld
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 \
    "PROJECT_PACKAGE_VERSION=%{version}" \
    SPEC_UNITDIR=%{_unitdir} \
    DEFINES+=Q_OS_SAILFISH \
    DEFINES+=USE_DBUS \
    DEFINES+=USE_SYSTEMD \
    DEFINES+=USE_PACKAGEKIT \
    DEFINES+=USE_RPM \
    DEFINES+=USE_CONNMAN \
    DEFINES+=USE_MLITE5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

mkdir -p %{buildroot}%{_libdir}/systemd/user/user-session.target.wants
ln -s ../qaservice.service %{buildroot}%{_libdir}/systemd/user/user-session.target.wants/

mkdir -p %{buildroot}%{_datadir}/qt5/qapreload
touch %{buildroot}%{_datadir}/qt5/qapreload/.keep

%pre
/usr/bin/env systemctl disable qabridge.service ||:

%post
/usr/bin/env systemctl daemon-reload
/usr/bin/env systemctl restart qabridge.service
/usr/bin/env systemctl enable qabridge.service

/usr/bin/env systemctl-user daemon-reload
/usr/bin/env systemctl-user restart qaservice.service

%pre ld
/usr/bin/env systemctl disable qabridge.socket ||:

%post ld
if grep libqapreloadhook /etc/ld.so.preload > /dev/null; then
    echo "Preload already exists"
else
    echo %{_libdir}/libqapreloadhook.so >> /etc/ld.so.preload
fi
/sbin/ldconfig

/usr/bin/env systemctl disable qabridge.socket
/usr/bin/env systemctl stop qabridge.socket
/usr/bin/env systemctl enable qabridge.service
/usr/bin/env systemctl restart qabridge.service

/usr/bin/env systemctl-user restart booster-qt5.service
/usr/bin/env systemctl-user restart booster-silica-qt5.service

%preun ld
if [ "$1" = "0" ]; then
echo Uninstalling package
sed -i "/libqapreloadhook/ d" /etc/ld.so.preload
fi
/sbin/ldconfig

%files
%defattr(-,root,root,-)
# hook files
%{_libdir}/libqapreloadhook.so
# library files
%{_libdir}/libqaengine.so
# bridge files
%{_bindir}/qabridge
%{_bindir}/qabridge-user
%{_unitdir}/qabridge.service
%{_unitdir}/qabridge.socket
%{_userunitdir}/qaservice.service
%{_userunitdir}/user-session.target.wants/qaservice.service
%{_datadir}/dbus-1/services/ru.omprussia.qaservice.service
%{_datadir}/qt5/qapreload

%files ld
