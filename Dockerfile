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
FROM ubuntu

RUN apt update -y
RUN apt install -y cmake catch2 ninja-build clang protobuf-compiler-grpc libgrpc++-dev libgtest-dev libgmock-dev libpthread-stubs0-dev

WORKDIR /cashemere

COPY ./cashmere.tar.gz .

RUN tar xzvf cashmere.tar.gz
RUN cmake --preset release
RUN cmake --build --preset release
RUN cmake --install build/release --prefix /usr

ENTRYPOINT ["cash", "-i", "aa", "-p", "5000", "-s"]
