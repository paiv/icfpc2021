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
        url = urljoin(self.endpoint, path)
        o = urlparse(url)
        headers = {'Authorization': f'Bearer {self.api_key}'}
        print(f'GET {o.path}', file=sys.stderr)
        self._open_conn()
        self.conn.request('GET', o.path, headers=headers)
        r = self.conn.getresponse()
        if r.status != 200:
            raise Exception((r.status, r.reason))
        content = r.read()
        return json.loads(content)

    def post(self, path, obj):
        content = json.dumps(obj, separators=',:')
        url = urljoin(self.endpoint, path)
        o = urlparse(url)
        headers = {'Authorization': f'Bearer {self.api_key}',
            'Content-Type': 'application/json'}
        print(f'POST {o.path}\n{content}', file=sys.stderr)
        self._open_conn()
        self.conn.request('POST', o.path, content, headers=headers)
        r = self.conn.getresponse()
        if not (200 <= r.status < 300):
            raise Exception((r.status, r.reason))
        content = r.read()
        return json.loads(content)

    def hello(self):
        return self.get('/api/hello')

    def getproblem(self, n):
        return self.get(f'/api/problems/{n}')

    def post_solution(self, n, sol):
        return self.post(f'/api/problems/{n}/solutions', sol)


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


def solve(n):
    sol = { "vertices": [
    [21, 28], [31, 28], [31, 87], [29, 41], [44, 43], [58, 70],
    [38, 79], [32, 31], [36, 50], [39, 40], [66, 77], [42, 29],
    [46, 49], [49, 38], [39, 57], [69, 66], [41, 70], [39, 60],
    [42, 25], [40, 35]
    ]}

    with PortalClient() as cli:
        cli.post_solution(n, sol)


def main(*args):
    # hello()
    # grab_problems(1, 60)
    solve(1)
    pass


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='')
    args = parser.parse_args()
    main(args)
