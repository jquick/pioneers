# Pioneers - Implementation of the excellent Settlers of Catan board game.
#   Go buy a copy.
#
# Copyright (C) 1999 Dave Cole
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

# see http://developer.gnome.org/dotplan/porting/ar01s06.html

figdir = images
docname = pioneers
lang = C
omffile = pioneers-C.omf
entities = legal.xml
include $(top_srcdir)/xmldocs.make
dist-hook: app-dist-hook

# HTMLHelp is the compressed help format for Microsoft Windows
htmlhelp: pioneers.xml
	xmlto -m custom.xsl htmlhelp pioneers.xml
	-/cygdrive/c/Program\ Files/HTML\ Help\ Workshop/hhc.exe htmlhelp.hhp
	rm htmlhelp.hhp
	rm toc.hhc
	rm *.html

simplehtml: pioneers.xml
	xmlto -m custom.xsl html pioneers.xml

html: pioneers.xml
	xsltproc /usr/share/xml/gnome/xslt/docbook/html/db2html.xsl pioneers.xml > index.html
	cp /usr/share/gnome-doc-utils/icons/hicolor/48x48/admon-note.png  .
