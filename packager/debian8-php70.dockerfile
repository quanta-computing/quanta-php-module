FROM debian:jessie
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN sed -i 's/httpredir\.debian\.org/ftp.fr.debian.org/g' /etc/apt/sources.list
RUN apt-get update && apt-get install -y curl
RUN echo 'deb http://apt.quanta.gr quanta/' > /etc/apt/sources.list.d/quanta.list
RUN echo 'deb http://packages.dotdeb.org jessie all' > /etc/apt/sources.list.d/dotdeb.list
RUN echo 'deb http://repo.mysql.com/apt/debian jessie mysql-5.6' > /etc/apt/sources.list.d/mysql.list
RUN curl -s https://www.quanta-monitoring.com/quanta-repo-key.gpg -o - | apt-key add -
RUN curl -s https://www.dotdeb.org/dotdeb.gpg -o - | apt-key add -
RUN apt-key adv --keyserver pgp.mit.edu --recv-keys 5072E1F5

RUN apt-get update && apt-get install -y \
  build-essential \
  debhelper \
  dh-exec \
  dh-autoreconf \
  autotools-dev \
  devscripts \
  dput \
  vim \
  quilt \
  openssh-client \
  sendmail \
  wget

RUN echo 'mysql-server mysql-server/root_password password ' | debconf-set-selections
RUN echo 'mysql-server mysql-server/root_password_again password ' | debconf-set-selections

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  apache2 \
  mysql-server \
  libcurl4-openssl-dev \
  quanta-agent \
  libmonikor-dev

RUN apt-get update && apt-get install -y \
  libapache2-mod-php7.0 \
  php7.0-dev \
  php7.0-curl \
  php7.0-gd \
  php7.0-intl \
  php7.0-json \
  php7.0-mcrypt \
  php7.0-mysql \
  php7.0-mbstring \
  php7.0-xml
