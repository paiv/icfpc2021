#!/usr/bin/env python
import configparser
import http.client
import json
import sys
import time
from pathlib import Path
from urllib.parse import urljoin, urlparse


class PortalClient:
    def __init__(self, endpoint=None, api_key=None):
        self._read_env()
        if endpoint:
            self.endpoint = endpoint
        if api_key:
            self.api_key = api_key
        self.conn = None

    def _read_env(self):
        env = Path(__file__).with_name('.env')
        if env.is_file():
            config = configparser.ConfigParser()
            config['default'] = {'ENDPOINT': 'https://poses.live'}
            config.read(env)
            self.endpoint = config['default']['ENDPOINT']
            self.api_key = config['default']['API_KEY']

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def _open_conn(self):
        o = urlparse(self.endpoint)
        self.conn = http.client.HTTPSConnection(o.hostname, port=o.port)

    def close(self):
        if self.conn:
            self.conn.close()

    def get(self, path):
        self._open_conn()
        url = urljoin(self.endpoint, path)
        o = urlparse(url)
        headers = {'Authorization': f'Bearer {self.api_key}'}
        print(f'GET {o.path}', file=sys.stderr)
        self.conn.request('GET', o.path, headers=headers)
        r = self.conn.getresponse()
        if r.status != 200:
            raise Exception((r.status, r.reason))
        content = r.read()
        return json.loads(content)

    def hello(self):
        return self.get('/api/hello')

    def getproblem(self, n):
        return self.get(f'/api/problems/{n}')

    def getproblems(self):
        return self.get(f'/api/problems')


def hello():
    with PortalClient() as cli:
        print(cli.hello())


def grab_problems(start, stop):
    path = Path(__file__).with_name('spec') / 'problems'
    with PortalClient() as cli:
        for i in range(start, stop):
            o = cli.getproblem(i)
            with open(path / f'{i}.json', 'w') as fp:
                json.dump(o, fp, separators=',:')
            time.sleep(0.25)


def main(*args):
    # hello()
    # grab_problems(1, 60)
    pass


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='')
    args = parser.parse_args()
    main(args)