# Copyright (c) 2019-2020 Open Mobile Platform LLC.
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

%post ld
if grep libqapreloadhook /etc/ld.so.preload > /dev/null; then
    echo "Preload already exists"
else
    echo /usr/lib/libqapreloadhook.so >> /etc/ld.so.preload
fi
/sbin/ldconfig

%preun ld
if [ "$1" = "0" ]; then
echo Uninstalling package
sed -i "/libqapreloadhook/ d" /etc/ld.so.preload
fi
/sbin/ldconfig

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
# hook files
%{_libdir}/libqapreloadhook.so
# library files
%{_libdir}/libqaengine.so
# bridge files
%{_bindir}/qabridge
%{_bindir}/qabridge-user
/lib/systemd/system/qabridge.service
/lib/systemd/system/qabridge.socket
%{_libdir}/systemd/user/qaservice.service
%{_datadir}/dbus-1/services/ru.omprussia.qaservice.service

%files ld
