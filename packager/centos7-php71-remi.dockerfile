FROM centos:centos7
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN curl -s https://www.quanta-monitoring.com/quanta-centos-repo.txt -o /etc/yum.repos.d/quanta.repo
RUN curl -s https://www.quanta-monitoring.com/quanta-repo-key.gpg -o /tmp/quanta.key \
  && rpm --import /tmp/quanta.key \
  && /bin/rm -f /tmp/quanta.key
RUN rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
RUN rpm -Uvh http://rpms.remirepo.net/enterprise/remi-release-7.rpm

RUN yum makecache fast

RUN yum -q -y groupinstall "Development tools"
RUN yum-config-manager --enable remi-php71

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
