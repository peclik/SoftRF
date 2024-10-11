#!/bin/sh

#
# uf2conv.sh
#
# Copyright (C) 2024 Richard Pecl
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# convert firmware in hex format into UF2 format for update via nRF52 device (Badge) mass storage
python/uf2conv.py -f 0xada52840 -c -o SoftRF-firmware-vX.Y.Z-pcl-nRF52.uf2 build/adafruit.nrf52.pca10056/SoftRF.ino.hex
