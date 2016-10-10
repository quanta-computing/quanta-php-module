Name:	php70-remi-quanta-mon
Version: 1.2.0
Release: 1%{?dist}
Summary: This package provides a PHP profiling module built for Magento.
Distribution: Quanta
Vendor: Quanta-Computing
Packager: Matthieu ROSINSKI <support@quanta-computing.com>
Group: Applications/Internet
URL: http://www.quanta-computing.com
License: toto
Source: php70-remi-quanta-mon.tgz

Buildroot	: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%global php_apiver  %((echo 0; /opt/remi/php70/root/usr/bin/php -i 2>/dev/null | sed -n 's/^PHP API => //p') | tail -1)

BuildRequires: php70-php-devel
Requires: php70-php(api) = %{php_apiver}-64
Requires: php70-php(zend-abi) = %{php_apiver}-64
Requires: libmonikor-devel >= 1.0.0

%description
This package contains a PHP extension to allow profiling for Quanta probes.

%prep
%setup -n php70-remi-quanta-mon

%build
/opt/remi/php70/root/usr/bin/phpize
%configure --with-php-config=/opt/remi/php70/root/usr/bin/php-config

%install
make
mkdir -p %{buildroot}/etc/opt/remi/php70/php.d/
mkdir -p %{buildroot}/opt/remi/php70/root/usr/lib64/php/modules
install -m 644 quanta_mon.ini %{buildroot}/etc/opt/remi/php70/php.d/20-quanta_mon.ini
install -m 755 modules/quanta_mon.so %{buildroot}/opt/remi/php70/root/usr/lib64/php/modules/quanta_mon.so

%files
/opt/remi/php70/root/usr/lib64/php/modules/quanta_mon.so
/etc/opt/remi/php70/php.d/20-quanta_mon.ini
