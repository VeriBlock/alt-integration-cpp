import subprocess
import tempfile

from .sync_util import wait_until


class ProcessManager:
    def __init__(self, args, datadir):
        self.args = args
        self.datadir = datadir
        self.stderr = None
        self.stdout = None
        self.process = None
        self.running = False

    def __del__(self):
        if self.process is not None:
            self.process.kill()

    def start(self):
        if self.running:
            return

        self.stderr = tempfile.NamedTemporaryFile(prefix="stderr_", dir=self.datadir, delete=False)
        self.stdout = tempfile.NamedTemporaryFile(prefix="stdout_", dir=self.datadir, delete=False)

        self.process = subprocess.Popen(
            self.args,
            cwd=self.datadir,
            stdout=self.stdout,
            stderr=self.stderr
        )
        wait_until(lambda: self.is_running(), timeout=30)
        self.running = True

    def stop(self):
        if not self.running:
            return

        self.process.terminate()
        wait_until(lambda: not self.is_running(), timeout=60)
        self.running = False
        self.process = None

        self.stdout.close()
        self.stdout = None
        self.stderr.close()
        self.stderr = None

    def is_running(self):
        if not self.process:
            return False
        return_code = self.process.poll()
        if return_code is None:
            return True
        assert return_code == 0, "Node exited with non-zero code ({})".format(return_code)
        return False
