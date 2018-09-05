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
BuildRequires:  qt5-qtdeclarative-devel-tools
Requires:   qtpreloadengine

%description
Library for performing automatic testing QML applications.

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

%files
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/qmldir
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/libqaengine.so
%{_libdir}/qt5/qml/ru/omprussia/sailfishtest/plugin.qmltypes
%{_libdir}/qtpreloadplugins/libqaengine.so
