FROM centos:centos7
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN curl -s https://www.quanta-monitoring.com/quanta-centos-repo.txt -o /etc/yum.repos.d/quanta.repo
RUN curl -s https://www.quanta-monitoring.com/quanta-repo-key.gpg -o /tmp/quanta.key \
  && rpm --import /tmp/quanta.key \
  && /bin/rm -f /tmp/quanta.key
RUN yum -q -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
RUN yum -q -y install http://mirror.webtatic.com/yum/el7/webtatic-release.rpm

RUN yum makecache fast

RUN yum -q -y groupinstall "Development tools"

RUN yum -q -y install \
  quanta-agent \
  php55w \
  php55w-devel \
  php55w-gd \
  php55w-mbstring \
  libmonikor-devel \
  libcurl-devel \
  rpm-build \
  rpmdevtools

RUN rpmdev-setuptree
