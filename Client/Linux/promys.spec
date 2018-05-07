Summary: Promys - Project My Screen
Name: promys
Version: 1.2
Release: 1
Vendor: Olivier DEBON <olivier@debon.net>
Group: X11
License: BSD3
%description
Linux client to cast desktop to a Promys device
%prep
%build
%install
%files
/opt/promys/libavutil.so.56
/opt/promys/libswscale.so.5
/opt/promys/libx264.so.155
/opt/promys/promys.png
/opt/promys/screencast
/usr/share/applications/promys.desktop
