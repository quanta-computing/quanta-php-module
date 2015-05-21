#
# Dockerfile template for building Quanta PHP module
#

FROM {{base_image}}
MAINTAINER Matthieu 'Korrigan' ROSINSKI <mro@quanta-computing.com>

RUN {{pkg_install_cmd}} {{pkg_nc}} {{pkg_zsh}}

CMD ["/bin/zsh"]

EXPOSE 80
EXPOSE 8080

COPY local.xml /
