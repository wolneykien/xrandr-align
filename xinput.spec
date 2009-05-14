Name: xinput
Version: 1.3.0
Release: alt1

Summary: utility to configure and test XInput devices
License: MIT/X11
Group: System/X11

Url: http://xorg.freedesktop.org
Source: %name-%version.tar.bz2

Packager: XOrg Maintainer Team <xorg@packages.altlinux.org>

BuildRequires: libX11-devel libXext-devel libXi-devel xorg-x11-util-macros

%description
xinput - utility to configure and test XInput devices

%prep
%setup -q

%build
%__autoreconf
%configure

%make_build

%install
%make DESTDIR=%buildroot install

%files
%_bindir/*
%_man1dir/*

%changelog
* Thu Dec 20 2007 Valery Inozemtsev <shrek@altlinux.ru> 1.3.0-alt1
- initial release
