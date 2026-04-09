Name:           gearboy
Version:        %{?version}%{!?version:1.0.0}
Release:        1%{?dist}
Summary:        Game Boy / Game Boy Color emulator

License:        GPL-3.0-or-later
URL:            https://github.com/drhelius/Gearboy
Source0:        https://github.com/drhelius/Gearboy/archive/refs/tags/%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pkgconf-pkg-config
BuildRequires:  mesa-libGL-devel
BuildRequires:  SDL3-devel

Requires:       mesa-libGL
Requires:       SDL3

%description
Gearboy is a cross-platform Game Boy / Game Boy Color emulator
written in C++.

%prep
%autosetup -n Gearboy-%{version}

%build
%make_build -C platforms/linux \
    GIT_VERSION="%{version}" \
    USE_CLANG=0 \
    DEBUG=0

%install
install -Dm755 platforms/linux/%{name} %{buildroot}%{_prefix}/lib/%{name}/%{name}
ln -s %{_prefix}/lib/%{name}/%{name} %{buildroot}%{_bindir}/%{name}

install -Dm644 platforms/shared/gamecontrollerdb.txt %{buildroot}%{_prefix}/lib/%{name}/gamecontrollerdb.txt

install -dm755 %{buildroot}%{_prefix}/lib/%{name}/mcp/resources/hardware
install -Dm644 platforms/shared/desktop/mcp/resources/hardware/toc.json %{buildroot}%{_prefix}/lib/%{name}/mcp/resources/hardware/toc.json

install -Dm644 platforms/linux/debian/%{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
sed -i 's|/usr/games/gearboy|gearboy|g' %{buildroot}%{_datadir}/applications/%{name}.desktop

install -Dm644 platforms/shared/desktop/mcp/icon.png %{buildroot}%{_datadir}/icons/hicolor/128x128/apps/%{name}.png

install -Dm644 platforms/linux/debian/%{name}.6 %{buildroot}%{_mandir}/man6/%{name}.6

%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}
%{_prefix}/lib/%{name}/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_mandir}/man6/%{name}.6*

%changelog
* %(date "+%a %b %d %Y") Nacho Sanchez <863613+drhelius@users.noreply.github.com> - %{version}-1
- Release %{version}
