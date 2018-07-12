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

%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

%post ld
if grep libqapreload /etc/ld.so.preload > /dev/null; then
    echo "Preload already exists"
else
    echo /usr/lib/libqapreload.so >> /etc/ld.so.preload
fi
/sbin/ldconfig

%postun ld
case "$*" in
0)
echo Uninstalling package
sed -i "/libqapreload/ d" /etc/ld.so.preload
;;
1)
echo Upgrading package
;;
*) echo case "$*" not handled in postun
esac
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libqapreload.so
%{_libdir}/libqaengine.so

%files ld
