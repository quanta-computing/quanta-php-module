FROM centos:centos7
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN curl -s https://rpm.quanta.io/quanta-centos-repo.txt -o /etc/yum.repos.d/quanta.repo
RUN curl -s https://rpm.quanta.io/quanta-repo-key.gpg -o /tmp/quanta.key \
  && rpm --import /tmp/quanta.key \
  && /bin/rm -f /tmp/quanta.key
RUN rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
RUN rpm -Uvh https://centos7.iuscommunity.org/ius-release.rpm

RUN yum makecache fast

RUN yum -q -y groupinstall "Development tools"

RUN yum -q -y install \
  quanta-agent \
  php71u \
  php71u-devel \
  php71u-gd \
  php71u-mbstring \
  libmonikor-devel \
  libcurl-devel \
  rpm-build \
  rpmdevtools

RUN rpmdev-setuptree
