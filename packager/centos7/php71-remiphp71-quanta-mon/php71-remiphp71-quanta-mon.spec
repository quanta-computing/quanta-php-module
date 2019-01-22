Name:	php71-remiphp71-quanta-mon
Version: 1.2.2
Release: 2%{?dist}
Summary: This package provides a PHP profiling module built for Magento.
Distribution: Quanta
Vendor: Quanta-Computing
Packager: Matthieu ROSINSKI <support@quanta-computing.com>
Group: Applications/Internet
URL: http://www.quanta-computing.com
License: toto
Source: php71-remiphp71-quanta-mon.tgz

Buildroot	: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: php71-php-devel
BuildRequires: libmonikor-devel
Requires: php71-php(zend-abi) = 20160303-64
Requires: php71-php(api) = 20160303-64
Requires: libmonikor >= 1.0.0

%description
This package contains a PHP extension to allow profiling for Quanta probes.

%prep
%setup -n php71-remiphp71-quanta-mon

%build
/opt/remi/php71/root/bin/phpize
%configure --with-php-config=/opt/remi/php71/root/bin/php-config

%install
make clean all
mkdir -p %{buildroot}/etc/opt/remi/php71/php.d/
mkdir -p %{buildroot}/opt/remi/php71/root/usr/lib64/php/modules/
install -m 644 quanta_mon.ini %{buildroot}/etc/opt/remi/php71/php.d/20-quanta_mon.ini
install -m 770 modules/quanta_mon.so %{buildroot}/opt/remi/php71/root/usr/lib64/php/modules/quanta_mon.so

%files
/opt/remi/php71/root/usr/lib64/php/modules/quanta_mon.so
/etc/opt/remi/php71/php.d/20-quanta_mon.ini
