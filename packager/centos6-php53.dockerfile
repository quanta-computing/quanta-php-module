FROM centos:centos6
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN curl -s https://www.quanta-monitoring.com/quanta-centos-repo.txt -o /etc/yum.repos.d/quanta.repo
RUN curl -s https://www.quanta-monitoring.com/quanta-repo-key.gpg -o /tmp/quanta.key \
  && rpm --import /tmp/quanta.key \
  && /bin/rm -f /tmp/quanta.key
RUN sed -i'' -e 's/rpm\.quanta\.gr\/quanta/rpm.quanta.gr\/quanta-el6/g' /etc/yum.repos.d/quanta.repo

RUN yum makecache fast

RUN yum -q -y groupinstall "Development tools"

RUN yum -q -y install \
  quanta-agent \
  php \
  php-devel \
  php-gd \
  php-mbstring \
  libmonikor-devel \
  libcurl-devel \
  rpm-build \
  rpmdevtools

RUN rpmdev-setuptree
