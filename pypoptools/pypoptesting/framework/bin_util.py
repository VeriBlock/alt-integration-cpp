import os
import socket
import errno
import contextlib
import random


def assert_dir_accessible(dirname: str) -> None:
    assert os.path.exists(dirname), "{} does not exist".format(dirname)
    assert os.path.isdir(dirname), "{} is not dir".format(dirname)
    assert os.access(dirname, os.W_OK), "{} is not writeable".format(dirname)


def get_open_port(lowest_port=0, highest_port=None, bind_address='', *socket_args, **socket_kwargs):
    if highest_port is None:
        highest_port = lowest_port + 100
    while True:
        try:
            port = random.randint(lowest_port, highest_port)
            with contextlib.closing(socket.socket(*socket_args, **socket_kwargs)) as my_socket:
                my_socket.bind((bind_address, port))
                this_port = my_socket.getsockname()[1]
                return this_port
        except socket.error as error:
            if not error.errno == errno.EADDRINUSE:
                raise error
