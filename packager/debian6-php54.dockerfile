FROM debian/eol:squeeze
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN sed -i 's/httpredir\.debian\.org/ftp.fr.debian.org/g' /etc/apt/sources.list
RUN apt-get update && apt-get install -y curl
RUN echo 'deb http://apt.quanta.gr quanta-squeeze/' > /etc/apt/sources.list.d/quanta.list
RUN curl -s https://www.quanta-monitoring.com/quanta-repo-key.gpg -o - | apt-key add -
RUN curl -s https://www.dotdeb.org/dotdeb.gpg -o - | apt-key add -
RUN apt-key adv --keyserver pgp.mit.edu --recv-keys 5072E1F5

RUN apt-get update && apt-get install -y \
  build-essential \
  debhelper \
  dh-autoreconf \
  autotools-dev \
  devscripts \
  dput \
  vim \
  quilt \
  openssh-client \
  wget

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  apache2 \
  libcurl4-openssl-dev \
  quanta-agent \
  libmonikor-dev \
  libyaml-dev

RUN echo 'deb http://packages.dotdeb.org squeeze-php54 all' > /etc/apt/sources.list.d/dotdeb.list

RUN apt-get update && apt-get install -y \
  php5 \
  php5-dev \
  php5-dbg \
  libapache2-mod-php5
