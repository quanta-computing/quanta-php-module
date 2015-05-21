import os

from .spec import Spec


class Suite:
    """
    This class represents a test suite

    """
    def __init__(self, path, suites=None):
        self.path = path
        self._load_tests(suites)


    @property
    def script(self):
        """
        This just returns self.path for correct duck-typing with Spec

        """
        return self.path


    def _load_tests(self, suites=None):
        """
        Read the tests or suites which are present in self.path and load them

        """
        self.tests = []
        if not os.path.isdir(self.path):
            return
        (_, dirs, files) = next(os.walk(self.path))
        for d in dirs:
            if suites is None or os.path.basename(d) in suites:
                self.tests.append(Suite(os.path.join(self.path, d)))
        for f in files:
            spec = os.path.join(self.path, f)
            if os.access(spec, os.X_OK):
                self.tests.append(Spec(spec))
            else:
                print("Warning: {} is not a valid spec file (not executable)".format(spec))



    def run(self, host, ports, context):
        """
        Run the contained tests or suite

        """
        print("Running test suite {}".format(self.path))
        code = True
        for t in self.tests:
            if not t.run(host, ports, context):
                code = False
        return code
