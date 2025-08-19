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
FROM ubuntu AS build

RUN apt update -y
RUN apt install -y \
 cmake ninja-build clang protobuf-compiler-grpc libgrpc++-dev \
 libgtest-dev libgmock-dev libedit-dev libspdlog-dev

WORKDIR /cashmere

COPY ./cashmere.tar.gz .

RUN tar xzvf cashmere.tar.gz
RUN cmake --preset release
RUN cmake --build --preset release
RUN cmake --install build/release --prefix /usr/local

FROM ubuntu AS runtime

RUN apt update -y
RUN apt install -y libgrpc++1.51t64 libedit2 libspdlog1.12

COPY --from=build /usr/local/. /usr/local/

FROM runtime AS test-runtime

RUN apt install -y netcat-openbsd screen

COPY ./run.sh /tmp/

RUN bash /tmp/run.sh
