import os
import pathlib
import socket
import errno
import contextlib


def assert_dir_accessible(dirname: pathlib.Path) -> None:
    assert dirname.exists(), "{} does not exist".format(dirname)
    assert dirname.is_dir(), "{} is not dir".format(dirname)
    assert os.access(dirname, os.W_OK), "{} is not writeable".format(dirname)


RESERVED_PORTS = set()


def get_open_port(lowest_port=0, highest_port=None, bind_address='', *socket_args, **socket_kwargs):
    if highest_port is None:
        highest_port = lowest_port + 100
    while lowest_port < highest_port:
        if lowest_port not in RESERVED_PORTS:
            try:
                with contextlib.closing(socket.socket(*socket_args, **socket_kwargs)) as my_socket:
                    my_socket.bind((bind_address, lowest_port))
                    this_port = my_socket.getsockname()[1]
                    RESERVED_PORTS.add(this_port)
                    return this_port
            except socket.error as error:
                if not error.errno == errno.EADDRINUSE:
                    raise
                assert not lowest_port == 0
                RESERVED_PORTS.add(lowest_port)
        lowest_port += 1
    raise Exception('Could not find open port')
