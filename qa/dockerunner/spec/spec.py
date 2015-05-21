import subprocess
import json

class Spec:
    """
    This class represents a spec script

    """
    DEFAULT_TIMEOUT = 5

    def __init__(self, script, expected_code=0, timeout=None):
        self.script = script
        self.expected_code = expected_code
        if timeout:
            self.timeout = timeout
        else:
            self.timeout = self.DEFAULT_TIMEOUT

    def run(self, host, ports, context):
        """
        Runs the spec script

        """
        proc = subprocess.Popen([self.script, host, json.dumps(ports), json.dumps(context)],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            (out, err) = proc.communicate(timeout=self.timeout)
            if proc.returncode != self.expected_code:
                print("ERROR: Test {} failed (code {})".format(self.script, proc.returncode))
                print("STDOUT: {}\nSTDERR: {}".format(out, err))
                return False
            else:
                print("OK: Test {} succeded".format(self.script))
                return True
        except subprocess.TimeoutExpired:
            print("ERROR: Test {} failed (timeout)".format(self.script))
            proc.kill()
            return False
