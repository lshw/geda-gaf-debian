# Copyright (C) 2013-2015 Roland Lutz
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
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

import getopt, sys, os
from gettext import gettext as _
import xorn.command
import xorn.config
import xorn.fileutils
import xorn.proxy
import xorn.storage
import xorn.geda.attrib
import xorn.geda.read
import xorn.geda.write

def unhide_attributes(ob):
    for attached in xorn.geda.attrib.find_attached_attribs(ob):
        attached_name, attached_value = \
            xorn.geda.attrib.parse_string(attached.text)
        for inherited in xorn.geda.attrib.find_inherited_attribs(ob):
            inherited_name, inherited_value = \
                xorn.geda.attrib.parse_string(inherited.text)
            if inherited_name == attached_name:
                inherited.visibility = attached.visibility

def read(path):
    try:
        return xorn.geda.read.read(path)
    except UnicodeDecodeError as e:
        sys.stderr.write(_("%s: can't read %s: %s\n")
                         % (xorn.command.program_short_name, path, str(e)))
        sys.exit(1)
    except xorn.geda.read.ParseError:
        sys.stderr.write(_("%s: can't read %s: %s\n")
                         % (xorn.command.program_short_name,
                            path, _("parse error")))
        sys.exit(1)

def write(rev, path):
    try:
        xorn.geda.write.write(rev, path)
    except (IOError, OSError) as e:
        sys.stderr.write(_("%s: can't write %s: %s\n")
                         % (xorn.command.program_short_name, path, e.strerror))
        sys.exit(1)

def write_file(data, path):
    try:
        def write_func(f):
            f.write(data)

        xorn.fileutils.write(path, write_func)
    except (IOError, OSError) as e:
        sys.stderr.write(_("%s: can't write %s: %s\n")
                         % (xorn.command.program_short_name, path, e.strerror))
        sys.exit(1)

def main():
    try:
        options, args = getopt.getopt(
            xorn.command.args, '', ['help', 'version'])
    except getopt.GetoptError as e:
        xorn.command.invalid_arguments(e.msg)

    for option, value in options:
        if option == '--help':
            sys.stdout.write(_(
"Usage: %s SCHEMATIC SYMBOL|PICTURE...\n") % xorn.command.program_name)
            sys.stdout.write(_(
"Extract objects embedded in a schematic into a separate file\n"))
            sys.stdout.write("\n")
            sys.stdout.write(_(
"      --help            give this help\n"
"      --version         display version number\n"))
            sys.stdout.write("\n")
            sys.stdout.write(_("Report %s bugs to %s\n")
                             % (xorn.config.PACKAGE_NAME,
                                xorn.config.PACKAGE_BUGREPORT))
            sys.exit(0)
        elif option == '--version':
            xorn.command.core_version()

    if len(args) < 2:
        xorn.command.invalid_arguments(_("not enough arguments"))

    rev = read(args[0])
    embedded_symbols = {}
    embedded_pixmaps = {}

    for ob in rev.toplevel_objects():
        data = ob.data()
        if isinstance(data, xorn.storage.Component) and data.symbol.embedded \
                and not data.symbol.basename in embedded_symbols:
            unhide_attributes(ob)
            embedded_symbols[data.symbol.basename.encode()] = \
                data.symbol.prim_objs
        if isinstance(data, xorn.storage.Picture) and data.pixmap.embedded:
            filename = os.path.basename(data.pixmap.filename)
            if not filename in embedded_pixmaps:
                embedded_pixmaps[filename.encode()] = data.pixmap.file_content

    for filename in args[1:]:
        basename = os.path.basename(filename)
        if basename not in embedded_symbols \
                and basename not in embedded_pixmaps:
            sys.stderr.write(_("%s: can't extract '%s': "
                               "No such embedded symbol or pixmap\n")
                             % (xorn.command.program_short_name, basename))
            sys.exit(1)

    for filename in args[1:]:
        basename = os.path.basename(filename)
        if basename in embedded_symbols:
            write(xorn.proxy.RevisionProxy(
                    embedded_symbols[basename]), filename)
        else:
            write_file(embedded_pixmaps[basename], filename)
