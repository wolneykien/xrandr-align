Name: xinput
Version: 1.4.2
Release: alt1
Summary: utility to configure and test XInput devices
License: MIT/X11
Group: System/X11
Url: http://xorg.freedesktop.org
Packager: Valery Inozemtsev <shrek@altlinux.ru>

Source: %name-%version.tar
Patch: %name-%version-%release.patch

BuildRequires: libX11-devel libXext-devel libXi-devel xorg-util-macros

%description
xinput - utility to configure and test XInput devices

%prep
%setup -q
%patch -p1

%build
%autoreconf
%configure

%make_build

%install
%make DESTDIR=%buildroot install

%files
%_bindir/*
%_man1dir/*

%changelog
* Thu May 14 2009 Valery Inozemtsev <shrek@altlinux.ru> 1.4.2-alt1
- 1.4.2

* Thu Dec 20 2007 Valery Inozemtsev <shrek@altlinux.ru> 1.3.0-alt1
- initial release
