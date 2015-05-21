import os
import json
import uuid

class Build:
    """
    This class represents a unique build

    """
    DOCKER_IMG_NAME = 'quanta/dockerunner'

    def __init__(self, builder, name, build_dir, context, debug=False):
        self.debug = debug
        self.builder = builder
        self.name = name
        self.build_dir = build_dir
        self.context = context
        self.uid = str(uuid.uuid1())
        self.context['docker_tag'] = self.docker_tag


    @property
    def docker_tag(self):
        """
        Returns the docker tag name

        """
        return '{}:{}_{}'.format(self.DOCKER_IMG_NAME, self.name, self.uid)


    def build(self):
        """
        Performs the build for this object, this step includes:
          - creation of the build directory
          - creation of a dockerfile
          - build of the docker image
          - import of the SQL database

        Each task is related to the build configuration defined in the builder

        """
        if self.debug:
            print("Building {}".format(self.name))
        if not os.path.isdir(self.build_dir):
            os.makedirs(self.build_dir)
        self.render_templates()
        if not self.builder.without_docker:
            self.build_docker()
        if not self.builder.without_sqldump:
            self.import_sqldump()
        print("Built {}".format(self.name))


    def render_templates(self):
        """
        This method iterates through the template list and render all templates into the build dir

        """
        for file_path, template in self.context.get('templates', {}).items():
            if self.debug:
                print('Rendering template {}'.format(template))
            with open(os.path.join(self.build_dir, file_path), 'w') as file:
                file.write(self.builder.get_template(template).render(**self.context))


    def build_docker(self):
        """
        This method is in charge of creating the Dockerfile from the template and building the docker image

        """
        for out in self.builder.docker.build(path=self.build_dir, rm=True, forcerm=True, tag=self.docker_tag):
            if self.debug:
                print("Building {} >> {}".format(self.docker_tag, out))


    def import_sqldump(self):
        """
        This method imports the sqldump into the database
        Both the sqldump to use and the database are fetched from the config

        """
        print("Acting like I'm importing dumps for {}".format(self.name))
