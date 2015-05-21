import os
import jinja2

from .build import Build


class Builder:
    """
    This class is in charge of pulling/building docker instances

    """
    def __init__(self, cfg, without_docker=False, without_sqldump=False, debug=True):
        from docker.client import Client
        from docker.utils import kwargs_from_env

        self.debug = debug
        self.config = cfg
        self.builds = []
        self.without_docker = without_docker
        self.without_sqldump = without_sqldump
        docker_kwargs = kwargs_from_env()
        docker_kwargs['tls'].assert_hostname = False
        self.docker = Client(**docker_kwargs)
        self.jinja = jinja2.Environment(loader=jinja2.FileSystemLoader(searchpath=self.config['global']['templates_dir']))


    def _build_dir(self):
        """
        Returns the build directory from configuration

        """
        return self.config['global'].get('docker_build_dir', '/tmp/build')


    def _create_context(self, build_name, vary, build_env):
        """
        Creates the context for a given build from the configuration

        """
        ctx = dict(self.config['environment'])
        for k, v in zip(vary, build_env):
            ctx.update(self.config['context_data'][k][v])
        ctx['build_name'] = build_name
        return ctx


    def get_template(self, name):
        """
        Fetch the dockerfile template path from the config and returns a jinja Template instance

        """
        return self.jinja.get_template(name)


    def prepare(self):
        """
        This method prepare the build by creating Build objects

        """
        import itertools

        vary_values = []
        vary_keys = self.config['environment'].get('vary', [])
        for key in vary_keys:
            vary_values.append(list(self.config['context_data'][key].keys()))
        for build_env in itertools.product(*vary_values):
            build_name = '_'.join(build_env)
            self.builds.append(
                Build(self, build_name,
                    os.path.join(self._build_dir(), build_name),
                    self._create_context(build_name, vary_keys, build_env),
                    debug=self.debug
                    )
            )


    def build(self):
        """
        This method builds all the needed environments

        """
        for build in self.builds:
            build.build()
