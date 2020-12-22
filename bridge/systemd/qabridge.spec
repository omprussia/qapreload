# Copyright (c) 2019-2020 Open Mobile Platform LLC.
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

Name:       qabridge

Summary:    QABridge for Appium server
Version:    1.0.0
Release:    1
Group:      Qt/Qt
License:    LGPL3
URL:        https://github.com/omp-qa/qabridge
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(systemd)
Requires:   qapreload

%description
QTcpServer for executing queries from the Appium server.

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%post
/bin/systemctl daemon-reload
/bin/systemctl enable qabridge.socket
/bin/systemctl start qabridge.socket
/bin/systemctl stop qabridge.service

%preun
/bin/systemctl daemon-reload
/bin/systemctl disable qabridge.socket
/bin/systemctl stop qabridge.socket
/bin/systemctl stop qabridge.service

%files
%defattr(-,root,root,-)
%{_bindir}/qabridge
/lib/systemd/system/qabridge.service
/lib/systemd/system/qabridge.socket
