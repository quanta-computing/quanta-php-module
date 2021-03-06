Name:	php56-quanta-mon
Version: 1.3.0-beta
Release: 1%{?dist}
Summary: This package provides a PHP profiling module built for Magento.
Distribution: Quanta
Vendor: Quanta-Computing
Packager: Matthieu ROSINSKI <support@quanta-computing.com>
Group: Applications/Internet
URL: http://www.quanta-computing.com
License: toto
Source: php56-quanta-mon.tgz

Buildroot	: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: php-devel
Requires: php(zend-abi) = %{php_zend_api}
Requires: php(api) = %{php_core_api}
Requires: libmonikor-devel >= 1.0.0

%description
This package contains a PHP extension to allow profiling for Quanta probes.

%prep
%setup -n php56-quanta-mon

%build
phpize
%configure

%install
make clean all
mkdir -p %{buildroot}/etc/php.d/
mkdir -p %{buildroot}/usr/lib64/php/modules
install -m 644 quanta_mon.ini %{buildroot}/etc/php.d/quanta_mon.ini
install -m 755 modules/quanta_mon.so %{buildroot}/usr/lib64/php/modules/quanta_mon.so

%files
/usr/lib64/php/modules/quanta_mon.so
/etc/php.d/quanta_mon.ini
