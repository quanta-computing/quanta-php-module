import os


class Runner:
    """
    This class is in charge of loading test suites and runs them on different environments

    """
    STOP_TIMEOUT = 3

    def __init__(self, cfg):
        from docker.client import Client
        from docker.utils import kwargs_from_env

        self.config = cfg
        docker_kwargs = kwargs_from_env()
        docker_kwargs['tls'].assert_hostname = False
        self.docker = Client(**docker_kwargs)


    def run(self, build, *tests):
        """
        Run all the test suites passed in as parameters on the given build
        This method will start a container of the build, run the tests and stop it

        """
        from docker.utils import create_host_config

        print("Running tests on {}".format(build.name))
        ports = self.config['environment']['ports']
        host = self.config['global'].get('docker_host', os.getenv('DOCKER_HOST').split('/')[-1].split(':')[0])
        container = self.docker.create_container(
                image=build.docker_tag,
                command='/bin/bash -c "nc -l 8080"',
                ports=ports,
                host_config=create_host_config(port_bindings=dict(zip(ports, [None] * len(ports))))
            ).get('Id')
        self.docker.start(container)
        info = self.docker.inspect_container(container)
        port_bindings = {port: bind[0]['HostPort'] for port, bind in info['NetworkSettings']['Ports'].items()}
        for test in tests:
            test.run(host, port_bindings, build.context)
        self.docker.stop(container, timeout=self.STOP_TIMEOUT)
        log_file_path = os.path.join(self.config['global'].get('logs_dir', '/tmp'), '{}.log'.format(build.name))
        with open(log_file_path, 'wb') as logs:
            logs.write(self.docker.logs(container, stdout=True, stderr=True, stream=False))
        print("Container logs wrote to {}".format(log_file_path))
