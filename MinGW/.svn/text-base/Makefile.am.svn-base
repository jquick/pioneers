# Pioneers - Implementation of the excellent Settlers of Catan board game.
#   Go buy a copy.
#
# Copyright (C) 2011 Roland Clobus <rclobus@rclobus.nl>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

if IS_MINGW_PORT

install-MinGW:
	cp /mingw/bin/libgdk-win32-2.0-0.dll /usr/local
	cp /mingw/bin/libgdk_pixbuf-2.0-0.dll /usr/local
	cp /mingw/bin/libglib-2.0-0.dll /usr/local
	cp /mingw/bin/libgobject-2.0-0.dll /usr/local
	cp /mingw/bin/libgtk-win32-2.0-0.dll /usr/local
	cp /mingw/bin/intl.dll /usr/local
	cp /mingw/bin/libpango-1.0-0.dll /usr/local
	cp /mingw/bin/libcairo-2.dll /usr/local
	cp /mingw/bin/libgio-2.0-0.dll /usr/local
	cp /mingw/bin/libpangocairo-1.0-0.dll /usr/local
	cp /mingw/bin/libgio-2.0-0.dll /usr/local
	cp /mingw/bin/libgmodule-2.0-0.dll /usr/local
	cp /mingw/bin/libatk-1.0-0.dll /usr/local
	cp /mingw/bin/libpangowin32-1.0-0.dll /usr/local
	cp /mingw/bin/libfontconfig-1.dll /usr/local
	cp /mingw/bin/freetype6.dll /usr/local
	cp /mingw/bin/libpng12-0.dll /usr/local
	cp /mingw/bin/libpangoft2-1.0-0.dll /usr/local
	cp /mingw/bin/libexpat-1.dll /usr/local
	cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-png.dll /usr/local
	mkdir -p /usr/local/etc/gtk-2.0
	cp $(srcdir)/MinGW/gdk-pixbuf.loaders /usr/local/etc/gtk-2.0
	cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-png.dll /usr/local

endif # IS_MINGW_PORT

EXTRA_DIST += \
	$(srcdir)/MinGW/gdk-pixbuf.loaders
