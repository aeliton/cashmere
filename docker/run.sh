# Cashmere - a distributed conflict-free replicated database.
# Copyright (C) 2025 Aeliton G. Silva
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#!/usr/bin/env bash

cash -i aa -d /cashmere/db/aa -p 5000 -s add 10 &> /dev/null & disown

while ! nc -z localhost 5000; do   
  sleep 0.1
done
until [ -e /cashmere/db/aa ]; do sleep 0.1; done

cash -i bb -d /cashmere/db/bb -p 5001 -s &> /dev/null & disown

while ! nc -z localhost 5001; do   
  sleep 0.1
done

cash -p 5001 connect 0.0.0.0:5000

cash -p 5000 quit
cash -p 5001 quit
